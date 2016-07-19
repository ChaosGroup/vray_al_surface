#ifndef __BLINN_BRDF_SAMPLER__
#define __BLINN_BRDF_SAMPLER__

namespace VUtils {

struct ALBSDFParams {
	Color diffuse;

	Color reflectColor1;
	float reflectRoughness1;
	float reflectIOR1;

	Color reflectColor2;
	float reflectRoughness2;
	float reflectIOR2;
	
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

	void normalizeWeights(void) {
		float sssSum=sssWeight1+sssWeight2+sssWeight3;
		if (sssSum>0.0f) {
			float normCoeff=1.0f/sssSum;
			sssWeight1*=normCoeff;
			sssWeight2*=normCoeff;
			sssWeight3*=normCoeff;
		}
	}
};

inline float fresnel(float cosi, float etai) {
	if (cosi >= 1.0f) return 0.0f;
	float sint = etai * sqrtf(1.0f-cosi*cosi);
	if ( sint >= 1.0f ) return 1.0f;

	float cost = sqrtf(1.0f-sint*sint);
	float pl = (cosi - (etai * cost)) / (cosi + (etai * cost));
	float pp = ((etai * cosi) - cost) / ((etai * cosi) + cost);
	return (pl*pl+pp*pp)*0.5f;
}

inline float computeDielectricFresnel(const Vector &viewDir, const Vector &lightDir, const Vector &normal, float ior) {
	Vector h=VR::normalize0(lightDir-viewDir);
	float csh=VR::Max(0.0f, -dotf(h, viewDir));
	return fresnel(csh, ior);
}

class MyBaseBSDF: public BRDFSampler, public BSDFSampler {
protected:
	ALBSDFParams params;

	int dontTrace; // true if we need to trace reflections and false otherwise
	int origBackside;

	int nsamples;

	Vector normal, gnormal;

	Matrix nm, inm; // A matrix with the normal as the z-axis; can be used for anisotropy

	float viewFresnel1; // An crude estimate of the Fresnel, used in getLightMult().
	float eta1; // Inverse of ior1

	float viewFresnel2;
	float eta2;

	int useMISForDiffuse; // true if the eval() method should use MIS for diffuse illumination.

	virtual void computeNormalMatrix(const VRayContext &rc, const Vector &normal, Matrix &nm);

	Color computeRawSSS(VRayContext &rc, const Color &diffuse);

	Color computeReflections(VRayContext &rc, float &reflectTransp);
public:
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

	RenderChannelsInfo* getRenderChannels(void);

	// From BSDFSampler
	BRDFSampler *getBRDF(BSDFSide side);

	// Other methods
	const Vector& getNormal(void) const { return normal; }
	const Vector& getGNormal(void) const { return gnormal; }
	const Matrix& getNormalMatrix(void) const { return nm; }
	const Matrix& getNormalMatrixInv(void) const { return inm; }
	float getEta1(void) const { return eta1; }
};

} // namespace VUtils

#endif