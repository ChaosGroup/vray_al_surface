#ifndef __BLINN_BRDF_SAMPLER__
#define __BLINN_BRDF_SAMPLER__

#include "brdfs.h"

namespace VUtils {

enum ALSSSMode {
	alSSSMode_diffusion,
	alSSSMode_directional
};

enum ALReflectDistribution {
	alReflectDistribution_beckmann,
	alReflectDistribution_GGX,
};

struct ALBSDFParams {
	Color diffuse;

	Color reflectColor1;
	float reflectRoughness1;
	float reflectIOR1;
	ALReflectDistribution reflectMode1;

	Color reflectColor2;
	float reflectRoughness2;
	float reflectIOR2;
	ALReflectDistribution reflectMode2;
	
	int subdivs;
	Color transparency;
	int doubleSided;

	ALSSSMode sssMode;
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

	Vector diffuseNormal;
	Vector reflectNormal1;
	Vector reflectNormal2;

	LayeredBSDFRenderChannels *renderChannels; // A list of material select render elements.

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

// A temporary structure to hold all the render elements that we want to write and passed to
// MyBaseBSDF::writeElements().
struct RenderElementsResults {
	Color reflection;
	Color rawReflection;
	Color specularLighting;
	Color reflectionFilter;

	Color diffuseFilter;
	Color diffuse;
	Color rawDiffuse;
	Color diffuseLighting;
	Color rawDiffuseLighting;

	Color sss;
	Color rawSSS;

	Color gi, rawGI;

	Color finalColor;
	Color finalShadow;

	RenderElementsResults(void) {}

	void makeZero(void) {
		memset(this, 0, sizeof(*this));
	}
};

// Render elements accumulated in MyBaseBSDF::eval() during direct lighting evaluations.
struct DirectRenderElements {
	Color specularLighting; // Specular contributions from the two reflective layers.
	Color diffuseLighting; // Diffuse contributions from the diffuse layer only.
	Color color; // The final contribution due to direct illumination.
	Color shadow; // Shadow contributions for the diffuse layer only.

	void makeZero(void) {
		memset(this, 0, sizeof(*this));
	}

	void multiply(float mult) {
		specularLighting*=mult;
		diffuseLighting*=mult;
		color*=mult;
		shadow*=mult;
	}

	void add(const DirectRenderElements &other) {
		specularLighting+=other.specularLighting;
		diffuseLighting+=other.diffuseLighting;
		color+=other.color;
		shadow+=other.shadow;
	}
};

class MyBaseBSDF: public BRDFSampler, public BSDFSampler {
protected:
	ALBSDFParams params;

	int dontTrace; // true if we need to trace reflections and false otherwise
	int origBackside;

	int nsamples;

	Vector normal, gnormal;

	Matrix nm1, inm1; // A matrix with the normal as the z-axis; can be used for anisotropy
	Matrix nm2, inm2;

	float viewFresnel1; // An crude estimate of the Fresnel, used in getLightMult().
	float eta1; // Inverse of ior1

	float viewFresnel2;
	float eta2;

	int useMISForDiffuse; // true if the eval() method should use MIS for diffuse illumination.

	int computeSpecular1; // true if primary specular needs to be computed in the eval() method.
	int computeSpecular2; // true if the secondary specular needs to be computed in the eval() method.

	virtual void computeNormalMatrix(const VRayContext &rc, const Vector &normal, Matrix &nm);

	Color computeRawSSS(VRayContext &rc, const Color &diffuse);

	Color computeReflections(VRayContext &rc, float &reflectTransp, Color &reflectionFilter);

	void writeRenderElements(Fragment *f, const RenderElementsResults &renderElements);

	int computeRenderElements;
	DirectRenderElements finalElements, currentElements;
public:
	// Return the params so that they can be set.
	ALBSDFParams& getParams(void) { return params; }

	// Initialization; assumes the params are already set.
	void init(const VRayContext &rc);

	// From BRDFSampler
	Color getDiffuseColor(Color &lightColor) VRAY_OVERRIDE;
	Color getLightMult(Color &lightColor) VRAY_OVERRIDE;
	Color getTransparency(const VRayContext &rc) VRAY_OVERRIDE;
	Vector getDiffuseNormal(const VRayContext &rc) VRAY_OVERRIDE;
	int getUseIrradianceMap(void) VRAY_OVERRIDE;

	Color eval(const VRayContext &rc, const Vector &direction, Color &lightColor, Color &origLightColor, float probLight, int flags) VRAY_OVERRIDE;
	void multiplyLight(float mult) VRAY_OVERRIDE;
	void lightFinished(int last) VRAY_OVERRIDE;
	void traceForward(VRayContext &rc, int doDiffuse) VRAY_OVERRIDE;

	RenderChannelsInfo* getRenderChannels(void);

	// From BSDFSampler
	BRDFSampler *getBRDF(BSDFSide side);

	// Other methods
	const Vector& getGNormal(void) const { return gnormal; }
};

} // namespace VUtils

#endif