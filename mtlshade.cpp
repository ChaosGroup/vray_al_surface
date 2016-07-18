#include "vrayalmtl.h"

#include "vraybase.h"
#include "vrayinterface.h"
#include "shadedata_new.h"
#include "tomax.h"
#include "vraycore.h"
#include "vrayrenderer.h"

/*===========================================================================*\
 |	Determine the characteristics of the material
\*===========================================================================*/

void SkeletonMaterial::SetAmbient(Color c, TimeValue t) {}		
void SkeletonMaterial::SetDiffuse(Color c, TimeValue t) {}		
void SkeletonMaterial::SetSpecular(Color c, TimeValue t) {}
void SkeletonMaterial::SetShininess(float v, TimeValue t) {}
				
Color SkeletonMaterial::GetAmbient(int mtlNum, BOOL backFace) {
	return Color(0,0,0);
}

Color SkeletonMaterial::GetDiffuse(int mtlNum, BOOL backFace) {
	return Color(0.5f, 0.5f, 0.5f);
}

Color SkeletonMaterial::GetSpecular(int mtlNum, BOOL backFace) {
	return Color(0,0,0);
}

float SkeletonMaterial::GetXParency(int mtlNum, BOOL backFace) {
	return 0.0f;
}

float SkeletonMaterial::GetShininess(int mtlNum, BOOL backFace) {
	return 0.0f;
}

float SkeletonMaterial::GetShinStr(int mtlNum, BOOL backFace) {
	return 0.0f;
}

float SkeletonMaterial::WireSize(int mtlNum, BOOL backFace) {
	return 0.0f;
}

		
/*===========================================================================*\
 |	Actual shading takes place
\*===========================================================================*/

void SkeletonMaterial::Shade(ShadeContext &sc) {
	if (sc.ClassID()==VRAYCONTEXT_CLASS_ID)
		shade(static_cast<VR::VRayInterface&>(sc), gbufID);
	else {
		if (gbufID) sc.SetGBufferID(gbufID);
		sc.out.c.Black(); sc.out.t.Black();
	}
}

float SkeletonMaterial::EvalDisplacement(ShadeContext& sc)
{
	return 0.0f;
}

Interval SkeletonMaterial::DisplacementValidity(TimeValue t)
{
	Interval iv;
	iv.SetInfinite();
	return iv;
}
	
VR::BSDFSampler* SkeletonMaterial::newBSDF(const VR::VRayContext &rc, VR::VRenderMtlFlags flags) {
	MyALBSDF *bsdf=bsdfPool.newBRDF(rc);
	if (!bsdf) return NULL;

	VR::ALBSDFParams &params=bsdf->getParams();

	params.diffuse=toColor(diffuse)*diffuseStrength;

	params.reflectColor1=toColor(reflectColor1)*reflectStrength1;
	params.reflectRoughness1=reflectRoughness1;
	params.reflectIOR1=reflectIOR1;
	
	params.transparency=toColor(opacity).whiteComplement();
	params.subdivs=8;
	params.doubleSided=true;

	params.sssMix=sssMix;

	params.sssWeight1=sssWeight1;
	params.sssColor1=toColor(sssColor1);
	params.sssRadius1=sssRadius1;

	params.sssWeight2=sssWeight2;
	params.sssColor2=toColor(sssColor2);
	params.sssRadius2=sssRadius2;

	params.sssWeight3=sssWeight3;
	params.sssColor3=toColor(sssColor3);
	params.sssRadius3=sssRadius3;

	params.sssDensityScale=sssDensityScale;

	params.normalizeWeights();

	bsdf->init(rc);
	return bsdf;
}

void SkeletonMaterial::deleteBSDF(const VR::VRayContext &rc, VR::BSDFSampler *b) {
	if (!b) return;
	MyALBSDF *bsdf=static_cast<MyALBSDF*>(b);
	bsdfPool.deleteBRDF(rc, bsdf);
}

VR::VRayVolume* SkeletonMaterial::getVolume(const VR::VRayContext &rc) {
	return NULL;
}

void SkeletonMaterial::addRenderChannel(int index) {
	renderChannels+=index;
}
