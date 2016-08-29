#include "vrayplugins.h"
#include "..\common\albrdf.h"

using namespace VR;

struct BRDFAlSurface_Params: VRayParameterListDesc {
	BRDFAlSurface_Params(void);
};

struct BRDFAlSurface: SimpleBSDF<MyBaseBSDF> {
	BRDFAlSurface(VRayPluginDesc *pluginDesc);
	void initBSDF(const VRayContext &rc, MyBaseBSDF *bsdf, VR::BSDFFlags flags) VRAY_OVERRIDE;
protected:
	TextureFloatInterface *opacity;
	TextureFloatInterface *bumpMap;

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

	Vector getBumpNormal(const VRayContext &rc);
};

#define BRDFAlSurface_PluginID PluginID(LARGE_CONST(2016082656))
PLUGIN_DESC(BRDFAlSurface_PluginID, EXT_BSDF, "BRDFAlSurface", BRDFAlSurface, BRDFAlSurface_Params);
PLUGIN_LIBRARY("BRDFAlSurface", "V-Ray port of the alSurface shader");

//************************************
// Implementation

const float defaultOpacity=1.0f;

const VR::Color defaultDiffuse(0.5f, 0.5f, 0.5f);
const float defaultDiffuseStrength=1.0f;

const Color defaultReflect1(1.0f, 1.0f, 1.0f);
const float defaultReflect1Strength=0.0f;
const float defaultReflect1Roughness=0.5f;
const float defaultReflect1IOR=1.4f;
const int defaultReflect1Distribution=0;

const Color defaultReflect2(1.0f, 1.0f, 1.0f);
const float defaultReflect2Strength=0.0f;
const float defaultReflect2Roughness=0.5f;
const float defaultReflect2IOR=1.4f;
const int defaultReflect2Distribution=0;

const int defaultSSSMode=0;
const float defaultSSSScale=1.0f;
const float defaultSSSMix=0.0f;

const float defaultSSSWeight1=1.0f;
const Color defaultSSSColor1(0.439f, 0.156f, 0.078f);
const float defaultSSSRadius1=1.5f;

const float defaultSSSWeight2=1.0f;
const Color defaultSSSColor2(0.439f, 0.08f, 0.018f);
const float defaultSSSRadius2=4.0f;

const float defaultSSSWeight3=1.0f;
const Color defaultSSSColor3(0.523f, 0.637f, 0.667f);
const float defaultSSSRadius3=0.75f;

BRDFAlSurface_Params::BRDFAlSurface_Params(void) {
	addParamTextureFloat("bump_map", -1, "Overall bump map");
	addParamTextureFloat("opacity", defaultOpacity, -1, "Opacity map");

	addParamTexture("diffuse", defaultDiffuse, -1, "The diffuse surface color");
	addParamTextureFloat("diffuse_strength", defaultDiffuseStrength, -1, "The strength of the diffuse component");

	addParamTexture("reflect1", defaultReflect1, -1, "The first specular color");
	addParamTextureFloat("reflect1_strength", defaultReflect1Strength, -1, "The strength of the first specular component");
	addParamTextureFloat("reflect1_roughness", defaultReflect1Roughness, -1, "The roughness of the first specular component");
	addParamTextureFloat("reflect1_ior", defaultReflect1IOR, -1, "The IOR for the first specular component");
	addParamInt("reflect1_distribution", 0, -1, "The BRDF distribution type for the first specular component (0 - Beckmann, 1 - GGX)");

	addParamTexture("reflect2", defaultReflect2, -1, "The second specular color");
	addParamTextureFloat("reflect2_strength", defaultReflect2Strength, -1, "The strength of the second specular component");
	addParamTextureFloat("reflect2_roughness", defaultReflect2Roughness, -1, "The roughness of the second specular component");
	addParamTextureFloat("reflect2_ior", defaultReflect2IOR, -1, "The IOR for the second specular component");
	addParamInt("reflect2_distribution", 0, -1, "The BRDF distribution type for the second specular component (0 - Beckmann, 1 - GGX)");

	addParamTextureFloat("sss_mix", defaultSSSMix, -1, "Mix between the diffuse component and the SSS component");
	addParamInt("sss_mode", defaultSSSMode, -1, "Sub-surface scattering mode (0 - diffusion; 1 - directional)");
	addParamFloat("sss_density_scale", defaultSSSScale, -1, "Scale for the SSS effect; smaller values make light go deeper into the object");

	addParamTextureFloat("sss1_weight", defaultSSSWeight1, -1, "Weight of the first SSS component");
	addParamTexture("sss1_color", defaultSSSColor1, -1, "Color of the first SSS component");
	addParamTextureFloat("sss1_radius", defaultSSSRadius1, -1, "Radius for the first SSS component. Larger values cause light to go deeper into the surface");

	addParamTextureFloat("sss2_weight", defaultSSSWeight2, -1, "Weight of the second SSS component");
	addParamTexture("sss2_color", defaultSSSColor2, -1, "Color of the second SSS component");
	addParamTextureFloat("sss2_radius", defaultSSSRadius2, -1, "Radius for the second SSS component. Larger values cause light to go deeper into the surface");

	addParamTextureFloat("sss3_weight", defaultSSSWeight3, -1, "Weight of the third SSS component");
	addParamTexture("sss3_color", defaultSSSColor3, -1, "Color of the third SSS component");
	addParamTextureFloat("sss3_radius", defaultSSSRadius3, -1, "Radius for the third SSS component. Larger values cause light to go deeper into the surface");
}

BRDFAlSurface::BRDFAlSurface(VRayPluginDesc *pluginDesc):SimpleBSDF<MyBaseBSDF>(pluginDesc) {
	paramList->setParamCache("bump_map", &bumpMap);
	paramList->setParamCache("opacity", &opacity);

	paramList->setParamCache("diffuse", &diffuse);
	paramList->setParamCache("diffuse_strength", &diffuseStrength);

	paramList->setParamCache("reflect1", &reflect[0]);
	paramList->setParamCache("reflect1_strength", &reflectStrength[0]);
	paramList->setParamCache("reflect1_roughness", &reflectRoughness[0]);
	paramList->setParamCache("reflect1_ior", &reflectIOR[0]);
	paramList->setParamCache("reflect1_distribution", &reflectDistribution[0]);

	paramList->setParamCache("reflect2", &reflect[1]);
	paramList->setParamCache("reflect2_strength", &reflectStrength[1]);
	paramList->setParamCache("reflect2_roughness", &reflectRoughness[1]);
	paramList->setParamCache("reflect2_ior", &reflectIOR[1]);
	paramList->setParamCache("reflect2_distribution", &reflectDistribution[1]);

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
}

Vector BRDFAlSurface::getBumpNormal(const VRayContext &rc) {
	return rc.rayresult.normal;
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

void BRDFAlSurface::initBSDF(const VRayContext &rc, MyBaseBSDF *bsdf, VR::BSDFFlags flags) {
	VR::ALBSDFParams &params=bsdf->getParams();

	// Compute the bumped normal
	VR::Vector bumpNormal=getBumpNormal(rc);

	// Before evaluating any textures, we must set the bumped normal as some
	// textures may depend on it.
	VR::VRayContext &rcc=const_cast<VR::VRayContext&>(rc);
	VR::Vector origNormal=rcc.rayresult.normal;
	rcc.rayresult.normal=bumpNormal;

	params.diffuse=getTexture(rc, diffuse, defaultDiffuse);
	params.diffuse*=getTexture(rc, diffuseStrength, defaultDiffuseStrength);

	params.reflectColor1=getTexture(rc, reflect[0], defaultReflect1);
	params.reflectColor1*=getTexture(rc, reflectStrength[0], defaultReflect1Strength);
	params.reflectRoughness1=getTexture(rc, reflectRoughness[0], defaultReflect1Roughness);
	params.reflectIOR1=getTexture(rc, reflectIOR[0], defaultReflect1IOR, false /* clamp */);
	params.reflectMode1=(reflectDistribution[0]==0? VR::alReflectDistribution_beckmann : VR::alReflectDistribution_GGX);

	params.reflectColor2=getTexture(rc, reflect[1], defaultReflect2);
	params.reflectColor2*=getTexture(rc, reflectStrength[1], defaultReflect2Strength);
	params.reflectRoughness2=getTexture(rc, reflectRoughness[1], defaultReflect2Roughness);
	params.reflectIOR2=getTexture(rc, reflectIOR[1], defaultReflect2IOR, false /* clamp */);
	params.reflectMode2=(reflectDistribution[1]==0? VR::alReflectDistribution_beckmann : VR::alReflectDistribution_GGX);

	params.transparency=1.0f-getTexture(rc, opacity, defaultOpacity);
	params.subdivs=8;
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

	params.normalizeWeights();

	bsdf->init(rc);

	// Restore the normal
	rcc.rayresult.normal=origNormal;
}
