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

protected:
	TextureFloatInterface *opacity;
	BumpParams bumpParams;

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
	addParamTextureFloat("bump_map", -1, "Overall bump map");
	addParamTextureFloat("bump_amount", 1.0f, -1, "Bump amount");
	addParamInt("bump_type", 0, -1, VRAY_BUMP_BRDF_MAP_TYPE_DESC, VRAY_BUMP_BRDF_MAP_TYPE_UIGUIDES);
	addParamTextureFloat("opacity", defaultOpacity, -1, "Opacity map", "minValue=(0), maxValue=(1)");

	addParamTexture("diffuse", defaultDiffuse, -1, "The diffuse surface color");
	addParamTextureFloat("diffuse_strength", defaultDiffuseStrength, -1, "The strength of the diffuse component", "minValue=(0), maxValue=(1)");

	addParamTexture("reflect1", defaultReflect1, -1, "The first specular color");
	addParamTextureFloat("reflect1_strength", defaultReflect1Strength, -1, "The strength of the first specular component", "minValue=(0), maxValue=(1)");
	addParamTextureFloat("reflect1_roughness", defaultReflect1Roughness, -1, "The roughness of the first specular component", "minValue=(0), maxValue=(1)");
	addParamTextureFloat("reflect1_ior", defaultReflect1IOR, -1, "The IOR for the first specular component");
	addParamInt("reflect1_distribution", 0, -1, "The BRDF distribution type for the first specular component (0 - Beckmann, 1 - GGX)", "enum=(0:Beckmann;1:GGX)");

	addParamTexture("reflect2", defaultReflect2, -1, "The second specular color");
	addParamTextureFloat("reflect2_strength", defaultReflect2Strength, -1, "The strength of the second specular component", "minValue=(0), maxValue=(1)");
	addParamTextureFloat("reflect2_roughness", defaultReflect2Roughness, -1, "The roughness of the second specular component", "minValue=(0), maxValue=(1)");
	addParamTextureFloat("reflect2_ior", defaultReflect2IOR, -1, "The IOR for the second specular component");
	addParamInt("reflect2_distribution", 0, -1, "The BRDF distribution type for the second specular component (0 - Beckmann, 1 - GGX)", "enum=(0:Beckmann;1:GGX)");

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
	paramList->setParamCache("bump_map", &bumpParams.bumpTexFloat);
	paramList->setParamCache("bump_amount", &bumpParams.bumpMultTex);
	paramList->setParamCache("bump_type", &bumpParams.mapType);
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

	// We only have a bump texture, and it is multiplied by the bumpMult, so set the
	// bumpMult to 1.0 here.
	bumpParams.bumpMult=1.0f;
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

	// For the moment just use the same bump normal for all layers.
	params.diffuseNormal=params.reflectNormal1=params.reflectNormal2=rc.rayresult.normal;

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
