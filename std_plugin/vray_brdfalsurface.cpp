#include "vrayplugins.h"
#include "../common/albrdf.h"
#include "vraytexutils.h"
#include "bump_brdf_impl.h"

using namespace VR;

struct BRDFAlSurface_Params: VRayParameterListDesc {
	BRDFAlSurface_Params(void);
};

struct BRDFAlSurface: SimpleBSDF<MyBaseBSDF> {
	BRDFAlSurface(VRayPluginDesc *pluginDesc);
	void initBSDF(const VRayContext &rc, MyBaseBSDF *bsdf, VR::BSDFFlags flags) VRAY_OVERRIDE;
	int isOpaque(void) VRAY_OVERRIDE;

	void frameBegin(VRayRenderer *vray) VRAY_OVERRIDE;
protected:
	TextureFloatInterface *opacity;
	BumpParams bumpParams;
	BumpParams diffuseBumpParams;
	BumpParams reflectBumpParams[2];

	TextureInterface *diffuse;
	TextureFloatInterface *diffuseStrength;

	TextureInterface *reflect[2];
	TextureFloatInterface *reflectStrength[2];
	TextureFloatInterface *reflectRoughness[2];
	TextureFloatInterface *reflectIOR[2];
	int reflectDistribution[2];

	TextureFloatInterface *sssMix;
	int sssMode;
	float sssDensityScale;

	TextureFloatInterface *sssWeight[3];
	TextureInterface *sssColor[3];
	TextureFloatInterface *sssRadius[3];
};

#define BRDFAlSurface_PluginID PluginID(LARGE_CONST(2016082656))
PLUGIN_DESC(BRDFAlSurface_PluginID, EXT_BSDF, "BRDFAlSurface", BRDFAlSurface, BRDFAlSurface_Params);
PLUGIN_LIBRARY("BRDFAlSurface", "V-Ray port of the alSurface shader");

//************************************
// Implementation

#include "../common/default_values.h"

BRDFAlSurface_Params::BRDFAlSurface_Params(void) {
	addParamTexture("bump_map", Color(0.0f, 0.0f, 0.0f), -1, "Overall bump map (applied to all components)");
	addParamTextureFloat("bump_amount", 1.0f, -1, "Bump amount");
	addParamInt("bump_type", 0, -1, VRAY_BUMP_BRDF_MAP_TYPE_DESC, VRAY_BUMP_BRDF_MAP_TYPE_UIGUIDES);
	addParamTextureFloat("opacity", defaultOpacity, -1, "Opacity map", "minValue=(0), maxValue=(1)");

	addParamTexture("diffuse", defaultDiffuse, -1, "The diffuse surface color");
	addParamTextureFloat("diffuse_strength", defaultDiffuseStrength, -1, "The strength of the diffuse component", "minValue=(0), maxValue=(1)");
	addParamTexture("diffuse_bump_map", Color(0.0f, 0.0f, 0.0f), -1, "Additoinal diffuse bump map");
	addParamTextureFloat("diffuse_bump_amount", 1.0f, -1, "Diffuse bump amount");
	addParamInt("diffuse_bump_type", 0, -1, VRAY_BUMP_BRDF_MAP_TYPE_DESC, VRAY_BUMP_BRDF_MAP_TYPE_UIGUIDES);

	addParamTexture("reflect1", defaultReflect1, -1, "The first specular color");
	addParamTextureFloat("reflect1_strength", defaultReflect1Strength, -1, "The strength of the first specular component", "minValue=(0), maxValue=(1)");
	addParamTextureFloat("reflect1_roughness", defaultReflect1Roughness, -1, "The roughness of the first specular component", "minValue=(0), maxValue=(1)");
	addParamTextureFloat("reflect1_ior", defaultReflect1IOR, -1, "The IOR for the first specular component");
	addParamInt("reflect1_distribution", 0, -1, "The BRDF distribution type for the first specular component (0 - Beckmann, 1 - GGX)", "enum=(0:Beckmann;1:GGX)");
	addParamTexture("reflect1_bump_map", Color(0.0f, 0.0f, 0.0f), -1, "Additional bump map for the first specular component");
	addParamTextureFloat("reflect1_bump_amount", 1.0f, -1, "Additional bump amount for the first speculer component");
	addParamInt("reflect1_bump_type", 0, -1, VRAY_BUMP_BRDF_MAP_TYPE_DESC, VRAY_BUMP_BRDF_MAP_TYPE_UIGUIDES);

	addParamTexture("reflect2", defaultReflect2, -1, "The second specular color");
	addParamTextureFloat("reflect2_strength", defaultReflect2Strength, -1, "The strength of the second specular component", "minValue=(0), maxValue=(1)");
	addParamTextureFloat("reflect2_roughness", defaultReflect2Roughness, -1, "The roughness of the second specular component", "minValue=(0), maxValue=(1)");
	addParamTextureFloat("reflect2_ior", defaultReflect2IOR, -1, "The IOR for the second specular component");
	addParamInt("reflect2_distribution", 0, -1, "The BRDF distribution type for the second specular component (0 - Beckmann, 1 - GGX)", "enum=(0:Beckmann;1:GGX)");
	addParamTexture("reflect2_bump_map", Color(0.0f, 0.0f, 0.0f), -1, "Additional bump map for the second specular component");
	addParamTextureFloat("reflect2_bump_amount", 1.0f, -1, "Additional bump amount for the second speculer component");
	addParamInt("reflect2_bump_type", 0, -1, VRAY_BUMP_BRDF_MAP_TYPE_DESC, VRAY_BUMP_BRDF_MAP_TYPE_UIGUIDES);

	addParamTextureFloat("sss_mix", defaultSSSMix, -1, "Mix between the diffuse component and the SSS component", "minValue=(0), maxValue=(1)");
	addParamInt("sss_mode", defaultSSSMode, -1, "Sub-surface scattering mode (0 - diffusion; 1 - directional)", "enum=(0:Diffusion;1:Directional)");
	addParamFloat("sss_density_scale", defaultSSSScale, -1, "Scale for the SSS effect; smaller values make light go deeper into the object");

	addParamTextureFloat("sss1_weight", defaultSSSWeight1, -1, "Weight of the first SSS component", "minValue=(0), maxValue=(10), softMinValue=(0), softMaxValue=(1)");
	addParamTexture("sss1_color", defaultSSSColor1, -1, "Color of the first SSS component");
	addParamTextureFloat("sss1_radius", defaultSSSRadius1, -1, "Radius for the first SSS component. Larger values cause light to go deeper into the surface");

	addParamTextureFloat("sss2_weight", defaultSSSWeight2, -1, "Weight of the second SSS component", "minValue=(0), maxValue=(10), softMinValue=(0), softMaxValue=(1)");
	addParamTexture("sss2_color", defaultSSSColor2, -1, "Color of the second SSS component");
	addParamTextureFloat("sss2_radius", defaultSSSRadius2, -1, "Radius for the second SSS component. Larger values cause light to go deeper into the surface");

	addParamTextureFloat("sss3_weight", defaultSSSWeight3, -1, "Weight of the third SSS component", "minValue=(0), maxValue=(10), softMinValue=(0), softMaxValue=(1)");
	addParamTexture("sss3_color", defaultSSSColor3, -1, "Color of the third SSS component");
	addParamTextureFloat("sss3_radius", defaultSSSRadius3, -1, "Radius for the third SSS component. Larger values cause light to go deeper into the surface");
}

BRDFAlSurface::BRDFAlSurface(VRayPluginDesc *pluginDesc):SimpleBSDF<MyBaseBSDF>(pluginDesc) {
	paramList->setParamCache("bump_map", &bumpParams.bumpTexColor);
	paramList->setParamCache("bump_amount", &bumpParams.bumpMultTex);
	paramList->setParamCache("bump_type", &bumpParams.mapType);
	paramList->setParamCache("opacity", &opacity);

	paramList->setParamCache("diffuse", &diffuse);
	paramList->setParamCache("diffuse_strength", &diffuseStrength);
	paramList->setParamCache("diffuse_bump_map", &diffuseBumpParams.bumpTexColor);
	paramList->setParamCache("diffuse_bump_amount", &diffuseBumpParams.bumpMultTex);
	paramList->setParamCache("diffuse_bump_type", &diffuseBumpParams.mapType);

	paramList->setParamCache("reflect1", &reflect[0]);
	paramList->setParamCache("reflect1_strength", &reflectStrength[0]);
	paramList->setParamCache("reflect1_roughness", &reflectRoughness[0]);
	paramList->setParamCache("reflect1_ior", &reflectIOR[0]);
	paramList->setParamCache("reflect1_distribution", &reflectDistribution[0]);
	paramList->setParamCache("reflect1_bump_map", &reflectBumpParams[0].bumpTexColor);
	paramList->setParamCache("reflect1_bump_amount", &reflectBumpParams[0].bumpMultTex);
	paramList->setParamCache("reflect1_bump_type", &reflectBumpParams[0].mapType);

	paramList->setParamCache("reflect2", &reflect[1]);
	paramList->setParamCache("reflect2_strength", &reflectStrength[1]);
	paramList->setParamCache("reflect2_roughness", &reflectRoughness[1]);
	paramList->setParamCache("reflect2_ior", &reflectIOR[1]);
	paramList->setParamCache("reflect2_distribution", &reflectDistribution[1]);
	paramList->setParamCache("reflect2_bump_map", &reflectBumpParams[1].bumpTexColor);
	paramList->setParamCache("reflect2_bump_amount", &reflectBumpParams[1].bumpMultTex);
	paramList->setParamCache("reflect2_bump_type", &reflectBumpParams[1].mapType);

	paramList->setParamCache("sss_mix", &sssMix);
	paramList->setParamCache("sss_mode", &sssMode);
	paramList->setParamCache("sss_density_scale", &sssDensityScale);

	paramList->setParamCache("sss1_weight", &sssWeight[0]);
	paramList->setParamCache("sss1_color", &sssColor[0]);
	paramList->setParamCache("sss1_radius", &sssRadius[0]);

	paramList->setParamCache("sss2_weight", &sssWeight[1]);
	paramList->setParamCache("sss2_color", &sssColor[1]);
	paramList->setParamCache("sss2_radius", &sssRadius[1]);

	paramList->setParamCache("sss3_weight", &sssWeight[2]);
	paramList->setParamCache("sss3_color", &sssColor[2]);
	paramList->setParamCache("sss3_radius", &sssRadius[2]);

	// We only have a bump texture, and it is multiplied by the bumpMult, so set the
	// bumpMult to 1.0 here.
	bumpParams.bumpMult=1.0f;
	diffuseBumpParams.bumpMult=1.0f;
	reflectBumpParams[0].bumpMult=1.0f;
	reflectBumpParams[1].bumpMult=1.0f;
}

// Check if the color texture in the given bump params is a constant color
// and if yes - remove it. This is in order to prevent odd renders if the user
// accidentally sets some color and leaves the type to some kind of normal map.
void checkBumpTex(BumpParams &bumpParams) {
	if (bumpParams.bumpTexColor) {
		AColor tmin(0.0f, 0.0f, 0.0f, 0.0f);
		AColor tmax(1.0f, 1.0f, 1.0f, 1.0f);
		bumpParams.bumpTexColor->getTexColorBounds(tmin, tmax);
		if ((tmax-tmin).lengthSqr()<1e-12f) {
			bumpParams.bumpTexColor=NULL;
		}
	}
}

void BRDFAlSurface::frameBegin(VRayRenderer *vray) {
	SimpleBSDF<MyBaseBSDF>::frameBegin(vray);

	// Clear any constant bump maps.
	checkBumpTex(bumpParams);
	checkBumpTex(diffuseBumpParams);
	checkBumpTex(reflectBumpParams[0]);
	checkBumpTex(reflectBumpParams[1]);
}

inline Color getTexture(const VRayContext &rc, TextureInterface *texmap, const VR::Color &defaultValue) {
	if (!texmap)
		return defaultValue;

	VR::Color color=texmap->getTexColor(rc).color;
	color.clampMinMax();
	return color;
}

inline float getTexture(const VRayContext &rc, TextureFloatInterface *texmap, float defaultValue, int clampValue=true) {
	if (!texmap)
		return defaultValue;

	float value=texmap->getTexFloat(rc);
	if (clampValue)
		value=clamp(value, 0.0f, 1.0f);

	return value;
}

inline Vector applyBumpToNormal(const Vector &normal, const Vector &bumpVector, int mapType, int normalFlipped) {
	Vector res;
	if (mapType == BUMP_MAP_NONE) {
		// Do nothing
		res=normal;
	} else if (mapType == BUMP_MAP_NORMAL_SET) {
		// Directly set the normal
		if (!normalFlipped) res=bumpVector;
		else res=-bumpVector;
	} else if (mapType == BUMP_MAP_BUMP) {
		// Bump based on rotation
		if (!normalFlipped) res=rotate(normal, -bumpVector);
		else res=rotate(normal, bumpVector);
	} else /*if (mapType == BUMP_MAP_BUMP_LOCAL)*/ {
		// Bump map based on addition
		if (!normalFlipped) res=normal+bumpVector;
		else res=normal-bumpVector;
		res.makeNormalized0();
	}

	return res;
}

inline Vector getBumpNormal(const BumpParams &bumpParams, const VRayContext &rc) {
	Vector res;
	if (!bumpParams.bumpTexColor) {
		res=rc.rayresult.normal;
	} else {
		VR::Vector bumpVector(0.0f, 0.0f, 0.0f), bumpElevation(0.0f, 0.0f, 0.0f);
		bumpParams.getBump(rc, bumpVector, bumpElevation);
		res=applyBumpToNormal(rc.rayresult.normal, bumpVector, bumpParams.mapType, rc.rayresult.normalFlipped);
	}
	return res;
}

void BRDFAlSurface::initBSDF(const VRayContext &rc, MyBaseBSDF *bsdf, VR::BSDFFlags flags) {
	VR::ALBSDFParams &params=bsdf->getParams();

	// Compute the bumped normal
	VR::Vector bumpVector(0.0f, 0.0f, 0.0f), bumpElevation(0.0f, 0.0f, 0.0f);
	bumpParams.getBump(rc, bumpVector, bumpElevation);

	// Save the current normal and hit point so that we can restore them later, in case they
	// are modified by bump mapping.
	Vector origNormal=rc.rayresult.normal;
	TracePoint origPoint=rc.rayresult.wpoint;

	// Before evaluating any textures, we must set the bumped normal as some
	// textures may depend on it.
	VRayContext &rcc=const_cast<VR::VRayContext&>(rc);
	applyBump(rcc, bumpVector, bumpElevation, bumpParams.mapType, rcc.rayresult.realBack);

	params.diffuse=getTexture(rc, diffuse, defaultDiffuse);
	params.diffuse*=getTexture(rc, diffuseStrength, defaultDiffuseStrength);
	params.diffuseNormal=getBumpNormal(diffuseBumpParams, rcc);

	params.reflectColor1=getTexture(rc, reflect[0], defaultReflect1);
	params.reflectColor1*=getTexture(rc, reflectStrength[0], defaultReflect1Strength);
	params.reflectRoughness1=getTexture(rc, reflectRoughness[0], defaultReflect1Roughness);
	params.reflectIOR1=getTexture(rc, reflectIOR[0], defaultReflect1IOR, false /* clamp */);
	params.reflectMode1=(reflectDistribution[0]==0? VR::alReflectDistribution_beckmann : VR::alReflectDistribution_GGX);
	params.reflectNormal1=getBumpNormal(reflectBumpParams[0], rcc);

	params.reflectColor2=getTexture(rc, reflect[1], defaultReflect2);
	params.reflectColor2*=getTexture(rc, reflectStrength[1], defaultReflect2Strength);
	params.reflectRoughness2=getTexture(rc, reflectRoughness[1], defaultReflect2Roughness);
	params.reflectIOR2=getTexture(rc, reflectIOR[1], defaultReflect2IOR, false /* clamp */);
	params.reflectMode2=(reflectDistribution[1]==0? VR::alReflectDistribution_beckmann : VR::alReflectDistribution_GGX);
	params.reflectNormal2=getBumpNormal(reflectBumpParams[1], rcc);

	params.transparency=1.0f-getTexture(rc, opacity, defaultOpacity);
	params.doubleSided=true;

	params.sssMode=(sssMode==0? VR::alSSSMode_diffusion : VR::alSSSMode_directional);
	params.sssMix=getTexture(rc, sssMix, defaultSSSMix);
	params.sssDensityScale=sssDensityScale;

	params.sssWeight1=getTexture(rc, sssWeight[0], defaultSSSWeight1);
	params.sssColor1=getTexture(rc, sssColor[0], defaultSSSColor1);
	params.sssRadius1=getTexture(rc, sssRadius[0], defaultSSSRadius1, false  /* clamp */);

	params.sssWeight2=getTexture(rc, sssWeight[1], defaultSSSWeight2);
	params.sssColor2=getTexture(rc, sssColor[1], defaultSSSColor2);
	params.sssRadius2=getTexture(rc, sssRadius[1], defaultSSSRadius2, false  /* clamp */);

	params.sssWeight3=getTexture(rc, sssWeight[2], defaultSSSWeight3);
	params.sssColor3=getTexture(rc, sssColor[2], defaultSSSColor3);
	params.sssRadius3=getTexture(rc, sssRadius[2], defaultSSSRadius3, false  /* clamp */);

	params.renderChannels=NULL;

	params.reflectMaxDepth=defaultReflectMaxDepth;
	params.reflectSubdivs=defaultReflectSubdivs;
	params.sssSubdivs=defaultSSSSubdivs;

	params.normalizeWeights();

	bsdf->init(rc);

	// Restore the normal
	rcc.rayresult.normal=origNormal;
	rcc.rayresult.wpoint=origPoint;
}

int BRDFAlSurface::isOpaque(void) {
	int isSolid=false;
	if (!opacity) {
		isSolid=true;
	} else {
		float tmin=0.0f, tmax=1.0f;
		opacity->getTexFloatBounds(tmin, tmax);
		if (tmin>1.0f-1e-6f && tmax>1.0f-1e-6f)
			isSolid=true;
	}

	int res=0;
	if (isSolid) res|=MTL_OPAQUE_FOR_SHADOWS;

	return res;
}
