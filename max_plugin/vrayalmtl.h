#ifndef __MTLSKEL__H
#define __MTLSKEL__H

#include "max.h"
#include <bmmlib.h>
#include "iparamm2.h"
#include "resource.h"

#if GET_MAX_RELEASE(VERSION_3DSMAX) >= 6000
#include "IMtlRender_Compatibility.h"
#endif
#if GET_MAX_RELEASE(VERSION_3DSMAX) >= 13900
#include <IMaterialBrowserEntryInfo.h>
#endif

#include "vraybase.h"
#include "vrayplugins.h"
#include "brdfs.h"
#include "vraygeom.h"
#include "brdfpool.h"
#include "../common/albrdf.h"

// IMPORTANT:
// The ClassID must be changed whenever a new project
// is created using this skeleton
#define	MTL_CLASSID	Class_ID(0x7d080943, 0x54fe7f7d)
#define STR_CLASSNAME _T("VRayALMtl")
#define STR_LIBDESC _T("VRayALMtl material")
#define STR_DLGTITLE _T("VRayALMtl Parameters")

extern ClassDesc* GetSkeletonMtlDesc();

// Paramblock2 name
enum { mtl_params, }; 

// Parameter map IDs
enum { map_basic, map_diffuse, map_sss1, map_sss2, map_sss3, map_reflect1, map_reflect2, map_textures, NUM_PMAPS };

// Paramblock2 parameter list
enum {
	mtl_reflect_roughness1,
	mtl_reflect_color1,

	mtl_diffuse,
	mtl_opacity,

	mtl_sssMix,

	mtl_sssWeight1,
	mtl_sssColor1,
	mtl_sssRadius1,

	mtl_sssWeight2,
	mtl_sssColor2,
	mtl_sssRadius2,

	mtl_sssWeight3,
	mtl_sssColor3,
	mtl_sssRadius3,

	mtl_diffuseStrength,
	mtl_sssDensityScale,

	mtl_reflect_strength1,
	mtl_reflect_ior1,

	mtl_reflect_roughness2,
	mtl_reflect_color2,
	mtl_reflect_strength2,
	mtl_reflect_ior2,

	mtl_opacity_tex, mtl_opacity_tex_on, mtl_opacity_tex_mult, mtl_opacity_tex_shortmap,
	mtl_bump_tex, mtl_bump_tex_on, mtl_bump_tex_mult,
	mtl_displacement_tex, mtl_displacement_tex_on, mtl_displacement_tex_mult,

	mtl_diffuse_tex, mtl_diffuse_tex_on, mtl_diffuse_tex_mult, mtl_diffuse_tex_shortmap,
	mtl_diffuse_strength_tex, mtl_diffuse_strength_tex_on, mtl_diffuse_strength_tex_mult, mtl_diffuse_strength_tex_shortmap,

	mtl_reflect_color1_tex, mtl_reflect_color1_tex_on, mtl_reflect_color1_tex_mult, mtl_reflect_color1_tex_shortmap,
	mtl_reflect_roughness1_tex, mtl_reflect_roughness1_tex_on, mtl_reflect_roughness1_tex_mult, mtl_reflect_roughness1_tex_shortmap,
	mtl_reflect_strength1_tex, mtl_reflect_strength1_tex_on, mtl_reflect_strength1_tex_mult, mtl_reflect_strength1_tex_shortmap,
	mtl_reflect_ior1_tex, mtl_reflect_ior1_tex_on, mtl_reflect_ior1_tex_mult, mtl_reflect_ior1_tex_shortmap,

	mtl_reflect_color2_tex, mtl_reflect_color2_tex_on, mtl_reflect_color2_tex_mult, mtl_reflect_color2_tex_shortmap,
	mtl_reflect_roughness2_tex, mtl_reflect_roughness2_tex_on, mtl_reflect_roughness2_tex_mult, mtl_reflect_roughness2_tex_shortmap,
	mtl_reflect_strength2_tex, mtl_reflect_strength2_tex_on, mtl_reflect_strength2_tex_mult, mtl_reflect_strength2_tex_shortmap,
	mtl_reflect_ior2_tex, mtl_reflect_ior2_tex_on, mtl_reflect_ior2_tex_mult, mtl_reflect_ior2_tex_shortmap,

	mtl_sss_mix_tex, mtl_sss_mix_tex_on, mtl_sss_mix_tex_mult, mtl_sss_mix_tex_shortmap,

	mtl_sss_weight1_tex, mtl_sss_weight1_tex_on, mtl_sss_weight1_tex_mult, mtl_sss_weight1_tex_shortmap,
	mtl_sss_color1_tex, mtl_sss_color1_tex_on, mtl_sss_color1_tex_mult, mtl_sss_color1_tex_shortmap,
	mtl_sss_radius1_tex, mtl_sss_radius1_tex_on, mtl_sss_radius1_tex_mult, mtl_sss_radius1_tex_shortmap,

	mtl_sss_weight2_tex, mtl_sss_weight2_tex_on, mtl_sss_weight2_tex_mult, mtl_sss_weight2_tex_shortmap,
	mtl_sss_color2_tex, mtl_sss_color2_tex_on, mtl_sss_color2_tex_mult, mtl_sss_color2_tex_shortmap,
	mtl_sss_radius2_tex, mtl_sss_radius2_tex_on, mtl_sss_radius2_tex_mult, mtl_sss_radius2_tex_shortmap,

	mtl_sss_weight3_tex, mtl_sss_weight3_tex_on, mtl_sss_weight3_tex_mult, mtl_sss_weight3_tex_shortmap,
	mtl_sss_color3_tex, mtl_sss_color3_tex_on, mtl_sss_color3_tex_mult, mtl_sss_color3_tex_shortmap,
	mtl_sss_radius3_tex, mtl_sss_radius3_tex_on, mtl_sss_radius3_tex_mult, mtl_sss_radius3_tex_shortmap,

	mtl_sssMode,
	mtl_reflect_distribution1,
	mtl_reflect_distribution2,

	mtl_diffuse_bump_tex, mtl_diffuse_bump_tex_on, mtl_diffuse_bump_tex_mult,
	mtl_reflect_bump1_tex, mtl_reflect_bump1_tex_on, mtl_reflect_bump1_tex_mult,
	mtl_reflect_bump2_tex, mtl_reflect_bump2_tex_on, mtl_reflect_bump2_tex_mult,
};

enum {
	SUBTEXNO_OPACITY,
	SUBTEXNO_BUMP,
	SUBTEXNO_DISPLACEMENT,

	SUBTEXNO_DIFFUSE,
	SUBTEXNO_DIFFUSE_STRENGTH,

	SUBTEXNO_REFLECT1_COLOR,
	SUBTEXNO_REFLECT1_STRENGTH,
	SUBTEXNO_REFLECT1_ROUGHNESS,
	SUBTEXNO_REFLECT1_IOR,

	SUBTEXNO_REFLECT2_COLOR,
	SUBTEXNO_REFLECT2_STRENGTH,
	SUBTEXNO_REFLECT2_ROUGHNESS,
	SUBTEXNO_REFLECT2_IOR,

	SUBTEXNO_SSS_MIX,
	SUBTEXNO_SSS1_COLOR,
	SUBTEXNO_SSS1_WEIGHT,
	SUBTEXNO_SSS1_RADIUS,
	SUBTEXNO_SSS2_COLOR,
	SUBTEXNO_SSS2_WEIGHT,
	SUBTEXNO_SSS2_RADIUS,
	SUBTEXNO_SSS3_COLOR,
	SUBTEXNO_SSS3_WEIGHT,
	SUBTEXNO_SSS3_RADIUS,

	SUBTEXNO_DIFFUSE_BUMP,
	SUBTEXNO_REFLECT1_BUMP,
	SUBTEXNO_REFLECT2_BUMP,

	NSUBTEX
};

/*===========================================================================*\
 |	The actual BRDF
\*===========================================================================*/

class MyALBSDF: public VR::MyBaseBSDF {
public:
	VR::Vector getGlossyReflectionDir(float uc, float vc, const VR::Vector &viewDir, float &rayProbability);
	VR::real getGlossyProbability(const VR::Vector &direction, const VR::Vector &viewDir);
	float remapGlossiness(float nk);
};

struct ALMtlTexInfo;

/*===========================================================================*\
 |	SkeletonMaterial class defn
\*===========================================================================*/

class SkeletonMaterial : public Mtl, public VR::VRenderMtl, public DADMgr, public PostLoadCallback {
	VR::BRDFPool<MyALBSDF> bsdfPool;
	VR::LayeredBSDFRenderChannels renderChannels;

	VR::Color getBlend(ShadeContext &sc, int i);

	// Depending on changed tex parameters  _tex, _tex_on, 
	// the corresponding mapId and subtexno of ALMtlTexInfo are returned.
	void findMapIDAndTexInfoRowByChangedParam(ParamID inChangedParam, MapID &mapid, int &texInfoRow);

	// By selected shortmap button, the corresponding subtexno of TexInfo structure is defined.
	// It is needed for access to the other referenced tex parameters.
	int getRowBySelectedShortCtrl(HWND hwnd);

	// Depending on that the texture is attached or attached and not included or not attached,
	// the shortcut button text is selected based on index the method returns.
	int getMapState(int subtexno);

	// Evaluate a color texture and combine with a color.
	VR::Color combineTex(const VR::VRayContext &rc, const VR::Color &origColor, int texIndex) const;

	// Evaluate a float texture and combine with a float.
	float combineTex(const VR::VRayContext &rc, float origValue, int texIndex) const;

	// Return a bumped normal, taking into account the bump map.
	VR::Vector getBumpNormal(const VR::VRayContext &rc, int texIndex, const VR::Vector &baseNormal) const;

	// The version of the material; used when loading older versions to convert some parameters.
	int version;
public:
	// various variables
	Interval ivalid;

	// Cached parameters
	float reflectRoughness1;
	Color reflectColor1;
	float reflectStrength1;
	float reflectIOR1;
	int reflectDistribution1;

	float reflectRoughness2;
	Color reflectColor2;
	float reflectStrength2;
	float reflectIOR2;
	int reflectDistribution2;

	Color opacity;
	Color diffuse;
	float diffuseStrength;

	int sssMode;
	float sssMix;
	float sssDensityScale;
	float sssRadius1, sssRadius2, sssRadius3;
	float sssWeight1, sssWeight2, sssWeight3;
	Color sssColor1, sssColor2, sssColor3;

	Texmap *subtex[NSUBTEX];
	float subtexMult[NSUBTEX];
	int subtexOn[NSUBTEX];

	// Parameter and UI management
	IParamBlock2 *pblock; 	
	ParamDlg* CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp);
	void Update(TimeValue t, Interval& valid);
	Interval Validity(TimeValue t);
	void Reset();

	SkeletonMaterial(BOOL loading);
	Class_ID ClassID() { return MTL_CLASSID; }
	SClass_ID SuperClassID() { return MATERIAL_CLASS_ID; }
	void GetClassName(TSTR& s) { s=STR_CLASSNAME; }
	void DeleteThis() { delete this; }
	
	void NotifyChanged();

	// From MtlBase and Mtl
	void SetAmbient(Color c, TimeValue t);		
	void SetDiffuse(Color c, TimeValue t);		
	void SetSpecular(Color c, TimeValue t);
	void SetShininess(float v, TimeValue t);
	Color GetAmbient(int mtlNum=0, BOOL backFace=FALSE);
	Color GetDiffuse(int mtlNum=0, BOOL backFace=FALSE);
	Color GetSpecular(int mtlNum=0, BOOL backFace=FALSE);
	float GetXParency(int mtlNum=0, BOOL backFace=FALSE);
	float GetShininess(int mtlNum=0, BOOL backFace=FALSE);		
	float GetShinStr(int mtlNum=0, BOOL backFace=FALSE);
	float WireSize(int mtlNum=0, BOOL backFace=FALSE);
			
	// Shade and displacement calculation
	void Shade(ShadeContext& sc);
	float EvalDisplacement(ShadeContext& sc); 
	Interval DisplacementValidity(TimeValue t); 
	ULONG LocalRequirements(int subMtlNum);

	// SubMaterial access methods
	int NumSubMtls() { return 0; }
	Mtl* GetSubMtl(int i) { return NULL; }
	void SetSubMtl(int i, Mtl *m) {}
	TSTR GetSubMtlSlotName(int i) { return _T(""); }
	TSTR GetSubMtlTVName(int i) { return _T(""); }

	// SubTexmap access methods
	int NumSubTexmaps();
	Texmap* GetSubTexmap(int i);
	void SetSubTexmap(int i, Texmap *m);
	TSTR GetSubTexmapSlotName(int i);

	// Number of subanims
	int NumSubs() { return 1; } 
	Animatable* SubAnim(int i);
	TSTR SubAnimName(int i);
	int SubNumToRefNum(int subNum) { return subNum; }

	// Number of references
 	int NumRefs() { return 1; }
	RefTargetHandle GetReference(int i);
	void SetReference(int i, RefTargetHandle rtarg);
	RefResult NotifyRefChanged(NOTIFY_REF_CHANGED_ARGS);

	RefTargetHandle Clone(RemapDir& remap);

	IOResult Save(ISave *isave); 
	IOResult Load(ILoad *iload); 

	// Direct Paramblock2 access
	int	NumParamBlocks() { return 1; }
	IParamBlock2* GetParamBlock(int i) { return pblock; }
	IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock->ID() == id) ? pblock : NULL; } 
	BOOL SetDlgThing(ParamDlg* dlg);

	void* GetInterface(ULONG id) {
		if (id==I_VRAYMTL) return static_cast<VR::VRenderMtl*>(this);
		return Mtl::GetInterface(id);
	}

	// From DADMgr
	SClass_ID GetDragType(HWND hwnd, POINT p);
	BOOL IsNew(HWND hwnd, POINT p, SClass_ID type);
	ReferenceTarget *GetInstance(HWND hwnd, POINT p, SClass_ID type);
	BOOL OkToDrop(ReferenceTarget *dropThis, HWND hfrom, HWND hto, POINT p, SClass_ID type, BOOL isNew);
	HCURSOR DropCursor(ReferenceTarget *dropThis, HWND hfrom, HWND hto, POINT p, SClass_ID type, BOOL isNew);
	int SlotOwner();
	BOOL AutoTooltip(){ return TRUE; }
#if GET_MAX_RELEASE(VERSION_3DSMAX) < 11900
	void Drop(ReferenceTarget *dropThis, HWND hwnd, POINT p, SClass_ID type);
#else
	void Drop(ReferenceTarget *dropThis, HWND hwnd, POINT p, SClass_ID type, DADMgr* srcMgr = NULL, BOOL bSrcClone = FALSE);
#endif

	void drop(ReferenceTarget *dropThis, HWND hwnd, POINT p, SClass_ID type, IParamBlock2 *pblock);

	void updateTexmapButtons(MapID mapid, int subtexno);

	// From VRenderMtl
	void renderBegin(TimeValue t, VR::VRayRenderer *vray) VRAY_OVERRIDE;
	void renderEnd(VR::VRayRenderer *vray) VRAY_OVERRIDE;

	VR::BSDFSampler* newBSDF(const VR::VRayContext &rc, VR::VRenderMtlFlags flags) VRAY_OVERRIDE;
	void deleteBSDF(const VR::VRayContext &rc, VR::BSDFSampler *bsdf) VRAY_OVERRIDE;
	
	void addRenderChannel(int index) VRAY_OVERRIDE;
	VR::VRayVolume* getVolume(const VR::VRayContext &rc) VRAY_OVERRIDE;

	// From PostLoadCallback
	void proc(ILoad *iload) VRAY_OVERRIDE;
};

/*===========================================================================*\
 |	Dialog Processor
\*===========================================================================*/

#endif