#include "vraybase.h"
#include "vrayinterface.h"
#include "vraycore.h"
#include "vrayrenderer.h"

#include "albrdf.h"
#include "sss.h"
#include "vraydmcsampler.h"
#include "beckmann.h"

using namespace VUtils;

void MyBaseBSDF::init(const VRayContext &rc) {
	origBackside=rc.rayresult.realBack;

	nsamples=sqr(params.subdivs);

	// Set the normals to use for lighting
	normal=rc.rayresult.normal;
	gnormal=rc.rayresult.gnormal;
	if (rc.rayresult.realBack) {
		normal=-normal;
		gnormal=-gnormal;
	}

	// Dim the diffuse/reflection colors by the transparency
	Color invTransp=params.transparency.whiteComplement();
	params.diffuse*=invTransp;
	params.reflectColor1*=invTransp;
	params.reflectColor2*=invTransp;

	eta1=1.0f/params.reflectIOR1;
	viewFresnel1=computeDielectricFresnel(rc.rayparams.viewDir, getReflectDir(rc.rayparams.viewDir, normal), normal, eta1);
	viewFresnel1=clamp(viewFresnel1, 0.001f, 0.999f); // Can be optimized for when there is no diffuse component.

	eta2=1.0f/params.reflectIOR2;
	viewFresnel2=computeDielectricFresnel(rc.rayparams.viewDir, getReflectDir(rc.rayparams.viewDir, normal), normal, eta2);
	viewFresnel2=clamp(viewFresnel2, 0.001f, 0.999f); // Can be optimized for when there is no diffuse component.

	// If GI or glossy ray, disable SSS
	if (rc.rayparams.diffuseLevel>0 || (rc.rayparams.totalLevel>1 && 0!=(rc.rayparams.rayType & RT_GLOSSY)))
		params.sssMix=0.0f;

	const VR::VRaySequenceData &sdata=rc.vray->getSequenceData();

	// Check if we need to trace the reflection
	dontTrace=p_or(
		p_or(rc.rayparams.totalLevel>=sdata.params.options.mtl_maxDepth, 0==sdata.params.options.mtl_reflectionRefraction),
		0!=(rc.rayparams.rayType & RT_INDIRECT) // p_and(0!=(rc.rayparams.rayType & RT_INDIRECT), !sdata.params.gi.reflectCaustics)
	);

	if (dontTrace) {
		params.reflectColor1.makeZero();
		params.reflectColor2.makeZero();
	} else {
		params.reflectRoughness1=VR::sqr(clamp(params.reflectRoughness1, 0.0f, 0.995f));
		params.reflectRoughness2=VR::sqr(clamp(params.reflectRoughness2, 0.0f, 0.995f));

		// Compute the normal matrix
		if (dotf(rc.rayparams.viewDir, normal)>0.0f) computeNormalMatrix(rc, gnormal, nm);
		else computeNormalMatrix(rc, normal, nm);

		inm=inverse(nm);
	}

	useMISForDiffuse=rc.vray->getSequenceData().globalLightManager->isGatheringPoint(rc);
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

Color MyBaseBSDF::eval(const VRayContext &rc, const Vector &direction, Color &lightColor, Color &origLightColor, float probLight, int flags) {
	Color res(0.0f, 0.0f, 0.0f);

	// Skip this part if specular component is not required
	if (!dontTrace && 0!=(flags & FBRDF_SPECULAR)) {
		if (params.reflectRoughness1>=1e-6f) {
			float probReflection=0.0f;
			float brdfValue=0.0f;
			if (params.reflectMode1==alReflectDistribution_GGX) brdfValue=ggxBRDFMixed(rc.rayparams.viewDir, direction, params.reflectRoughness1, 2.0f, normal, nm, inm, probReflection, false);
			else brdfValue=beckmannBRDF(rc.rayparams.viewDir, direction, params.reflectRoughness1, normal, nm, inm, probReflection);

			float k=getReflectionWeight(probLight, probReflection);
			float fresnel=computeDielectricFresnel(rc.rayparams.viewDir, direction, normal, eta1);
			res+=(0.5f*k*brdfValue*fresnel)*(params.reflectColor1*lightColor);

			if (probReflection>0.0f) {
				float kti=clamp(1.0f-fresnel*params.reflectColor1.maxComponentValue(), 0.0f, 1.0f);
				lightColor*=kti;
				origLightColor*=kti;
			}
		}

		if (params.reflectRoughness2>=1e-6f) {
			float probReflection=0.0f;
			float brdfValue=0.0f;
			if (params.reflectMode2==alReflectDistribution_GGX) brdfValue=ggxBRDFMixed(rc.rayparams.viewDir, direction, params.reflectRoughness2, 2.0f, normal, nm, inm, probReflection, false);
			else brdfValue=beckmannBRDF(rc.rayparams.viewDir, direction, params.reflectRoughness2, normal, nm, inm, probReflection);

			float k=getReflectionWeight(probLight, probReflection);
			float fresnel=computeDielectricFresnel(rc.rayparams.viewDir, direction, normal, eta2);
			res+=(0.5f*k*brdfValue*fresnel)*(params.reflectColor2*lightColor);

			if (probReflection>0.0f) {
				float kti=clamp(1.0f-fresnel*params.reflectColor2.maxComponentValue(), 0.0f, 1.0f);
				lightColor*=kti;
				origLightColor*=kti;
			}
		}
	}

	// Skip this part if diffuse component is not required
	if (0!=(flags & FBRDF_DIFFUSE)) {
		float cs=dotf(direction, normal);
		if (cs<0.0f) cs=0.0f;
		float probReflection=2.0f*cs;

		float k=useMISForDiffuse? getReflectionWeight(probLight, probReflection) : 1.0f;
		res+=(0.5f*probReflection*k)*(1.0f-params.sssMix)*(params.diffuse*lightColor);
	}

	lightColor*=params.transparency;
	origLightColor*=params.transparency;

	return res;
}

void MyBaseBSDF::traceForward(VRayContext &rc, int doDiffuse) {
	rc.mtlresult.clear();

	float reflectTransp=1.0f;
	int reflectOpaque=false;
	if (2!=doDiffuse && !dontTrace && nsamples!=0) {
		rc.mtlresult.color+=computeReflections(rc, reflectTransp);
		reflectOpaque=(reflectTransp<1e-6f);
	}

	if (!reflectOpaque) {
		Color diffuse=params.diffuse*reflectTransp;
		if (doDiffuse) rc.mtlresult.color+=rc.evalDiffuse()*diffuse*(1.0f-params.sssMix);

		if (params.sssMix>0.0f) {
			VR::Color rawSSSResult=computeRawSSS(rc, diffuse);
			rc.mtlresult.color+=rawSSSResult*params.sssMix*diffuse;
		}
	}

	rc.mtlresult.transp=params.transparency;
	rc.mtlresult.alpha=params.transparency.whiteComplement();
	rc.mtlresult.alphaTransp=params.transparency;
}

static const VR::Color red(1.0f, 0.0f, 0.0f);
static const VR::Color green(0.0f, 1.0f, 0.0f);
static const VR::Color blue(0.0f, 0.0f, 1.0f);

VR::Color MyBaseBSDF::computeRawSSS(VRayContext &rc, const Color &diffuse) {
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
	VR::Color result_sss=alsDiffusion(rc, &diffusion_msgdata, directional, nc, params.sssMix, diffuse);

	return result_sss;
}

struct ReflectionsSampler: AdaptiveColorSampler {
	ReflectionsSampler(const VRayContext &rc, MyBaseBSDF &bsdf, const VR::Color &color, float roughness, float eta, ALReflectDistribution mode)
		:fresnelSum(0.0f), bsdf(bsdf), color(color), roughness(roughness), eta(eta), mode(mode)
	{}

	VR::Color sampleColor(const VR::VRayContext &rc, VR::VRayContext &nrc, float uc, VR::ValidType &valid) VRAY_OVERRIDE {
		Vector dir;
		float brdfMult=1.0f;

		if (roughness<1e-6f) {
			// Pure reflection
			dir=getReflectDir(rc.rayparams.viewDir, bsdf.getNormal());

			// If the reflection direction is below the surface, use the geometric normal
			const Vector &gnormal=bsdf.getGNormal();
			real r0=-dotf(rc.rayparams.viewDir, gnormal);
			real r1=dotf(dir, gnormal);
			if (r0*r1<0.0f) dir=getReflectDir(rc.rayparams.viewDir, gnormal);
		} else {
			// Compute a reflection direction
			if (mode==alReflectDistribution_GGX) {
				dir=ggxDirMixed(uc, AdaptiveColorSampler::getDMCParam(nrc, 1), roughness, 2.0f, rc.rayparams.viewDir, bsdf.getNormalMatrix(), nrc.rayparams.rayProbability, brdfMult, false, bsdf.getNormalMatrixInv());
			} else {
				dir=beckmannDir(uc, AdaptiveColorSampler::getDMCParam(nrc, 1), roughness, rc.rayparams.viewDir, bsdf.getNormalMatrix(), nrc.rayparams.rayProbability, brdfMult);
			}

			// If this is below the surface, ignore
			if (dotf(dir, rc.rayresult.gnormal)<0.0f)
				return false;
		}

		// Set ray derivatives
		VR::getReflectDerivs(rc.rayparams.viewDir, dir, rc.rayparams.dDdx, rc.rayparams.dDdy, nrc.rayparams.dDdx, nrc.rayparams.dDdy);

		// Compute Fresnel term
		float fresnel=computeDielectricFresnel(rc.rayparams.viewDir, dir, bsdf.getNormal(), eta);
		fresnelSum+=fresnel;

		// Set the direction into the ray context
		nrc.rayparams.tracedRay.dir=nrc.rayparams.viewDir=dir;

		// Trace the ray
		Color col=nrc.traceCurrentRay();

		// Apply reflectoin color, fresnel and BRDF value
		col*=color*brdfMult*fresnel;

		return col;
	}

	void multResult(float m) VRAY_OVERRIDE {
		fresnelSum*=m;
	}

	float getFresnel(void) {
		return fresnelSum;
	}

protected:
	float fresnelSum;
	MyBaseBSDF &bsdf;
	float roughness;
	float eta;
	VR::Color color;
	ALReflectDistribution mode;
};

Color MyBaseBSDF::computeReflections(VRayContext &rc, float &reflectTransp) {
	// Create a new context
	VRayContext &nrc=rc.newSpawnContext(2, params.reflectColor1*viewFresnel1, RT_REFLECT | RT_GLOSSY | RT_ENVIRONMENT, normal);

	// Set up the new context
	nrc.rayparams.dDdx.makeZero(); // Zero out the directional derivatives
	nrc.rayparams.dDdy.makeZero();
	nrc.rayparams.mint=0.0f; // Set the ray extents
	nrc.rayparams.maxt=1e18f;
	nrc.rayparams.tracedRay.p=rc.rayresult.wpoint; // Set the new ray origin to be the surface hit point

	// Figure out how many samples to take
	int nsamples=this->nsamples;
	if (rc.rayparams.currentPass==RPASS_GI || (rc.rayparams.rayType & RT_LIGHT)!=0 || params.reflectRoughness1<1e-6f) nsamples=0;

	Color reflections(0.0f, 0.0f, 0.0f);

	{ // Integrate primary reflections
		ReflectionsSampler reflectionsSampler(rc, *this, params.reflectColor1, params.reflectRoughness1, eta1, params.reflectMode1);
		reflections+=reflectionsSampler.sample(rc, nrc, nsamples, 0x6763223);
		reflectTransp=1.0f-(reflectionsSampler.getFresnel()*params.reflectColor1.maxComponentValue());
	}

	{ // Integrate secondary reflections
		nrc.rayparams.currentMultResult=params.reflectColor2*viewFresnel2*reflectTransp;
		ReflectionsSampler reflectionsSampler(rc, *this, params.reflectColor2*reflectTransp, params.reflectRoughness2, eta2, params.reflectMode2);
		reflections+=reflectionsSampler.sample(rc, nrc, nsamples, 0x3223AC2);
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

void MyBaseBSDF::computeNormalMatrix(const VR::VRayContext &rc, const VR::Vector &normal, VR::Matrix &nm) {
	makeNormalMatrix(normal, nm);
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
