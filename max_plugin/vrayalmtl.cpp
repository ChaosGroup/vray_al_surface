#include "max.h"

#include "vrayinterface.h"
#include "vrender_unicode.h"

#include "vrayalmtl.h"
#include "pb2template_generator.h"

#define CURRENT_VERSION 1

struct ALMtlTexInfo {
	ParamID texID;
	ParamID texOnID;
	ParamID texMultID;
	ParamID texShortID;
	int shortCtrlID;
	const TCHAR *texName;
};

ALMtlTexInfo texInfo[NSUBTEX]={
	{ mtl_opacity_tex, mtl_opacity_tex_on, mtl_opacity_tex_mult, mtl_opacity_tex_shortmap, -1, _T("opacity_texture") },
	{ mtl_bump_tex, mtl_bump_tex_on, mtl_bump_tex_mult, -1, -1, _T("bump_texture") },
	{ mtl_displacement_tex, mtl_displacement_tex_on, mtl_displacement_tex_mult, -1, -1, _T("displacement_texture") },
	
	{ mtl_diffuse_tex, mtl_diffuse_tex_on, mtl_diffuse_tex_mult, mtl_diffuse_tex_shortmap, -1, _T("diffuse_texture") },
	{ mtl_diffuse_strength_tex, mtl_diffuse_strength_tex_on, mtl_diffuse_strength_tex_mult, mtl_diffuse_strength_tex_shortmap, -1, _T("diffuse_strength_texture") },

	{ mtl_reflect_color1_tex, mtl_reflect_color1_tex_on, mtl_reflect_color1_tex_mult, mtl_reflect_color1_tex_shortmap, -1, _T("reflection1_texture") },
	{ mtl_reflect_strength1_tex, mtl_reflect_strength1_tex_on, mtl_reflect_strength1_tex_mult, mtl_reflect_strength1_tex_shortmap, -1, _T("reflection1_strength_texture") },
	{ mtl_reflect_roughness1_tex, mtl_reflect_roughness1_tex_on, mtl_reflect_roughness1_tex_mult, mtl_reflect_roughness1_tex_shortmap, -1, _T("reflection1_roughness_texture") },
	{ mtl_reflect_ior1_tex, mtl_reflect_ior1_tex_on, mtl_reflect_ior1_tex_mult, mtl_reflect_ior1_tex_shortmap, -1, _T("reflection1_ior_texture") },

	{ mtl_reflect_color2_tex, mtl_reflect_color2_tex_on, mtl_reflect_color2_tex_mult, mtl_reflect_color2_tex_shortmap, -1, _T("reflecton2_texture") },
	{ mtl_reflect_strength2_tex, mtl_reflect_strength2_tex_on, mtl_reflect_strength2_tex_mult, mtl_reflect_strength2_tex_shortmap, -1, _T("reflection2_strength_texture") },
	{ mtl_reflect_roughness2_tex, mtl_reflect_roughness2_tex_on, mtl_reflect_roughness2_tex_mult, mtl_reflect_roughness2_tex_shortmap, -1, _T("reflection2_roughness_texture") },
	{ mtl_reflect_ior2_tex, mtl_reflect_ior2_tex_on, mtl_reflect_ior2_tex_mult, mtl_reflect_ior2_tex_shortmap, -1, _T("reflection2_ior_texture") },

	{ mtl_sss_mix_tex, mtl_sss_mix_tex_on, mtl_sss_mix_tex_mult, mtl_sss_mix_tex_shortmap, -1, _T("sss_mix_texture") },

	{ mtl_sss_color1_tex, mtl_sss_color1_tex_on, mtl_sss_color1_tex_mult, mtl_sss_color1_tex_shortmap, -1, _T("sss1_color_texture") },
	{ mtl_sss_weight1_tex, mtl_sss_weight1_tex_on, mtl_sss_weight1_tex_mult, mtl_sss_weight1_tex_shortmap, -1, _T("sss1_weight_texture") },
	{ mtl_sss_radius1_tex, mtl_sss_radius1_tex_on, mtl_sss_radius1_tex_mult, mtl_sss_radius1_tex_shortmap, -1, _T("sss1_radius_texture") },

	{ mtl_sss_color2_tex, mtl_sss_color2_tex_on, mtl_sss_color2_tex_mult, mtl_sss_color2_tex_shortmap, -1, _T("sss2_color_texture") },
	{ mtl_sss_weight2_tex, mtl_sss_weight2_tex_on, mtl_sss_weight2_tex_mult, mtl_sss_weight2_tex_shortmap, -1, _T("sss2_weight_texture") },
	{ mtl_sss_radius2_tex, mtl_sss_radius2_tex_on, mtl_sss_radius2_tex_mult, mtl_sss_radius2_tex_shortmap, -1, _T("sss2_radius_texture") },

	{ mtl_sss_color3_tex, mtl_sss_color3_tex_on, mtl_sss_color3_tex_mult, mtl_sss_color3_tex_shortmap, -1, _T("sss3_color_texture") },
	{ mtl_sss_weight3_tex, mtl_sss_weight3_tex_on, mtl_sss_weight3_tex_mult, mtl_sss_weight3_tex_shortmap, -1, _T("sss3_weight_texture") },
	{ mtl_sss_radius3_tex, mtl_sss_radius3_tex_on, mtl_sss_radius3_tex_mult, mtl_sss_radius3_tex_shortmap, -1, _T("sss3_radius_texture") },

	{ mtl_diffuse_bump_tex, mtl_diffuse_bump_tex_on, mtl_diffuse_bump_tex_mult, -1, -1, _T("diffuse_bump_texture") },
	{ mtl_reflect_bump1_tex, mtl_reflect_bump1_tex_on, mtl_reflect_bump1_tex_mult, -1, -1, _T("reflection1_bump_texture") },
	{ mtl_reflect_bump2_tex, mtl_reflect_bump2_tex_on, mtl_reflect_bump2_tex_mult, -1, -1, _T("reflection2_bump_texture") }
};

/*===========================================================================*\
 |	Class Descriptor
\*===========================================================================*/

class SkeletonMaterialClassDesc: public ClassDesc2
#if GET_MAX_RELEASE(VERSION_3DSMAX) >= 6000
, public IMtlRender_Compatibility_MtlBase 
#endif
#if GET_MAX_RELEASE(VERSION_3DSMAX) >= 13900
, public IMaterialBrowserEntryInfo
#endif
{
	HIMAGELIST imageList;
public:
	int IsPublic() { return 1; }
	void* Create(BOOL loading) { return new SkeletonMaterial(loading); }
	const TCHAR *	ClassName() { return STR_CLASSNAME; }
	SClass_ID SuperClassID() { return MATERIAL_CLASS_ID; }
	Class_ID ClassID() { return MTL_CLASSID; }
	const TCHAR* Category() { return _T("");  }

	// Hardwired name, used by MAX Script as unique identifier
	const TCHAR* InternalName() { return STR_CLASSNAME; }
	HINSTANCE HInstance() { return hInstance; }

	SkeletonMaterialClassDesc(void) {
		imageList=NULL;
#if GET_MAX_RELEASE(VERSION_3DSMAX) >= 6000
		IMtlRender_Compatibility_MtlBase::Init(*this);
#endif
	}

	~SkeletonMaterialClassDesc(void) {
		if (imageList) ImageList_Destroy(imageList);
		imageList=NULL;
	}

#if GET_MAX_RELEASE(VERSION_3DSMAX) >= 6000
	// From IMtlRender_Compatibility_MtlBase
	bool IsCompatibleWithRenderer(ClassDesc& rendererClassDesc) {
		if (rendererClassDesc.ClassID()!=VRENDER_CLASS_ID) return false;
		return true;
	}
	bool GetCustomMtlBrowserIcon(HIMAGELIST& hImageList, int& inactiveIndex, int& activeIndex, int& disabledIndex) {
		if (!imageList) {
			HBITMAP bmp=LoadBitmap(hInstance, MAKEINTRESOURCE(bm_MTL));
			HBITMAP mask=LoadBitmap(hInstance, MAKEINTRESOURCE(bm_MTLMASK));

			imageList=ImageList_Create(11, 11, ILC_COLOR24 | ILC_MASK, 5, 0);
			int index=ImageList_Add(imageList, bmp, mask);
			if (index==-1) return false;
		}

		if (!imageList) return false;

		hImageList=imageList;
		inactiveIndex=0;
		activeIndex=1;
		disabledIndex=2;
		return true;
	}
#endif

#if GET_MAX_RELEASE(VERSION_3DSMAX) >= 13900
	FPInterface* GetInterface(Interface_ID id) {
		if (IMATERIAL_BROWSER_ENTRY_INFO_INTERFACE==id) {
			return static_cast<IMaterialBrowserEntryInfo*>(this);
		}
		return ClassDesc2::GetInterface(id);
	}

	// From IMaterialBrowserEntryInfo
	const MCHAR* GetEntryName() const { return NULL; }
	const MCHAR* GetEntryCategory() const {
#if GET_MAX_RELEASE(VERSION_3DSMAX) >= 14900
		HINSTANCE hInst=GetModuleHandle(_T("sme.gup"));
		if (hInst) {
			static MSTR category(MaxSDK::GetResourceStringAsMSTR(hInst, IDS_3DSMAX_SME_MATERIALS_CATLABEL).Append(_T("\\V-Ray")));
			return category.data();
		}
#endif
		return _T("Materials\\V-Ray");
	}
	Bitmap* GetEntryThumbnail() const { return NULL; }
#endif
};

static SkeletonMaterialClassDesc SkelMtlCD;
ClassDesc* GetSkeletonMtlDesc() {return &SkelMtlCD;}

/*===========================================================================*\
 |	Basic implimentation of a dialog handler
\*===========================================================================*/

/*===========================================================================*\
 |	Paramblock2 Descriptor
\*===========================================================================*/

static int numID=100;
int ctrlID(void) { return numID++; }

#include "..\common\default_values.h"

#define DEFINE_SUBTEX(subtexIndex, texName, defMult, rangeMin, rangeMax, mapID) \
	texInfo[subtexIndex].texID, _T(texName), TYPE_TEXMAP, 0, 0,\
		p_subtexno, subtexIndex,\
		p_ui, mapID, TYPE_TEXMAPBUTTON, ctrlID(),\
	PB_END,\
	texInfo[subtexIndex].texOnID, _T(texName) _T("_on"), TYPE_BOOL, 0, 0,\
		p_default, TRUE,\
		p_ui, mapID, TYPE_SINGLECHEKBOX, ctrlID(),\
	PB_END,\
	texInfo[subtexIndex].texMultID, _T(texName) _T("_multiplier"), TYPE_FLOAT, P_ANIMATABLE, 0,\
		p_default, defMult,\
		p_range, rangeMin, rangeMax,\
		p_ui, mapID, TYPE_SPINNER, EDITTYPE_FLOAT, ctrlID(), ctrlID(), 1.0f,\
	PB_END

#define DEFINE_SUBTEX_SHORTMAP(subtexIndex, texName, defMult, rangeMin, rangeMax, mapID) \
	texInfo[subtexIndex].texShortID, _T(texName) _T("_shortmap"), TYPE_TEXMAP, P_NO_AUTO_LABELS + P_INVISIBLE + P_TRANSIENT + P_OBSOLETE, 0,\
		p_subtexno, subtexIndex,\
		p_ui, mapID, TYPE_TEXMAPBUTTON, texInfo[subtexIndex].shortCtrlID = ctrlID(),\
	PB_END,\
	texInfo[subtexIndex].texID, _T(texName), TYPE_TEXMAP, 0, 0,\
		p_subtexno, subtexIndex,\
		p_ui, map_textures, TYPE_TEXMAPBUTTON, ctrlID(),\
	PB_END,\
	texInfo[subtexIndex].texOnID, _T(texName) _T("_on"), TYPE_BOOL, 0, 0,\
		p_default, TRUE,\
		p_ui, map_textures, TYPE_SINGLECHEKBOX, ctrlID(),\
	PB_END,\
	texInfo[subtexIndex].texMultID, _T(texName) _T("_multiplier"), TYPE_FLOAT, P_ANIMATABLE, 0,\
		p_default, defMult,\
		p_range, rangeMin, rangeMax,\
		p_ui, map_textures, TYPE_SPINNER, EDITTYPE_FLOAT, ctrlID(), ctrlID(), 1.0f,\
	PB_END

static ParamBlockDesc2 smtl_param_blk ( mtl_params, _T("VRayAL parameters"),  0, &SkelMtlCD, P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP, 0, 
	//rollouts
	9,
	map_basic, 0, IDS_PARAMETERS, 0, 0, NULL,
	map_diffuse, 0, IDS_PARAMETERS, 0, 0, NULL,
	map_sss1, 0, IDS_PARAMETERS, 0, 0, NULL,
	map_sss2, 0, IDS_PARAMETERS, 0, 0, NULL,
	map_sss3, 0, IDS_PARAMETERS, 0, 0, NULL,
	map_reflect1, 0, IDS_PARAMETERS, 0, 0, NULL,
	map_reflect2, 0, IDS_PARAMETERS, 0, 0, NULL,
	map_options, 0, IDS_PARAMETERS, 0, 0, NULL,
	map_textures, 0, IDS_PARAMETERS, 0, 0, NULL,

	// params
	mtl_sssDensityScale, _T("sss_density_scale"), TYPE_FLOAT, P_ANIMATABLE, 0,
		p_default, defaultSSSScale,
		p_range, 1e-6f, 1e6f,
		p_ui, map_basic, TYPE_SPINNER, EDITTYPE_FLOAT, ctrlID(), ctrlID(), 0.01f,
	PB_END,
	mtl_opacity, _T("opacity"), TYPE_RGBA, P_ANIMATABLE, 0,
		p_default, Color(defaultOpacity, defaultOpacity, defaultOpacity),
		p_ui, map_basic, TYPE_COLORSWATCH, ctrlID(),
	PB_END,
	DEFINE_SUBTEX_SHORTMAP(SUBTEXNO_OPACITY, "opacity_texture", 100.0f, 0.0f, 100.0f, map_basic),
	DEFINE_SUBTEX(SUBTEXNO_BUMP, "bump_texture", 30.0f, -1000.0f, 1000.0f, map_basic),
	DEFINE_SUBTEX(SUBTEXNO_DISPLACEMENT, "displacement_texture", 100.0f, 0.0f, 100.0f, map_basic),

	mtl_diffuse, _T("diffuse_color"), TYPE_RGBA, P_ANIMATABLE, 0,
		p_default, defaultDiffuse,
		p_ui, map_diffuse, TYPE_COLORSWATCH, ctrlID(),
	PB_END,
	DEFINE_SUBTEX_SHORTMAP(SUBTEXNO_DIFFUSE, "diffuse_color_texture", 100.0f, 0.0f, 100.0f, map_diffuse),

	mtl_diffuseStrength, _T("diffuse_strength"), TYPE_FLOAT, P_ANIMATABLE, 0,
		p_default, defaultDiffuseStrength,
		p_range, 0.0f, 1.0f,
		p_ui, map_diffuse, TYPE_SPINNER, EDITTYPE_FLOAT, ctrlID(), ctrlID(), 0.01f,
	PB_END,
	DEFINE_SUBTEX_SHORTMAP(SUBTEXNO_DIFFUSE_STRENGTH, "diffuse_strength_texture", 100.0f, 0.0f, 100.0f, map_diffuse),
	
	mtl_sssMix, _T("sss_mix"), TYPE_FLOAT, P_ANIMATABLE, 0,
		p_default, defaultSSSMix,
		p_range, 0.0f, 1.0f,
		p_ui, map_diffuse, TYPE_SPINNER, EDITTYPE_FLOAT, ctrlID(), ctrlID(), 0.1f,
	PB_END,
	DEFINE_SUBTEX_SHORTMAP(SUBTEXNO_SSS_MIX, "sss_mix_texture", 100.0f, 0.0f, 100.0f, map_diffuse),

	mtl_sssMode, _T("sss_mode"), TYPE_INT, 0, 0,
		p_range, 0, 1,
		p_default, defaultSSSMode,
		p_ui, map_diffuse, TYPE_INTLISTBOX, ctrlID(), 2, ids_sss_mode_diffusion, ids_sss_mode_directional,
	PB_END,

	DEFINE_SUBTEX(SUBTEXNO_DIFFUSE_BUMP, "diffuse_bump_texture", 30.0f, -1000.0f, 1000.0f, map_diffuse),

	// Reflection 1
	mtl_reflect_color1, _T("reflect1_color"), TYPE_RGBA, P_ANIMATABLE, 0,
		p_default, defaultReflect1,
		p_ui, map_reflect1, TYPE_COLORSWATCH, ctrlID(),
	PB_END,
	DEFINE_SUBTEX_SHORTMAP(SUBTEXNO_REFLECT1_COLOR, "reflect1_color_texture", 100.0f, 0.0f, 100.0f, map_reflect1),

	mtl_reflect_strength1, _T("reflect1_strength"), TYPE_FLOAT, P_ANIMATABLE, 0,
		p_default, defaultReflect1Strength,
		p_range, 0.0f, 1.0f,
		p_ui, map_reflect1, TYPE_SPINNER, EDITTYPE_FLOAT, ctrlID(), ctrlID(), 0.01f,
	PB_END,
	DEFINE_SUBTEX_SHORTMAP(SUBTEXNO_REFLECT1_STRENGTH, "reflect1_strength_texture", 100.0f, 0.0f, 100.0f, map_reflect1),

	mtl_reflect_roughness1, _T("reflect1_roughness"), TYPE_FLOAT, P_ANIMATABLE, 0,
		p_default, defaultReflect1Roughness,
		p_range, 0.0f, 1.0f,
		p_ui, map_reflect1, TYPE_SPINNER, EDITTYPE_FLOAT, ctrlID(), ctrlID(), 0.01f,
	PB_END,
	DEFINE_SUBTEX_SHORTMAP(SUBTEXNO_REFLECT1_ROUGHNESS, "reflect1_roughness_texture", 100.0f, 0.0f, 100.0f, map_reflect1),
	
	mtl_reflect_ior1, _T("reflect1_ior"), TYPE_FLOAT, P_ANIMATABLE, 0,
		p_default, defaultReflect1IOR,
		p_range, 1.0f, 999.0f,
		p_ui, map_reflect1, TYPE_SPINNER, EDITTYPE_FLOAT, ctrlID(), ctrlID(), 0.01f,
	PB_END,
	DEFINE_SUBTEX_SHORTMAP(SUBTEXNO_REFLECT1_IOR, "reflect1_ior_texture", 100.0f, 0.0f, 100.0f, map_reflect1),

	mtl_reflect_distribution1, _T("reflect1_distribution"), TYPE_INT, 0, 0,
		p_range, 0, 1,
		p_default, defaultReflect1Distribution,
		p_ui, map_reflect1, TYPE_INTLISTBOX, ctrlID(), 2, ids_reflect_distribution_beckmann, ids_reflect_distribution_GGX,
	PB_END,

	DEFINE_SUBTEX(SUBTEXNO_REFLECT1_BUMP, "reflect1_bump_texture", 30.0f, -1000.0f, 1000.0f, map_reflect1),

	// Reflection 2
	mtl_reflect_color2, _T("reflect2_color"), TYPE_RGBA, P_ANIMATABLE, 0,
		p_default, defaultReflect2,
		p_ui, map_reflect2, TYPE_COLORSWATCH, ctrlID(),
	PB_END,
	DEFINE_SUBTEX_SHORTMAP(SUBTEXNO_REFLECT2_COLOR, "reflect2_color_texture", 100.0f, 0.0f, 100.0f, map_reflect2),

	mtl_reflect_strength2, _T("reflect2_strength"), TYPE_FLOAT, P_ANIMATABLE, 0,
		p_default, defaultReflect2Strength,
		p_range, 0.0f, 1.0f,
		p_ui, map_reflect2, TYPE_SPINNER, EDITTYPE_FLOAT, ctrlID(), ctrlID(), 0.01f,
	PB_END,
	DEFINE_SUBTEX_SHORTMAP(SUBTEXNO_REFLECT2_STRENGTH, "reflect2_strength_texture", 100.0f, 0.0f, 100.0f, map_reflect2),

	mtl_reflect_roughness2, _T("reflect2_roughness"), TYPE_FLOAT, P_ANIMATABLE, 0,
		p_default, defaultReflect2Roughness,
		p_range, 0.0f, 1.0f,
		p_ui, map_reflect2, TYPE_SPINNER, EDITTYPE_FLOAT, ctrlID(), ctrlID(), 0.01f,
	PB_END,
	DEFINE_SUBTEX_SHORTMAP(SUBTEXNO_REFLECT2_ROUGHNESS, "reflect2_roughness_texture", 100.0f, 0.0f, 100.0f, map_reflect2),
	
	mtl_reflect_ior2, _T("reflect2_ior"), TYPE_FLOAT, P_ANIMATABLE, 0,
		p_default, defaultReflect2IOR,
		p_range, 1.0f, 999.0f,
		p_ui, map_reflect2, TYPE_SPINNER, EDITTYPE_FLOAT, ctrlID(), ctrlID(), 0.01f,
	PB_END,
	DEFINE_SUBTEX_SHORTMAP(SUBTEXNO_REFLECT2_IOR, "reflect2_ior_texture", 100.0f, 0.0f, 100.0f, map_reflect2),

	mtl_reflect_distribution2, _T("reflect2_distribution"), TYPE_INT, 0, 0,
		p_range, 0, 1,
		p_default, defaultReflect2Distribution,
		p_ui, map_reflect2, TYPE_INTLISTBOX, ctrlID(), 2, ids_reflect_distribution_beckmann, ids_reflect_distribution_GGX,
	PB_END,

	DEFINE_SUBTEX(SUBTEXNO_REFLECT2_BUMP, "reflect2_bump_texture", 30.0f, -1000.0f, 1000.0f, map_reflect2),

	// SSS
	mtl_sssColor1, _T("sss1_color"), TYPE_RGBA, P_ANIMATABLE, 0,
		p_default, defaultSSSColor1,
		p_ui, map_sss1, TYPE_COLORSWATCH, ctrlID(),
	PB_END,
	DEFINE_SUBTEX_SHORTMAP(SUBTEXNO_SSS1_COLOR, "sss1_color_texture", 100.0f, 0.0f, 100.0f, map_sss1),

	mtl_sssWeight1, _T("sss1_weight"), TYPE_FLOAT, P_ANIMATABLE, 0,
		p_default, defaultSSSWeight1,
		p_range, 0.0f, 1.0f,
		p_ui, map_sss1, TYPE_SPINNER, EDITTYPE_FLOAT, ctrlID(), ctrlID(), 0.01f,
	PB_END,
	DEFINE_SUBTEX_SHORTMAP(SUBTEXNO_SSS1_WEIGHT, "sss1_weight_texture", 100.0f, 0.0f, 100.0f, map_sss1),

	mtl_sssRadius1, _T("sss1_radius"), TYPE_FLOAT, P_ANIMATABLE, 0,
		p_default, defaultSSSRadius1,
		p_range, 0.0f, 1e6f,
		p_ui, map_sss1, TYPE_SPINNER, EDITTYPE_FLOAT, ctrlID(), ctrlID(), 0.01f,
	PB_END,
	DEFINE_SUBTEX_SHORTMAP(SUBTEXNO_SSS1_RADIUS, "sss1_radius_texture", 100.0f, 0.0f, 100.0f, map_sss1),

	mtl_sssColor2, _T("sss2_color"), TYPE_RGBA, P_ANIMATABLE, 0,
		p_default, defaultSSSColor2,
		p_ui, map_sss2, TYPE_COLORSWATCH, ctrlID(),
	PB_END,
	DEFINE_SUBTEX_SHORTMAP(SUBTEXNO_SSS2_COLOR, "sss2_color_texture", 100.0f, 0.0f, 100.0f, map_sss2),

	mtl_sssWeight2, _T("sss2_weight"), TYPE_FLOAT, P_ANIMATABLE, 0,
		p_default, defaultSSSWeight2,
		p_range, 0.0f, 1.0f,
		p_ui, map_sss2, TYPE_SPINNER, EDITTYPE_FLOAT, ctrlID(), ctrlID(), 0.01f,
	PB_END,
	DEFINE_SUBTEX_SHORTMAP(SUBTEXNO_SSS2_WEIGHT, "sss2_weight_texture", 100.0f, 0.0f, 100.0f, map_sss2),

	mtl_sssRadius2, _T("sss2_radius"), TYPE_FLOAT, P_ANIMATABLE, 0,
		p_default, defaultSSSRadius2,
		p_range, 0.0f, 1e6f,
		p_ui, map_sss2, TYPE_SPINNER, EDITTYPE_FLOAT, ctrlID(), ctrlID(), 0.1f,
	PB_END,
	DEFINE_SUBTEX_SHORTMAP(SUBTEXNO_SSS2_RADIUS, "sss2_radius_texture", 100.0f, 0.0f, 100.0f, map_sss2),

	mtl_sssColor3, _T("sss3_color"), TYPE_RGBA, P_ANIMATABLE, 0,
		p_default, defaultSSSColor3,
		p_ui, map_sss3, TYPE_COLORSWATCH, ctrlID(),
	PB_END,
	DEFINE_SUBTEX_SHORTMAP(SUBTEXNO_SSS3_COLOR, "sss3_color_texture", 100.0f, 0.0f, 100.0f, map_sss3),

	mtl_sssWeight3, _T("sss3_weight"), TYPE_FLOAT, P_ANIMATABLE, 0,
		p_default, defaultSSSWeight3,
		p_range, 0.0f, 1.0f,
		p_ui, map_sss3, TYPE_SPINNER, EDITTYPE_FLOAT, ctrlID(), ctrlID(), 0.01f,
	PB_END,
	DEFINE_SUBTEX_SHORTMAP(SUBTEXNO_SSS3_WEIGHT, "sss3_weight_texture", 100.0f, 0.0f, 100.0f, map_sss3),

	mtl_sssRadius3, _T("sss3_radius"), TYPE_FLOAT, P_ANIMATABLE, 0,
		p_default, defaultSSSRadius3,
		p_range, 0.0f, 1e6f,
		p_ui, map_sss3, TYPE_SPINNER, EDITTYPE_FLOAT, ctrlID(), ctrlID(), 0.01f,
	PB_END,
	DEFINE_SUBTEX_SHORTMAP(SUBTEXNO_SSS3_RADIUS, "sss3_radius_texture", 100.0f, 0.0f, 100.0f, map_sss3),

	mtl_reflect_max_depth, _T("reflect_max_depth"), TYPE_INT, 0, 0,
		p_default, defaultReflectMaxDepth,
		p_range, 0, 100,
		p_ui, map_options, TYPE_SPINNER, EDITTYPE_INT, ctrlID(), ctrlID(), SPIN_AUTOSCALE,
	PB_END,
	mtl_reflect_subdivs, _T("reflect_subdivs"), TYPE_INT, 0, 0,
		p_default, defaultReflectSubdivs,
		p_range, 1, 1000,
		p_ui, map_options, TYPE_SPINNER, EDITTYPE_INT, ctrlID(), ctrlID(), SPIN_AUTOSCALE,
	PB_END,
	mtl_sss_subdivs, _T("sss_subdivs"), TYPE_INT, 0, 0,
		p_default, defaultSSSSubdivs,
		p_range, 1, 1000,
		p_ui, map_options, TYPE_SPINNER, EDITTYPE_INT, ctrlID(), ctrlID(), SPIN_AUTOSCALE,
	PB_END,
PB_END
);

/*===========================================================================*\
 |	UI stuff
\*===========================================================================*/

class SkelMtlDlgProc : public ParamMap2UserDlgProc {
	void shortmapsInit(SkeletonMaterial* mtl, HWND hWnd, IParamMap2 *map) {
		if (!map) return;

		IParamBlock2 *pblock = map->GetParamBlock();
		if (!pblock) return;

		MapID mapid;
		int currRow;
		for (int iTexInfoRow=0; iTexInfoRow<NSUBTEX ; ++iTexInfoRow) {
			if (texInfo[iTexInfoRow].texShortID > -1) {
				ParamDef pDef = pblock->GetParamDef(texInfo[iTexInfoRow].texShortID);
				if (pDef.maps.Count() > 0) {
					mapid = pDef.maps[0];
					if (mapid==map->GetMapID()) {
						currRow = iTexInfoRow;
						mtl->updateTexmapButtons(mapid, currRow);
						ICustButton *btn=GetICustButton(GetDlgItem(hWnd, texInfo[currRow].shortCtrlID));
						if (btn) {
							btn->SetDADMgr(mtl);
							ReleaseICustButton(btn);
						}
					}
				}
			}
		}
	}

public:
	SkeletonMaterial *sm;

	SkelMtlDlgProc(void) { sm = NULL; }
	INT_PTR DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		int id = LOWORD(wParam);
		switch (msg) 
		{
			case WM_INITDIALOG: {
				SkeletonMaterial *mtl=static_cast<SkeletonMaterial*>(map->GetParamBlock()->GetOwner());
				if (mtl) {
					//mtl->greyDlgControls(map);
					shortmapsInit(mtl, hWnd, map);
				}
				break;
			}
			case WM_DESTROY:
				break;
			case WM_COMMAND: {
				break;
			}
		}
		return FALSE;
	}
	void DeleteThis() {}
	void SetThing(ReferenceTarget *m) { sm = (SkeletonMaterial*)m; }
};

static SkelMtlDlgProc dlgProc;
static Pb2TemplateGenerator templateGenerator;

struct ParamMapInfo {
	const TCHAR *title;
	int flags;
};

ParamMapInfo pmapInfo[NUM_PMAPS]={
	{ _T("General parameters"), 0 },
	{ _T("Diffuse"), 0 },
	{ _T("SSS 1"), 0 },
	{ _T("SSS 2"), 0 },
	{ _T("SSS 3"), 0 },
	{ _T("Reflection 1"), 0, },
	{ _T("Reflection 2"), 0, },
	{ _T("Options and sampling"), APPENDROLL_CLOSED },
	{ _T("Textures"), APPENDROLL_CLOSED }
};

class SkelMtlParamDlg: public ParamDlg {
public:
	SkeletonMaterial *mtl;
	IMtlParams *ip;
	IParamMap2 *pmaps[NUM_PMAPS];
	
	SkelMtlParamDlg(SkeletonMaterial *m, HWND hWnd, IMtlParams *i) {
		mtl=m;
		ip=i;
		
		DLGTEMPLATE* tmp=NULL;
		
		for (int i=0; i<NUM_PMAPS; i++) {
			tmp=templateGenerator.GenerateTemplate(i, mtl->pblock, pmapInfo[i].title, 217);
			pmaps[i]=CreateMParamMap2(i, mtl->pblock, ip, hInstance, hWnd, NULL, NULL, tmp, pmapInfo[i].title, pmapInfo[i].flags, &dlgProc);
			templateGenerator.ReleaseDlgTemplate(tmp);
		}
	}

	void DeleteThis(void) VRAY_OVERRIDE {
		for (int i=0; i<NUM_PMAPS; i++) {
			if (pmaps[i]) DestroyMParamMap2(pmaps[i]);
			pmaps[i]=NULL;
		}
		delete this;
	}
	
	Class_ID ClassID(void) VRAY_OVERRIDE { return MTL_CLASSID; }

	void SetThing(ReferenceTarget *m) VRAY_OVERRIDE {
		mtl=(SkeletonMaterial*) m;
		for (int i=0; i<NUM_PMAPS; i++) {
			if (pmaps[i])
				pmaps[i]->SetParamBlock(mtl->pblock);
		}
	}
	
	ReferenceTarget* GetThing(void) VRAY_OVERRIDE { return mtl; }
	void SetTime(TimeValue t) VRAY_OVERRIDE {}
	void ReloadDialog(void) VRAY_OVERRIDE {}
	void ActivateDlg(BOOL onOff) VRAY_OVERRIDE {}
};

/*===========================================================================*\
 |	Constructor and Reset systems
 |  Ask the ClassDesc2 to make the AUTO_CONSTRUCT paramblocks and wire them in
\*===========================================================================*/

void SkeletonMaterial::Reset() {
	ivalid.SetEmpty();
	SkelMtlCD.Reset(this);
}

SkeletonMaterial::SkeletonMaterial(BOOL loading) {
	static int pblockDesc_inited=false;
	if (!pblockDesc_inited) {
		initPBlockDesc(smtl_param_blk);
		pblockDesc_inited=true;
	}

	for (int i=0; i<NSUBTEX; i++) subtex[i]=NULL;

	version=CURRENT_VERSION;

	pblock=NULL;
	ivalid.SetEmpty();
	SkelMtlCD.MakeAutoParamBlocks(this);	// make and intialize paramblock2
}

ParamDlg* SkeletonMaterial::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp) {
	return new SkelMtlParamDlg(this, hwMtlEdit, imp);
}

BOOL SkeletonMaterial::SetDlgThing(ParamDlg* dlg) {
	return FALSE;
}

Interval SkeletonMaterial::Validity(TimeValue t) {
	Interval temp=FOREVER;
	Update(t, temp);
	return ivalid;
}

int SkeletonMaterial::NumSubTexmaps() { return NSUBTEX; }

Texmap* SkeletonMaterial::GetSubTexmap(int i) {
	if (i<0 || i>=NSUBTEX) return NULL;
	return pblock->GetTexmap(texInfo[i].texID);
}

void SkeletonMaterial::SetSubTexmap(int i, Texmap *m) {
	if (i<0 || i>=NSUBTEX) return;
	pblock->SetValue(texInfo[i].texID, 0, m);
}

TSTR SkeletonMaterial::GetSubTexmapSlotName(int i) {
	if (i<0 || i>=NSUBTEX) return _T("???");
	return texInfo[i].texName;
}

/*===========================================================================*\
 |	Subanim & References support
\*===========================================================================*/

RefTargetHandle SkeletonMaterial::GetReference(int i) {
	if (i==0) return pblock;
	return NULL;
}

void SkeletonMaterial::SetReference(int i, RefTargetHandle rtarg) {
	if (i==0) pblock=(IParamBlock2*) rtarg;
}

TSTR SkeletonMaterial::SubAnimName(int i) {
	if (i==0) return _T("Parameters");
	return _T("");
}

Animatable* SkeletonMaterial::SubAnim(int i) {
	if (i==0) return pblock;
	return NULL;
}

RefResult SkeletonMaterial::NotifyRefChanged(NOTIFY_REF_CHANGED_ARGS) {
	switch (message) {
		case REFMSG_CHANGE:
			ivalid.SetEmpty();
			if (hTarget==pblock) {
				ParamID paramID=pblock->LastNotifyParamID();

				// If changed parameter is map or map related parameter (as _tex, _tex_on)
				// we have to check if that param is presented in texInfo structure.
				// If it is true then by received MapID and TexInfoRow 
				// corresponding shortmap button text is actualized.
				MapID inMapID=-1;
				int inTexIR=-1;
				findMapIDAndTexInfoRowByChangedParam(paramID, inMapID, inTexIR);
				if (inMapID>=0 && inTexIR>=0)
					updateTexmapButtons(inMapID, inTexIR);
				smtl_param_blk.InvalidateUI(paramID);
			}
			break;
	}
	return(REF_SUCCEED);
}

/*===========================================================================*\
 |	Standard IO
\*===========================================================================*/

#define MTL_HDR_CHUNK 0x4000
#define MTL_VERSION_CHUNK 0x4001

IOResult SkeletonMaterial::Save(ISave *isave) { 
	IOResult res;
	isave->BeginChunk(MTL_HDR_CHUNK);
	res = MtlBase::Save(isave);
	if (res!=IO_OK) return res;
	isave->EndChunk();
	
	ULONG nwr=0;

	isave->BeginChunk(MTL_VERSION_CHUNK);
	res=isave->Write(&version, sizeof(version), &nwr);
	if (res!=IO_OK) return res;
	isave->EndChunk();

	return IO_OK;
}	

IOResult SkeletonMaterial::Load(ILoad *iload) { 
	// Set the version to be the oldest possible version
	version=0;
	iload->RegisterPostLoadCallback(this);

	IOResult res;
	int id;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(id = iload->CurChunkID())  {
			case MTL_HDR_CHUNK:
				res = MtlBase::Load(iload);
				break;
			case MTL_VERSION_CHUNK: {
				ULONG nr;
				res=iload->Read(&version, sizeof(version), &nr);
				break;
			}
		}
		iload->CloseChunk();
		if (res!=IO_OK)
			return res;
	}
	return IO_OK;
}

void SkeletonMaterial::proc(ILoad *iload) {
	if (version<1) {
		// In the initial version, "directional" was the default SSS mode, so change to that when
		// loading old materials.
		pblock->SetValue(mtl_sssMode, 0, 1);

		// In the old version, the reflection distribution was always GGX, so change to that.
		pblock->SetValue(mtl_reflect_distribution1, 0, 1);
		pblock->SetValue(mtl_reflect_distribution2, 0, 1);
	}
	version=CURRENT_VERSION;
}

/*===========================================================================*\
 |	FROM DADMgr
\*===========================================================================*/

SClass_ID SkeletonMaterial::GetDragType(HWND hwnd, POINT p) {
	return TEXMAP_CLASS_ID;
}

BOOL SkeletonMaterial::IsNew(HWND hwnd, POINT p, SClass_ID type) {
	return FALSE;
}

ReferenceTarget* SkeletonMaterial::GetInstance(HWND hwnd, POINT p, SClass_ID type) {
	int iRow = getRowBySelectedShortCtrl(hwnd);
	if (iRow >= 0) 
		return pblock->GetTexmap(texInfo[iRow].texID);
	
	return NULL;
}

BOOL SkeletonMaterial::OkToDrop(ReferenceTarget *dropThis, HWND hfrom, HWND hto, POINT p, SClass_ID type, BOOL isNew) {
	return (type==TEXMAP_CLASS_ID);
}

HCURSOR SkeletonMaterial::DropCursor(ReferenceTarget *dropThis, HWND hfrom, HWND hto, POINT p, SClass_ID type, BOOL isNew) {
	return NULL;
}

int SkeletonMaterial::SlotOwner() {
	return OWNER_MTL_TEX;
}

#if GET_MAX_RELEASE(VERSION_3DSMAX) < 11900
void SkeletonMaterial::Drop(ReferenceTarget *dropThis, HWND hwnd, POINT p, SClass_ID type) {
#else
void SkeletonMaterial::Drop(ReferenceTarget *dropThis, HWND hwnd, POINT p, SClass_ID type, DADMgr* srcMgr, BOOL bSrcClone) {
#endif
	drop(dropThis, hwnd, p, type, pblock);
}

void SkeletonMaterial::drop(ReferenceTarget *dropThis, HWND hwnd, POINT p, SClass_ID type, IParamBlock2 *pblock) {
	int iRow = getRowBySelectedShortCtrl(hwnd);
	if (iRow >= 0) {
		IParamMap2 *mapTextures = pblock->GetMap(map_textures);
		theHold.Begin();
		pblock->SetValue(texInfo[iRow].texID, 0, (Texmap*) dropThis);
		mapTextures->Invalidate(texInfo[iRow].texID);
		theHold.Accept(_T("VRayALMtl set texture"));
	}
}

void SkeletonMaterial::findMapIDAndTexInfoRowByChangedParam(ParamID inChangedParam, MapID &mapid, int &texInfoRow) {
	for (int iTexInfoRow=0; iTexInfoRow<NSUBTEX ; ++iTexInfoRow) {
		if ((texInfo[iTexInfoRow].texID == inChangedParam || texInfo[iTexInfoRow].texOnID == inChangedParam) &&
			texInfo[iTexInfoRow].texShortID > -1) {
			ParamDef pDef = pblock->GetParamDef(texInfo[iTexInfoRow].texShortID);
			if (pDef.maps.Count() > 0)
				mapid = pDef.maps[0];
			texInfoRow = iTexInfoRow;
			break;
		}
	}
}

int SkeletonMaterial::getRowBySelectedShortCtrl(HWND hwnd) {
	HWND mapWnd;
	MapID mapid;
	IParamMap2 *map;
	
	for (int iTexInfoRow=0; iTexInfoRow<NSUBTEX ; ++iTexInfoRow) {
		if (texInfo[iTexInfoRow].texShortID > -1) {
			ParamDef pDef = pblock->GetParamDef(texInfo[iTexInfoRow].texShortID);
			if (pDef.maps.Count() > 0) {
				mapid = pDef.maps[0];
				map = pblock->GetMap(mapid);
				mapWnd=map->GetHWnd();
				if (texInfo[iTexInfoRow].shortCtrlID && hwnd==GetDlgItem(mapWnd, texInfo[iTexInfoRow].shortCtrlID))
					return iTexInfoRow;
			}
		}
	}
	
	return -1;
}

int SkeletonMaterial::getMapState(int subtexno) {
	Texmap* map = pblock->GetTexmap(texInfo[subtexno].texID);
	if(!map) return 0;

	int enabled;
	Interval ivalid=FOREVER;
	TimeValue t=GetCOREInterface()->GetTime();
	pblock->GetValue(texInfo[subtexno].texOnID, t, enabled, ivalid);
	if (!enabled) return 1;

	return 2;
}

void SkeletonMaterial::updateTexmapButtons(MapID mapid, int subtexno) {
	IParamMap2 *map;
	if (!pblock || !(map=pblock->GetMap(mapid))) return;
	
	HWND hWnd=map->GetHWnd();

	ICustButton *button=GetICustButton(GetDlgItem(hWnd, texInfo[subtexno].shortCtrlID));
	if (button) {
		button->SetText(externShortcutTexts[getMapState(subtexno)]);
		ReleaseICustButton(button);
	}
}

/*===========================================================================*\
 |	Updating and cloning
\*===========================================================================*/

RefTargetHandle SkeletonMaterial::Clone(RemapDir &remap) {
	SkeletonMaterial *mnew = new SkeletonMaterial(FALSE);
	*((MtlBase*)mnew) = *((MtlBase*)this); 
	BaseClone(this, mnew, remap);
	mnew->ReplaceReference(0, remap.CloneRef(pblock));
	mnew->ivalid.SetEmpty();	
	return (RefTargetHandle) mnew;
}

void SkeletonMaterial::NotifyChanged() {
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
}

void SkeletonMaterial::Update(TimeValue t, Interval& valid) {
	if (!ivalid.InInterval(t)) {
		ivalid.SetInfinite();

		// Sub-textures
		for (int i=0; i<NSUBTEX; i++) {
			subtex[i]=NULL;

			pblock->GetValue(texInfo[i].texOnID, t, subtexOn[i], ivalid);
			pblock->GetValue(texInfo[i].texMultID, t, subtexMult[i], ivalid);
			if (fabsf(subtexMult[i])<1e-6f) subtexOn[i]=false;
			subtexMult[i]*=0.01f;

			if (subtexOn[i]) {
				subtex[i]=pblock->GetTexmap(texInfo[i].texID);
				if (subtex[i]) subtex[i]->Update(t, ivalid);
			}
		}

		pblock->GetValue(mtl_reflect_roughness1, t, reflectRoughness1, ivalid);
		pblock->GetValue(mtl_reflect_color1, t, reflectColor1, ivalid);
		pblock->GetValue(mtl_reflect_strength1, t, reflectStrength1, ivalid);
		pblock->GetValue(mtl_reflect_ior1, t, reflectIOR1, ivalid);
		pblock->GetValue(mtl_reflect_distribution1, t, reflectDistribution1, ivalid);

		pblock->GetValue(mtl_reflect_roughness2, t, reflectRoughness2, ivalid);
		pblock->GetValue(mtl_reflect_color2, t, reflectColor2, ivalid);
		pblock->GetValue(mtl_reflect_strength2, t, reflectStrength2, ivalid);
		pblock->GetValue(mtl_reflect_ior2, t, reflectIOR2, ivalid);
		pblock->GetValue(mtl_reflect_distribution2, t, reflectDistribution2, ivalid);

		pblock->GetValue(mtl_diffuse, t, diffuse, ivalid);
		pblock->GetValue(mtl_opacity, t, opacity, ivalid);

		pblock->GetValue(mtl_sssMix, t, sssMix, ivalid);
		pblock->GetValue(mtl_sssMode, t, sssMode, ivalid);

		pblock->GetValue(mtl_sssWeight1, t, sssWeight1, ivalid);
		pblock->GetValue(mtl_sssRadius1, t, sssRadius1, ivalid);
		pblock->GetValue(mtl_sssColor1, t, sssColor1, ivalid);

		pblock->GetValue(mtl_sssWeight2, t, sssWeight2, ivalid);
		pblock->GetValue(mtl_sssRadius2, t, sssRadius2, ivalid);
		pblock->GetValue(mtl_sssColor2, t, sssColor2, ivalid);

		pblock->GetValue(mtl_sssWeight3, t, sssWeight3, ivalid);
		pblock->GetValue(mtl_sssRadius3, t, sssRadius3, ivalid);
		pblock->GetValue(mtl_sssColor3, t, sssColor3, ivalid);

		pblock->GetValue(mtl_sssDensityScale, t, sssDensityScale, ivalid);
		pblock->GetValue(mtl_diffuseStrength, t, diffuseStrength, ivalid);

		pblock->GetValue(mtl_reflect_max_depth, t, reflectMaxDepth, ivalid);
		pblock->GetValue(mtl_reflect_subdivs, t, reflectSubdivs, ivalid);
		pblock->GetValue(mtl_sss_subdivs, t, sssSubdivs, ivalid);
	}

	valid &= ivalid;
}

Interval SkeletonMaterial::DisplacementValidity(TimeValue t) {
	Interval valid=FOREVER;

	int texOn=true;
	pblock->GetValue(texInfo[SUBTEXNO_DISPLACEMENT].texOnID, t, texOn, valid);
	if (!texOn) return valid;

	Texmap *texmap=NULL;
	float texMult=100.0f;

	pblock->GetValue(texInfo[SUBTEXNO_DISPLACEMENT].texID, t, texmap, valid);
	pblock->GetValue(texInfo[SUBTEXNO_DISPLACEMENT].texMultID, t, texMult, valid);

	if (texmap) {
		valid&=texmap->Validity(t);
	}

	return valid;
}

ULONG SkeletonMaterial::LocalRequirements(int subMtlNum) {
	ULONG res=0;
	if (subtex[SUBTEXNO_DISPLACEMENT] && subtexOn[SUBTEXNO_DISPLACEMENT] && subtexMult[SUBTEXNO_DISPLACEMENT]!=0.0f) res|=MTLREQ_DISPLACEMAP;

	int isOpaque=true;

	if (fabsf(Intens(opacity)-1.0f)>1e-6f || (subtexOn[SUBTEXNO_OPACITY] && subtex[SUBTEXNO_OPACITY]!=NULL && fabsf(subtexMult[SUBTEXNO_OPACITY])>1e-6f)) {
		isOpaque=false;
	}

	if (isOpaque) res|=VRAY_MTLREQ_OPAQUE_SHADOWS;
	else res|=MTLREQ_TRANSP;

	return res;
}

void SkeletonMaterial::renderBegin(TimeValue t, VR::VRayRenderer *vray) {
	const VR::VRaySequenceData &sdata=vray->getSequenceData();
	bsdfPool.init(sdata.maxRenderThreads);
}

void SkeletonMaterial::renderEnd(VR::VRayRenderer *vray) {
	bsdfPool.freeMem();
	renderChannels.freeMem();
}
