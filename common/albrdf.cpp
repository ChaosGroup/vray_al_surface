#include "vraybase.h"
#include "vrayinterface.h"
#include "vraycore.h"
#include "vrayrenderer.h"
#include "vraydmcsampler.h"
#include "brdfs.h"

#include "albrdf.h"
#include "sss.h"
#include "beckmann.h"

using namespace VUtils;

inline void invertIOR(float &ior) {
	if (ior<1.0f && ior>0.0f) ior=clamp(1.0f/ior, 1.0f, 1e4f);
}

void MyBaseBSDF::init(const VRayContext &rc) {
	origBackside=rc.rayresult.realBack;

	reflectSamples=sqr(params.reflectSubdivs);
	sssSamples=sqr(params.sssSubdivs);

	// Set the normals to use for lighting
	normal=rc.rayresult.normal;
	gnormal=rc.rayresult.gnormal;
	if (rc.rayresult.realBack) {
		normal=-normal;
		gnormal=-gnormal;
		params.diffuseNormal=-params.diffuseNormal;
		params.reflectNormal1=-params.reflectNormal1;
		params.reflectNormal2=-params.reflectNormal2;
	}

	// If IORs are <1.0f, invert them.
	invertIOR(params.reflectIOR1);
	invertIOR(params.reflectIOR2);

	// Dim the diffuse/reflection colors by the transparency
	Color invTransp=params.transparency.whiteComplement();
	params.diffuse*=invTransp;
	params.reflectColor1*=invTransp;
	params.reflectColor2*=invTransp;

	eta1=1.0f/params.reflectIOR1;
	viewFresnel1=computeDielectricFresnel(rc.rayparams.viewDir, getReflectDir(rc.rayparams.viewDir, params.reflectNormal1), normal, eta1);
	viewFresnel1=clamp(viewFresnel1, 0.001f, 0.999f); // Can be optimized for when there is no diffuse component.

	eta2=1.0f/params.reflectIOR2;
	viewFresnel2=computeDielectricFresnel(rc.rayparams.viewDir, getReflectDir(rc.rayparams.viewDir, params.reflectNormal2), normal, eta2);
	viewFresnel2=clamp(viewFresnel2, 0.001f, 0.999f); // Can be optimized for when there is no diffuse component.

	// If GI or glossy ray, disable SSS
	if (rc.rayparams.diffuseLevel>0 || (rc.rayparams.totalLevel>1 && 0!=(rc.rayparams.rayType & RT_GLOSSY)))
		params.sssMix=0.0f;

	const VR::VRaySequenceData &sdata=rc.vray->getSequenceData();

	// Check if we need to trace the reflection
	int maxDepth=params.reflectMaxDepth;
	if (maxDepth==-1)
		maxDepth=sdata.params.options.mtl_maxDepth;

	dontTrace=p_or(
		p_or(rc.rayparams.totalLevel>=maxDepth, 0==sdata.params.options.mtl_reflectionRefraction),
		0!=(rc.rayparams.rayType & RT_INDIRECT)
	);

	computeSpecular1=(params.reflectRoughness1>1e-6f && params.reflectColor1.maxComponentValue()>1e-6f);
	computeSpecular2=(params.reflectRoughness2>1e-6f && params.reflectColor2.maxComponentValue()>1e-6f);

	if (dontTrace) {
		params.reflectColor1.makeZero();
		params.reflectColor2.makeZero();
	} else {
		params.reflectRoughness1=VR::sqr(clamp(params.reflectRoughness1, 0.0f, 0.995f));
		params.reflectRoughness2=VR::sqr(clamp(params.reflectRoughness2, 0.0f, 0.995f));

		// Compute the normal matrices
		computeNormalMatrix(rc, params.reflectNormal1, nm1);
		inm1=inversef(nm1);

		if ((params.reflectNormal1-params.reflectNormal2).lengthSqr()<1e-12f || !computeSpecular2) {
			// If the secondary normal is the same as the primary one,
			// or we don't have secondary specular, just reuse the primary matrices.
			nm2=nm1;
			inm2=inm1;
		} else {
			computeNormalMatrix(rc, params.reflectNormal2, nm2);
			inm2=inversef(nm2);
		}
	}

	useMISForDiffuse=rc.vray->getSequenceData().globalLightManager->isGatheringPoint(rc);

	computeRenderElements=(rc.mtlresult.fragment!=NULL);
	if (computeRenderElements) {
		finalElements.makeZero();
		currentElements.makeZero();
	}
}

// From BRDFSampler
Color MyBaseBSDF::getDiffuseColor(Color &lightColor) {
	float reflAmount1=viewFresnel1*params.reflectColor1.maxComponentValue();
	float reflAmount2=viewFresnel2*params.reflectColor2.maxComponentValue();
	Color res=params.diffuse*(1.0f-params.sssMix)*(1.0f-reflAmount1)*(1.0f-reflAmount2);
	res*=lightColor;
	lightColor*=params.transparency;
	return res;
}

Color MyBaseBSDF::getLightMult(Color &lightColor) {
	float reflAmount1=viewFresnel1*params.reflectColor1.maxComponentValue();
	float reflAmount2=viewFresnel2*params.reflectColor2.maxComponentValue();
	Color res=params.diffuse*(1.0f-params.sssMix)*(1.0f-reflAmount1)*(1.0f-reflAmount2)
		+params.reflectColor2*viewFresnel2*(1.0f-reflAmount1)
		+params.reflectColor1*viewFresnel1;
	res*=lightColor;
	lightColor*=params.transparency;
	return res;
}

Vector MyBaseBSDF::getDiffuseNormal(const VRayContext &rc) {
	return params.diffuseNormal;
}

Color MyBaseBSDF::eval(const VRayContext &rc, const Vector &direction, Color &lightColor, Color &origLightColor, float probLight, int flags) {
	Color res(0.0f, 0.0f, 0.0f);

	// Skip this part if specular component is not required
	if (!dontTrace && 0!=(flags & FBRDF_SPECULAR)) {
		if (computeSpecular1) {
			float probReflection=0.0f;
			float brdfValue=0.0f;
			if (params.reflectMode1==alReflectDistribution_GGX) {
				brdfValue=ggxBRDFMixed3f(
					rc.rayparams.getViewDir3f(),
					simd::Vector3f(direction),
					params.reflectRoughness1, 2.0f,
					simd::Vector3f(params.reflectNormal1),
					nm1, inm1,
					probReflection,
					false
				);
			}
			else {
				brdfValue=beckmannBRDF3f(
					rc.rayparams.getViewDir3f(),
					simd::Vector3f(direction),
					params.reflectRoughness1,
					simd::Vector3f(params.reflectNormal1),
					nm1, inm1,
					probReflection
				);
			}

			float k=getReflectionWeight(probLight, probReflection);
			float fresnel=computeDielectricFresnel(rc.rayparams.viewDir, direction, params.reflectNormal1, eta1);
			Color rawSpecularLighting=(0.5f*k*brdfValue)*lightColor;
			Color specularFilter=(params.reflectColor1*fresnel);
			Color specularLighting=(rawSpecularLighting*specularFilter);
			res+=specularLighting;

			if (fresnel>0.0f) {
				float kti=clamp(1.0f-fresnel*params.reflectColor1.maxComponentValue(), 0.0f, 1.0f);
				lightColor*=kti;
				origLightColor*=kti;
			}

			if (computeRenderElements) {
				currentElements.specularLighting+=specularLighting;
			}
		}

		if (computeSpecular2) {
			float probReflection=0.0f;
			float brdfValue=0.0f;
			if (params.reflectMode2==alReflectDistribution_GGX) {
				brdfValue=ggxBRDFMixed3f(
					rc.rayparams.getViewDir3f(),
					simd::Vector3f(direction),
					params.reflectRoughness2, 2.0f,
					simd::Vector3f(params.reflectNormal2),
					nm2, inm2,
					probReflection,
					false
				);
			} else {
				brdfValue=beckmannBRDF3f(
					rc.rayparams.getViewDir3f(),
					simd::Vector3f(direction),
					params.reflectRoughness2,
					simd::Vector3f(params.reflectNormal2),
					nm2, inm2,
					probReflection
				);
			}

			float k=getReflectionWeight(probLight, probReflection);
			float fresnel=computeDielectricFresnel(rc.rayparams.viewDir, direction, params.reflectNormal2, eta2);
			Color rawSpecularLighting=(0.5f*k*brdfValue)*lightColor;
			Color specularFilter=(params.reflectColor2*fresnel);
			Color specularLighting=(rawSpecularLighting*specularFilter);
			res+=specularLighting;

			if (fresnel>0.0f) {
				float kti=clamp(1.0f-fresnel*params.reflectColor2.maxComponentValue(), 0.0f, 1.0f);
				lightColor*=kti;
				origLightColor*=kti;
			}

			if (computeRenderElements) {
				currentElements.specularLighting+=specularLighting;
			}
		}
	}

	// Skip this part if diffuse component is not required
	if (0!=(flags & FBRDF_DIFFUSE)) {
		float cs=dotf(direction, params.diffuseNormal);
		if (cs<0.0f) cs=0.0f;
		float probReflection=2.0f*cs;

		float k=useMISForDiffuse? getReflectionWeight(probLight, probReflection) : 1.0f;
		k*=(0.5f*probReflection)*(1.0f-params.sssMix);
		Color rawDiffuseLighting=k*lightColor;
		Color diffuseLighting=rawDiffuseLighting*params.diffuse;
		res+=diffuseLighting;

		if (computeRenderElements) {
			currentElements.diffuseLighting+=diffuseLighting;
			currentElements.shadow+=(origLightColor-lightColor)*(k*params.diffuse);
		}
	}

	lightColor*=params.transparency;
	origLightColor*=params.transparency;

	if (computeRenderElements) {
		currentElements.color+=res;
	}

	return res;
}

void MyBaseBSDF::multiplyLight(float mult) {
	if (computeRenderElements) {
		currentElements.multiply(mult);
	}
}

void MyBaseBSDF::lightFinished(int last) {
	if (computeRenderElements) {
		finalElements.add(currentElements);
		currentElements.makeZero();
	}
}

void MyBaseBSDF::traceForward(VRayContext &rc, int doDiffuse) {
	RenderElementsResults renderElements;
	renderElements.makeZero();

	if (computeRenderElements) {
		renderElements.diffuseLighting=finalElements.diffuseLighting;
		renderElements.specularLighting=finalElements.specularLighting;
		renderElements.finalShadow=finalElements.shadow;
	}

	float reflectTransp=1.0f;
	int reflectOpaque=false;
	if (2!=doDiffuse && !dontTrace && reflectSamples!=0) {
		renderElements.reflection=computeReflections(rc, reflectTransp, renderElements.reflectionFilter);
		rc.mtlresult.color+=renderElements.reflection;
		reflectOpaque=(reflectTransp<1e-6f);
	}

	if (!reflectOpaque) {
		renderElements.diffuseFilter=params.diffuse*reflectTransp;
		if (doDiffuse) {
			renderElements.rawGI=rc.evalDiffuse()*(1.0f-params.sssMix);
			renderElements.gi=renderElements.rawGI*renderElements.diffuseFilter;
			rc.mtlresult.color+=renderElements.gi;
		}

		if (params.sssMix>1e-6f && renderElements.diffuseFilter.sum()>1e-6f) {
			// We accumulate the raw SSS, along with any raw light select contributions,
			// into a temporary shading result. This result is later multiplied by the sssMix value
			// and the diffuse color.
			ShadeResult sssResult;
			if (rc.mtlresult.fragment) sssResult.fragment=rc.fragmentList->getFragmentManager()->newFragment();
			sssResult.clear();

			computeRawSSS(rc, renderElements.diffuseFilter, sssResult);

			sssResult*=params.sssMix;
			renderElements.rawSSS=sssResult.color;

			sssResult*=renderElements.diffuseFilter;
			renderElements.sss=sssResult.color;

			sssResult.alpha.makeZero();
			sssResult.transp.makeZero();
			sssResult.alphaTransp.makeZero();

			rc.mtlresult.add(sssResult, fbm_normal);
			if (sssResult.fragment) rc.fragmentList->getFragmentManager()->deleteFragment(sssResult.fragment);
		}
	}

	rc.mtlresult.transp=params.transparency;
	rc.mtlresult.alpha=params.transparency.whiteComplement();
	rc.mtlresult.alphaTransp=params.transparency;

	Fragment *f=(Fragment*) rc.mtlresult.fragment;
	if (f) {
		renderElements.finalColor=finalElements.color+rc.mtlresult.color;
		writeRenderElements(f, renderElements);
	}
}

static const VR::Color red(1.0f, 0.0f, 0.0f);
static const VR::Color green(0.0f, 1.0f, 0.0f);
static const VR::Color blue(0.0f, 0.0f, 1.0f);

void MyBaseBSDF::computeRawSSS(VRayContext &rc, const Color &diffuse, ShadeResult &result) {
	int nc=3;
	if (params.sssWeight2>0.0f) nc=6;
	if (params.sssWeight3>0.0f) nc=9;

	float Rd[9]={
		params.sssColor1.r, params.sssColor1.g, params.sssColor1.b,
		params.sssColor2.r, params.sssColor2.g, params.sssColor2.b,
		params.sssColor3.r, params.sssColor3.g, params.sssColor3.b
	};

	float radii[9]={
		params.sssRadius1, params.sssRadius1, params.sssRadius1,
		params.sssRadius2, params.sssRadius2, params.sssRadius2,
		params.sssRadius3, params.sssRadius3, params.sssRadius3
	};

	VR::Color weights[9]={
		red*params.sssWeight1, green*params.sssWeight1, blue*params.sssWeight1,
		red*params.sssWeight2, green*params.sssWeight2, blue*params.sssWeight2,
		red*params.sssWeight3, green*params.sssWeight3, blue*params.sssWeight3
	};

	DirectionalMessageData diffusion_msgdata;
	memset(&diffusion_msgdata, 0, sizeof(DirectionalMessageData));

	memcpy(diffusion_msgdata.weights, weights, sizeof(VR::Color)*nc);

	for (int i=0; i<nc; ++i) {
		diffusion_msgdata.sp[i]=ScatteringProfileDirectional(VR::Max(Rd[i], 0.001f), (params.sssDensityScale/radii[i]));
	}

	bool directional=(params.sssMode==alSSSMode_directional);
	alsDiffusion(rc, &diffusion_msgdata, directional, nc, params.sssMix, diffuse, sssSamples, result);
}

struct ReflectionsSampler: AdaptiveColorSampler {
	ReflectionsSampler(const VRayContext &rc, MyBaseBSDF &bsdf, const VR::Color &color, float roughness, float eta, ALReflectDistribution mode, const Vector &normal, const simd::Matrix3x3f &nm, const simd::Matrix3x3f &inm)
		:fresnelSum(0.0f), bsdf(bsdf), color(color), roughness(roughness), eta(eta), mode(mode), normal(normal), nm(nm), inm(inm), reflectionFilter(0.0f, 0.0f, 0.0f)
	{}

	VR::Color sampleColor(const VR::VRayContext &rc, VR::VRayContext &nrc, float uc, VR::ValidType &valid) VRAY_OVERRIDE {
		Vector dir;
		float brdfMult=1.0f;

		if (roughness<1e-6f) {
			// Pure reflection
			dir=getReflectDir(rc.rayparams.viewDir, normal);

			// If the reflection direction is below the surface, use the geometric normal
			const Vector &gnormal=bsdf.getGNormal();
			real r0=-dotf(rc.rayparams.viewDir, gnormal);
			real r1=dotf(dir, gnormal);
			if (r0*r1<0.0f) dir=getReflectDir(rc.rayparams.viewDir, gnormal);
		} else {
			// Compute a reflection direction
			if (mode==alReflectDistribution_GGX) {
				dir=ggxDirMixed3f(uc, AdaptiveColorSampler::getDMCParam(nrc, 1), roughness, 2.0f, rc.rayparams.getViewDir3f(), nm, nrc.rayparams.rayProbability, brdfMult, false, inm).toVector();
			} else {
				dir=beckmannDir3f(uc, AdaptiveColorSampler::getDMCParam(nrc, 1), roughness, rc.rayparams.getViewDir3f(), nm, nrc.rayparams.rayProbability, brdfMult).toVector();
			}

			// If this is below the surface, ignore
			if (dotf(dir, rc.rayresult.gnormal)<0.0f)
				return false;
		}

		// Set ray derivatives
		VR::getReflectDerivs(rc.rayparams.viewDir, dir, rc.rayparams.dDdx, rc.rayparams.dDdy, nrc.rayparams.dDdx, nrc.rayparams.dDdy);

		// Compute Fresnel term
		float fresnel=computeDielectricFresnel(rc.rayparams.viewDir, dir, normal, eta);
		fresnelSum+=fresnel;

		// Set the direction into the ray context
		nrc.rayparams.tracedRay.dir=nrc.rayparams.viewDir=dir;

		// Trace the ray
		Color col=nrc.traceCurrentRay();

		// Apply BRDF value
		col*=brdfMult;

		// Apply reflection color and Fresnel
		Color filter=(color*fresnel);
		reflectionFilter+=filter;

		col*=filter;
		return col;
	}

	void multResult(float m) VRAY_OVERRIDE {
		fresnelSum*=m;
		reflectionFilter*=m;
	}

	float getFresnel(void) const {
		return fresnelSum;
	}

	Color getReflectionFilter(void) const {
		return reflectionFilter;
	}

protected:
	float fresnelSum;
	MyBaseBSDF &bsdf;
	float roughness;
	float eta;
	Color color;
	ALReflectDistribution mode;
	const Vector &normal;
	const simd::Matrix3x3f &nm;
	const simd::Matrix3x3f &inm;

	Color reflectionFilter;
};

Color MyBaseBSDF::computeReflections(VRayContext &rc, float &reflectTransp, Color &reflectionFilter) {
	// Create a new context
	VRayContext &nrc=rc.newSpawnContext(2, params.reflectColor1*viewFresnel1, RT_REFLECT | RT_GLOSSY | RT_ENVIRONMENT, gnormal);

	// Set up the new context
	nrc.rayparams.dDdx.makeZero(); // Zero out the directional derivatives
	nrc.rayparams.dDdy.makeZero();
	nrc.rayparams.mint=0.0f; // Set the ray extents
	nrc.rayparams.maxt=1e18f;
	nrc.rayparams.tracedRay.p=rc.rayresult.wpoint; // Set the new ray origin to be the surface hit point

	// Figure out how many samples to take
	int nsamples=this->reflectSamples;
	if (rc.rayparams.currentPass==RPASS_GI || (rc.rayparams.rayType & RT_LIGHT)!=0 || params.reflectRoughness1<1e-6f) nsamples=0;

	Color reflections(0.0f, 0.0f, 0.0f);

	// Integrate primary reflections
	if (params.reflectColor1.sum()>1e-6f) {
		ReflectionsSampler reflectionsSampler(rc, *this, params.reflectColor1, params.reflectRoughness1, eta1, params.reflectMode1, params.reflectNormal1, nm1, inm1);
		reflections+=reflectionsSampler.sample(rc, nrc, nsamples, 0x6763223);
		reflectionFilter+=reflectionsSampler.getReflectionFilter();
		reflectTransp=1.0f-(reflectionsSampler.getFresnel()*params.reflectColor1.maxComponentValue());
	}

	// Integrate secondary reflections
	if (params.reflectColor2.sum()>1e-6f) {
		nrc.rayparams.multResult=nrc.rayparams.currentMultResult=params.reflectColor2*viewFresnel2*reflectTransp;
		ReflectionsSampler reflectionsSampler(rc, *this, params.reflectColor2*reflectTransp, params.reflectRoughness2, eta2, params.reflectMode2, params.reflectNormal2, nm2, inm2);
		reflections+=reflectionsSampler.sample(rc, nrc, nsamples, 0x3223AC2);
		reflectionFilter+=reflectionsSampler.getReflectionFilter();
		reflectTransp*=1.0f-(reflectionsSampler.getFresnel()*params.reflectColor2.maxComponentValue());
	}

	// Release the context
	nrc.releaseContext();

	return reflections;
}

Color MyBaseBSDF::getTransparency(const VRayContext &rc) {
	return params.transparency;
}

RenderChannelsInfo* MyBaseBSDF::getRenderChannels(void) { return &RenderChannelsInfo::reflectChannels; }

void MyBaseBSDF::computeNormalMatrix(const VR::VRayContext &rc, const VR::Vector &normal, simd::Matrix3x3f &nm) {
	makeNormalMatrix3f(simd::Vector3f(normal), nm);
}

// From BSDFSampler
BRDFSampler* MyBaseBSDF::getBRDF(BSDFSide side) {
	if (!params.doubleSided) {
		if (side==bsdfSide_back) return NULL; // There is nothing on the back side
		return static_cast<VR::BRDFSampler*>(this);
	} else {
		if (side==bsdfSide_front) {
			if (origBackside) return NULL;
		} else {
			if (!origBackside) return NULL;
		}
		return static_cast<BRDFSampler*>(this);
	}
}

int MyBaseBSDF::getUseIrradianceMap(void) {
	// Use irradiance map/light cache for diffuse GI
	int flags=BRDF_FLAG_USE_CACHED_GI;

	// We generate our own lighting render elements
	flags|=BRDF_FLAG_CUSTOM_RENDER_ELEMENTS;

	return flags;
}

inline Color divideColor(const Color &a, const Color &b) {
	Color res;
	for (int i=0; i<3; i++) {
		if (b[i]>1e-6f) res[i]=a[i]/b[i];
		else res[i]=0.0f;
	}
	return res;
}

void MyBaseBSDF::writeRenderElements(Fragment *f, const RenderElementsResults &renderElements) {
	f->setChannelDataByAlias(REG_CHAN_VFB_DIFFUSE, &renderElements.diffuseFilter);
	f->setChannelDataByAlias(REG_CHAN_VFB_GI, &renderElements.gi);
	f->setChannelDataByAlias(REG_CHAN_VFB_LIGHTING, &renderElements.diffuseLighting);

	// Because of the glossy Fresnel, it is difficult to predict what the raw lighting should
	// look like, so we just derive it from the diffuse filter and the raw lighting.
	Color rawDiffuseLighting=divideColor(renderElements.diffuseLighting, renderElements.diffuseFilter);
	f->setChannelDataByAlias(REG_CHAN_VFB_RAWLIGHT, &rawDiffuseLighting);

	f->setChannelDataByAlias(REG_CHAN_VFB_TOTALLIGHT, &renderElements.gi);
	f->setChannelDataByAlias(REG_CHAN_VFB_TOTALLIGHT, &renderElements.diffuseLighting);

	f->setChannelDataByAlias(REG_CHAN_VFB_RAWGI, &renderElements.rawGI);
	f->setChannelDataByAlias(REG_CHAN_VFB_RAWTOTALLIGHT, &renderElements.rawGI);
	f->setChannelDataByAlias(REG_CHAN_VFB_RAWTOTALLIGHT, &rawDiffuseLighting);

	f->setChannelDataByAlias(REG_CHAN_VFB_REFLECTION_FILTER, &renderElements.reflectionFilter);
	f->setChannelDataByAlias(REG_CHAN_VFB_REFLECT, &renderElements.reflection);

	// Because of the glossy Fresnel, it is difficult to predict what the raw reflection should
	// look like, so we just derive it from the reflection filter and the raw reflections.
	Color rawReflection=divideColor(renderElements.reflection, renderElements.reflectionFilter);
	f->setChannelDataByAlias(REG_CHAN_VFB_RAW_REFLECTION, &renderElements.rawReflection);
	
	f->setChannelDataByAlias(REG_CHAN_VFB_SPECULAR, &renderElements.specularLighting);
	f->setChannelDataByAlias(REG_CHAN_VFB_SSS2, &renderElements.sss);

	// Because of the glossy Fresnel, it is difficult to predict what the raw lighting should
	// look like, so we just derive it from the diffuse filter and the raw lighting.
	Color rawShadow=divideColor(renderElements.finalShadow, renderElements.diffuseFilter);
	f->setChannelDataByAlias(REG_CHAN_VFB_RAWSHADOW, &rawShadow);
				
	VR::Color *matteChan=(VR::Color*) f->getChannelDataPtrByAlias(REG_CHAN_VFB_MATTESHADOW);
	if (matteChan) {
		VR::Color total=renderElements.diffuseLighting+renderElements.finalShadow;
		VR::Color shadowMatte=divideColor(renderElements.finalShadow, total);
		f->setChannelDataByAlias(REG_CHAN_VFB_MATTESHADOW, &shadowMatte);
	}

	f->setChannelDataByAlias(REG_CHAN_VFB_SHADOW, &renderElements.finalShadow);

	if (params.renderChannels) {
		for (int i=0; i<params.renderChannels->count(); i++) {
			f->setChannelData((*params.renderChannels)[i], &renderElements.finalColor);
		}
	}
}
