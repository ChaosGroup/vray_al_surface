#ifndef __BLINN_BRDF_SAMPLER__
#define __BLINN_BRDF_SAMPLER__

namespace VUtils {

struct ALBSDFParams {
	Color diffuse;
	Color reflection;
	float reflectionGlossiness;
	int subdivs;
	Color transparency;
	int doubleSided;
	float sssMix;

	float sssWeight1;
	Color sssColor1;
	float sssRadius1;

	float sssWeight2;
	Color sssColor2;
	float sssRadius2;

	float sssWeight3;
	Color sssColor3;
	float sssRadius3;

	float sssDensityScale;
};

class MyBaseBSDF: public BRDFSampler, public BSDFSampler {
protected:
	ALBSDFParams params;

	int dontTrace; // true if we need to trace reflections and false otherwise
	int origBackside;

	int nsamples;

	Vector normal, gnormal;

	Matrix nm, inm; // A matrix with the normal as the z-axis; can be used for anisotropy

	virtual void computeNormalMatrix(const VRayContext &rc, const Vector &normal, Matrix &nm);

	Color computeRawSSS(VRayContext &rc);
public:
	// Methods required for derived classes
	virtual Vector getGlossyReflectionDir(float uc, float vc, const Vector &viewDir, float &rayProbability)=0;
	virtual VUtils::real getGlossyProbability(const Vector &direction, const Vector &viewDir)=0;
	virtual float remapGlossiness(float nk)=0;

	// Return the params so that they can be set.
	ALBSDFParams& getParams(void) { return params; }

	// Initialization; assumes the params are already set.
	void init(const VRayContext &rc);

	// From BRDFSampler
	Color getDiffuseColor(Color &lightColor);
	Color getLightMult(Color &lightColor);
	Color getTransparency(const VRayContext &rc);

	Color eval(const VRayContext &rc, const Vector &direction, Color &lightColor, Color &origLightColor, float probLight, int flags);
	void traceForward(VRayContext &rc, int doDiffuse);

	int getNumSamples(const VRayContext &rc, int doDiffuse);
	VRayContext* getNewContext(const VRayContext &rc, int &samplerID, int doDiffuse);
	ValidType setupContext(const VRayContext &rc, VRayContext &nrc, float uc, int doDiffuse);

	RenderChannelsInfo* getRenderChannels(void);

	// From BSDFSampler
	BRDFSampler *getBRDF(BSDFSide side);
};

} // namespace VUtils

#endif