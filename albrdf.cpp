#include "vraybase.h"
#include "vrayinterface.h"
#include "vraycore.h"
#include "vrayrenderer.h"

#include "albrdf.h"
#include "sss.h"

using namespace VUtils;

void MyBaseBSDF::init(const VRayContext &rc) {
	origBackside=rc.rayresult.realBack;

	nsamples=sqr(params.subdivs);

	// Dim the diffuse color by the reflection
	if (nsamples>0) {
		params.diffuse*=params.reflection.whiteComplement();
	}

	// Dim the diffuse/reflection colors by the transparency
	Color invTransp=params.transparency.whiteComplement();
	params.diffuse*=invTransp;
	params.reflection*=invTransp;

	// If GI or glossy ray, disable SSS
	if (rc.rayparams.diffuseLevel>0 || (rc.rayparams.totalLevel>1 && 0!=(rc.rayparams.rayType & RT_GLOSSY)))
		params.sssMix=0.0f;

	const VR::VRaySequenceData &sdata=rc.vray->getSequenceData();

	// Set the normals to use for lighting
	normal=rc.rayresult.normal;
	gnormal=rc.rayresult.gnormal;
	if (rc.rayresult.realBack) {
		normal=-normal;
		gnormal=-gnormal;
	}

	// Check if we need to trace the reflection
	dontTrace=p_or(
		p_or(rc.rayparams.totalLevel>=sdata.params.options.mtl_maxDepth, 0==sdata.params.options.mtl_reflectionRefraction),
		p_and(0!=(rc.rayparams.rayType & RT_INDIRECT), !sdata.params.gi.reflectCaustics)
	);

	if (dontTrace) {
		params.reflection.makeZero();
	} else {
		params.reflectionGlossiness=remapGlossiness(params.reflectionGlossiness);

		// Compute the normal matrix
		if (rc.rayparams.viewDir*normal>0.0f) computeNormalMatrix(rc, gnormal, nm);
		else computeNormalMatrix(rc, normal, nm);

		inm=inverse(nm);
	}
}

// From BRDFSampler
Color MyBaseBSDF::getDiffuseColor(Color &lightColor) {
	Color res=params.diffuse*lightColor*(1.0f-params.sssMix);
	lightColor*=params.transparency;
	return res;
}

Color MyBaseBSDF::getLightMult(Color &lightColor) {
	Color res=(params.diffuse+params.reflection)*lightColor;
	lightColor*=params.transparency;
	return res;
}

Color MyBaseBSDF::eval(const VRayContext &rc, const Vector &direction, Color &lightColor, Color &origLightColor, float probLight, int flags) {
	Color res(0.0f, 0.0f, 0.0f);

	// Skip this part if diffuse component is not required
	if (0!=(flags & FBRDF_DIFFUSE)) {
		float cs=dotf(direction, normal);
		if (cs<0.0f) cs=0.0f;
		float probReflection=2.0f*cs;

		float k=getReflectionWeight(probLight, probReflection);
		res+=(0.5f*probReflection*k)*(1.0f-params.sssMix)*(params.diffuse*lightColor);
	}

	// Skip this part if specular component is not required
	if (!dontTrace && params.reflectionGlossiness>=0.0f && 0!=(flags & FBRDF_SPECULAR)) {
		float probReflection=getGlossyProbability(direction, rc.rayparams.viewDir);
		float k=getReflectionWeight(probLight, probReflection);
		res+=(0.5f*probReflection*k)*(params.reflection*lightColor);
	}

	lightColor*=params.transparency;
	origLightColor*=params.transparency;

	return res;
}

void MyBaseBSDF::traceForward(VRayContext &rc, int doDiffuse) {
	BRDFSampler::traceForward(rc, doDiffuse);

	if (doDiffuse) rc.mtlresult.color+=rc.evalDiffuse()*params.diffuse*(1.0f-params.sssMix);

	if (params.sssMix>0.0f) {
		VR::Color rawSSSResult=computeRawSSS(rc);
		rc.mtlresult.color+=rawSSSResult*params.sssMix*params.diffuse;
	}
}

static const VR::Color red(1.0f, 0.0f, 0.0f);
static const VR::Color green(0.0f, 1.0f, 0.0f);
static const VR::Color blue(0.0f, 0.0f, 1.0f);

VR::Color MyBaseBSDF::computeRawSSS(VRayContext &rc) {
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

	VR::Color result_sss_direct;
	VR::Color result_sss_indirect;
	
	int directional=true;
	VR::Color result_sss=alsDiffusion(rc, &diffusion_msgdata, directional, nc, params.sssMix, params.diffuse);

	return result_sss;
}

int MyBaseBSDF::getNumSamples(const VRayContext &rc, int doDiffuse) {
	return p_or(rc.rayparams.currentPass==RPASS_GI, (rc.rayparams.rayType & RT_LIGHT)!=0, params.reflectionGlossiness<0.0f)? 0 : nsamples;
}

Color MyBaseBSDF::getTransparency(const VRayContext &rc) {
	return params.transparency;
}

VRayContext* MyBaseBSDF::getNewContext(const VRayContext &rc, int &samplerID, int doDiffuse) {
	if (2==doDiffuse || dontTrace || nsamples==0) return NULL;

	// Create a new context
	VRayContext &nrc=rc.newSpawnContext(2, params.reflection, RT_REFLECT | RT_GLOSSY | RT_ENVIRONMENT, normal);

	// Set up the new context
	nrc.rayparams.dDdx.makeZero(); // Zero out the directional derivatives
	nrc.rayparams.dDdy.makeZero();
	nrc.rayparams.mint=0.0f; // Set the ray extents
	nrc.rayparams.maxt=1e18f;
	nrc.rayparams.tracedRay.p=rc.rayresult.wpoint; // Set the new ray origin to be the surface hit point
	return &nrc;
}

ValidType MyBaseBSDF::setupContext(const VRayContext &rc, VRayContext &nrc, float uc, int doDiffuse) {
	if (params.reflectionGlossiness<0.0f) {
		// Pure reflection
		Vector dir=getReflectDir(rc.rayparams.viewDir, rc.rayresult.normal);

		// If the reflection direction is below the surface, use the geometric normal
		real r0=-(real) (rc.rayparams.viewDir*rc.rayresult.gnormal);
		real r1=(real) (dir*rc.rayresult.gnormal);
		if (r0*r1<0.0f) dir=getReflectDir(rc.rayparams.viewDir, rc.rayresult.gnormal);

		// Set ray derivatives
		VR::getReflectDerivs(rc.rayparams.viewDir, dir, rc.rayparams.dDdx, rc.rayparams.dDdy, nrc.rayparams.dDdx, nrc.rayparams.dDdy);

		// Set the direction into the ray context
		nrc.rayparams.tracedRay.dir=nrc.rayparams.viewDir=dir;
		nrc.rayparams.tracedRay.p=rc.rayresult.wpoint; // Set the new ray origin to be the surface hit point
		nrc.rayparams.mint=0.0f; // Set the ray extents
		nrc.rayparams.maxt=1e18f;
	} else {
		// Compute a Blinn reflection direction
		Vector dir=getGlossyReflectionDir(uc, BRDFSampler::getDMCParam(nrc, 1), rc.rayparams.viewDir, nrc.rayparams.rayProbability);

		// If this is below the surface, ignore
		if (dotf(dir, gnormal)<0.0f) return false;

		// Set ray derivatives
		VR::getReflectDerivs(rc.rayparams.viewDir, dir, rc.rayparams.dDdx, rc.rayparams.dDdy, nrc.rayparams.dDdx, nrc.rayparams.dDdy);

		// Set the direction into the ray context
		nrc.rayparams.tracedRay.dir=nrc.rayparams.viewDir=dir;
	}

	return true;
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
