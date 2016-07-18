#include "max.h"

#include "vrayinterface.h"
#include "vrender_unicode.h"

#include "vrayalmtl.h"
#include "pb2template_generator.h"

// no param block script access for VRay free
#ifdef _FREE_
#define _FT(X) _T("")
#define IS_PUBLIC 0
#else
#define _FT(X) _T(X)
#define IS_PUBLIC 1
#endif // _FREE_

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
	int IsPublic() { return IS_PUBLIC; }
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

static ParamBlockDesc2 smtl_param_blk ( mtl_params, _T("VRayAL parameters"),  0, &SkelMtlCD, P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP, 0, 
	//rollouts
	8,
	map_basic, 0, IDS_PARAMETERS, 0, 0, NULL,
	map_diffuse, 0, IDS_PARAMETERS, 0, 0, NULL,
	map_sss1, 0, IDS_PARAMETERS, 0, 0, NULL,
	map_sss2, 0, IDS_PARAMETERS, 0, 0, NULL,
	map_sss3, 0, IDS_PARAMETERS, 0, 0, NULL,
	map_reflect1, 0, IDS_PARAMETERS, 0, 0, NULL,
	map_reflect2, 0, IDS_PARAMETERS, 0, 0, NULL,
	map_textures, 0, IDS_PARAMETERS, 0, 0, NULL,

	// params
	mtl_sssDensityScale, _FT("sss_density_scale"), TYPE_FLOAT, P_ANIMATABLE, 0,
		p_default, 1.0f,
		p_range, 1e-6f, 1e6f,
		p_ui, map_basic, TYPE_SPINNER, EDITTYPE_FLOAT, ctrlID(), ctrlID(), 0.01f,
	PB_END,
	mtl_opacity, _FT("opacity"), TYPE_RGBA, P_ANIMATABLE, 0,
		p_default, Color(1.0f, 1.0f, 1.0f),
		p_ui, map_basic, TYPE_COLORSWATCH, ctrlID(),
	PB_END,

	mtl_diffuse, _FT("diffuse_color"), TYPE_RGBA, P_ANIMATABLE, 0,
		p_default, Color(0.5f, 0.5f, 0.5f),
		p_ui, map_diffuse, TYPE_COLORSWATCH, ctrlID(),
	PB_END,
	mtl_diffuseStrength, _FT("diffuse_strength"), TYPE_FLOAT, P_ANIMATABLE, 0,
		p_default, 1.0f,
		p_range, 0.0f, 1.0f,
		p_ui, map_diffuse, TYPE_SPINNER, EDITTYPE_FLOAT, ctrlID(), ctrlID(), 0.01f,
	PB_END,
	mtl_sssMix, _FT("sss_mix"), TYPE_FLOAT, P_ANIMATABLE, 0,
		p_default, 0.0f,
		p_range, 0.0f, 1.0f,
		p_ui, map_diffuse, TYPE_SPINNER, EDITTYPE_FLOAT, ctrlID(), ctrlID(), 0.1f,
	PB_END,

	mtl_color, _FT("reflection"), TYPE_RGBA, P_ANIMATABLE, 0,
		p_default, Color(0.0f, 0.0f, 0.0f),
		p_ui, map_reflect1, TYPE_COLORSWATCH, ctrlID(),
	PB_END,
	mtl_glossiness, _FT("glossiness"), TYPE_FLOAT, P_ANIMATABLE, 0,
		p_default, 0.5f,
		p_range, 0.0f, 1.0f,
		p_ui, map_reflect1, TYPE_SPINNER, EDITTYPE_FLOAT, ctrlID(), ctrlID(), 0.01f,
	PB_END,
	
	mtl_sssColor1, _FT("sss_color1"), TYPE_RGBA, P_ANIMATABLE, 0,
		p_default, Color(0.439f, 0.156f, 0.078f),
		p_ui, map_sss1, TYPE_COLORSWATCH, ctrlID(),
	PB_END,
	mtl_sssWeight1, _FT("sss_weight1"), TYPE_FLOAT, P_ANIMATABLE, 0,
		p_default, 1.0f,
		p_range, 0.0f, 1.0f,
		p_ui, map_sss1, TYPE_SPINNER, EDITTYPE_FLOAT, ctrlID(), ctrlID(), 0.01f,
	PB_END,
	mtl_sssRadius1, _FT("sss_radius1"), TYPE_FLOAT, P_ANIMATABLE, 0,
		p_default, 1.5f,
		p_range, 0.0f, 1e6f,
		p_ui, map_sss1, TYPE_SPINNER, EDITTYPE_FLOAT, ctrlID(), ctrlID(), 0.01f,
	PB_END,

	mtl_sssColor2, _FT("sss_color2"), TYPE_RGBA, P_ANIMATABLE, 0,
		p_default, Color(0.439f, 0.08f, 0.018f),
		p_ui, map_sss2, TYPE_COLORSWATCH, ctrlID(),
	PB_END,
	mtl_sssWeight2, _FT("sss_weight2"), TYPE_FLOAT, P_ANIMATABLE, 0,
		p_default, 1.0f,
		p_range, 0.0f, 1.0f,
		p_ui, map_sss2, TYPE_SPINNER, EDITTYPE_FLOAT, ctrlID(), ctrlID(), 0.01f,
	PB_END,
	mtl_sssRadius2, _FT("sss_radius2"), TYPE_FLOAT, P_ANIMATABLE, 0,
		p_default, 4.0f,
		p_range, 0.0f, 1e6f,
		p_ui, map_sss2, TYPE_SPINNER, EDITTYPE_FLOAT, ctrlID(), ctrlID(), 0.1f,
	PB_END,

	mtl_sssColor3, _FT("sss_color3"), TYPE_RGBA, P_ANIMATABLE, 0,
		p_default, Color(0.523f, 0.637f, 0.667f),
		p_ui, map_sss3, TYPE_COLORSWATCH, ctrlID(),
	PB_END,
	mtl_sssWeight3, _FT("sss_weight3"), TYPE_FLOAT, P_ANIMATABLE, 0,
		p_default, 1.0f,
		p_range, 0.0f, 1.0f,
		p_ui, map_sss3, TYPE_SPINNER, EDITTYPE_FLOAT, ctrlID(), ctrlID(), 0.01f,
	PB_END,
	mtl_sssRadius3, _FT("sss_radius3"), TYPE_FLOAT, P_ANIMATABLE, 0,
		p_default, 0.75f,
		p_range, 0.0f, 1e6f,
		p_ui, map_sss3, TYPE_SPINNER, EDITTYPE_FLOAT, ctrlID(), ctrlID(), 0.01f,
	PB_END,
PB_END
);

/*===========================================================================*\
 |	UI stuff
\*===========================================================================*/

class SkelMtlDlgProc : public ParamMap2UserDlgProc {
public:
	SkeletonMaterial *sm;

	SkelMtlDlgProc(void) { sm = NULL; }
	INT_PTR DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		int id = LOWORD(wParam);
		switch (msg) 
		{
			case WM_INITDIALOG:
				break;
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
			pmaps[i]=CreateMParamMap2(i, mtl->pblock, ip, hInstance, hWnd, NULL, NULL, tmp, pmapInfo[i].title, pmapInfo[i].flags, NULL);
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
	pblock=NULL;
	ivalid.SetEmpty();
	SkelMtlCD.MakeAutoParamBlocks(this);	// make and intialize paramblock2
}

ParamDlg* SkeletonMaterial::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp) {
	return new SkelMtlParamDlg(this, hwMtlEdit, imp);
	/*
	IAutoMParamDlg* masterDlg = SkelMtlCD.CreateParamDlgs(hwMtlEdit, imp, this);
	smtl_param_blk.SetUserDlgProc(new SkelMtlDlgProc(this));
	return masterDlg;
	*/
}

BOOL SkeletonMaterial::SetDlgThing(ParamDlg* dlg) {
	return FALSE;
}

Interval SkeletonMaterial::Validity(TimeValue t) {
	Interval temp=FOREVER;
	Update(t, temp);
	return ivalid;
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
				ParamID changing_param = pblock->LastNotifyParamID();
				smtl_param_blk.InvalidateUI(changing_param);
			}
			break;
	}
	return(REF_SUCCEED);
}

/*===========================================================================*\
 |	Standard IO
\*===========================================================================*/

#define MTL_HDR_CHUNK 0x4000

IOResult SkeletonMaterial::Save(ISave *isave) { 
	IOResult res;
	isave->BeginChunk(MTL_HDR_CHUNK);
	res = MtlBase::Save(isave);
	if (res!=IO_OK) return res;
	isave->EndChunk();
	return IO_OK;
}	

IOResult SkeletonMaterial::Load(ILoad *iload) { 
	IOResult res;
	int id;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(id = iload->CurChunkID())  {
			case MTL_HDR_CHUNK:
				res = MtlBase::Load(iload);
				break;
		}
		iload->CloseChunk();
		if (res!=IO_OK) return res;
	}
	return IO_OK;
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

		pblock->GetValue(mtl_glossiness, t, glossiness, ivalid);
		pblock->GetValue(mtl_color, t, reflect, ivalid);
		pblock->GetValue(mtl_diffuse, t, diffuse, ivalid);
		pblock->GetValue(mtl_opacity, t, opacity, ivalid);

		pblock->GetValue(mtl_sssMix, t, sssMix, ivalid);

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
	}

	valid &= ivalid;
}

void SkeletonMaterial::renderBegin(TimeValue t, VR::VRayRenderer *vray) {
	const VR::VRaySequenceData &sdata=vray->getSequenceData();
	bsdfPool.init(sdata.maxRenderThreads);
}

void SkeletonMaterial::renderEnd(VR::VRayRenderer *vray) {
	bsdfPool.freeMem();
	renderChannels.freeMem();
}
