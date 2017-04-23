#include "utils.h"
#include "beckmann.h"
#include "misc_ray.h"

using namespace VUtils;

float evalBeckmannMicrofacetDistribution(float cosThetaM, float beckmannAlpha) {
	if (cosThetaM<1e-6f) return 0.0f;

	float cosThetaM2=cosThetaM*cosThetaM;
	float tanThetaM2=1.0f/cosThetaM2-1.0f;

	float cosThetaM4=cosThetaM2*cosThetaM2;
	float alpha2=beckmannAlpha*beckmannAlpha;

	float factor=1.0f/(pi()*alpha2*cosThetaM4);
	float arg=-tanThetaM2/alpha2;
	float exponent=expf(arg);

	return factor*exponent;
}

float evalBeckmannShadowingG1(float cosThetaV, float beckmannAlpha) {
	if (cosThetaV<1e-6f)
		return 0.0f;

	float sinThetaV=sqrtf(Max(0.0f, 1.0f-cosThetaV*cosThetaV));
	float tanThetaV=sinThetaV/cosThetaV;

	float divd=(beckmannAlpha*tanThetaV);
	if (divd<1e-6f)
		return 0.0f;

	float a=1.0f/divd;

	if (a>=1.6f)
		return 1.0f;
	
	float denominator = 1.0f+a*(2.276f+a*2.577f);

	if (denominator<1e-6f)
		return 0.0f;

	float numerator = a*(3.535f+a*2.181f);
	return numerator/denominator;
}

float evalBeckmannShadowingSmithApprox(float cosThetaIV, float cosThetaOV, float beckmannAlpha) {
	return evalBeckmannShadowingG1(cosThetaIV, beckmannAlpha)*evalBeckmannShadowingG1(cosThetaOV, beckmannAlpha);
}

simd::Vector3f sampleBeckmannMicronormal(float uc, float vc, float beckmannAlpha) {
	float arg=-sqr(beckmannAlpha)*logf(1.0f-uc);
	float theta=atanf(sqrtf(arg));
	float phi=2.0f*pi()*vc;

	float thetaSin=sinf(theta);
	float thetaCos=cosf(theta);
	return simd::Vector3f(cosf(phi)*thetaSin, sinf(phi)*thetaSin, thetaCos);
}

/// Given the incoming and outgoing directions the function computes the partial brdf value, the partial
/// probability and the microfacet distribution value (D). Full values of the brdf and the probability can
/// be computed if the partial values are multiplied by the term D*2*pi. The separation is done to optimize
/// the calculations after calling ggxDir and to handle better the case when D gets zero.
/// @param[in] viewDir The direction of the incoming/camera direction.
/// @param[in] lightDir The direction of the outgoing/light direction.
/// @param[in] halfVector The micronormal at the point of interest in world space.
/// @param[in] halfVectorLocal The micronormal at the point of interest in local space.
/// @param[in] roughness The roughness of the BRDF, between 0 and 1.
/// @param[in] normal The macronormal at the point of interest.
/// @param[out] partialProb The computed probability w/o D*2*pi term.
/// @param[out] Dval The computed microfacet distribution value.
/// @return The partial brdf value for the given incoming and outgoing directions.
inline float _beckmannBRDF(const simd::Vector3f &viewDir, const simd::Vector3f &lightDir, const simd::Vector3f &halfVector, const simd::Vector3f &halfVectorLocal, float roughness, const simd::Vector3f &normal, float &partialProb, float &Dval) {
	simd::Vector3f incomingDir=-viewDir;
	
	float cosIN=fabsf(dotf(incomingDir, normal));
	float cosON=dotf(lightDir, normal);

	if(cosIN <= 1e-6f || cosON <= 1e-6f)
		return 0.f;
	
	float partialBrdf=0.f;
	
	Dval=evalBeckmannMicrofacetDistribution(halfVectorLocal.z(), roughness);
	partialBrdf = 0.25f * 
		evalBeckmannShadowingSmithApprox(cosIN, cosON, roughness)
		/cosIN;

	float hn=halfVectorLocal.z(); //*/ dotf(halfVector, normal);
	if (hn>0.0f) {
		float ho=dotf(halfVector, lightDir);
		partialProb=ho>0.0f? 0.25f/ho : 0.0f;
	}

	return partialBrdf;
}

float VUtils::beckmannBRDF3f(const simd::Vector3f &viewDir, const simd::Vector3f &lightDir, float roughness, const simd::Vector3f &normal, const simd::Matrix3x3f &nm, const simd::Matrix3x3f &inm, float &prob) {
	simd::Vector3f halfVector=simd::normalize0(lightDir-viewDir);
	simd::Vector3f halfVectorLocal=simd::normalize0(inm*halfVector);

	float Dval=0.0f;
	float partialProb=0.0f;
	float partialBrdf=_beckmannBRDF(viewDir, lightDir, halfVector, halfVectorLocal, roughness, normal, partialProb, Dval);

	// compute full brdf and probability, and apply vray specific corrections
	float fullBrdf=partialBrdf*Dval*2.0f*pi();
	prob=partialProb*Dval*2.0f*pi();
	
	// Compute the length of the half-vector in local space and correct probabilities for anisotropy
	simd::Vector3f microNormal=nm*halfVectorLocal;
	float L2=microNormal.lengthSqr();
	float L=sqrtf(L2);
	prob*=L*L2;

	return fullBrdf;
}

simd::Vector3f VUtils::beckmannDir3f(float u, float v, float roughness, const simd::Vector3f &viewDir, const simd::Matrix3x3f &nm, float &prob, float &brdfDivByProb) {
	simd::Vector3f microNormalLocal=sampleBeckmannMicronormal(u, v, roughness);
	if (microNormalLocal.z()<=0.0f) {
		prob=0.0f;
		return nm[2];
	}

	simd::Vector3f microNormal=nm*microNormalLocal;

	// Compute and keep the length of the half-vector in local space; needed for anisotropy correction
	float L2=microNormal.lengthSqr();
	float L=sqrtf(L2);
	microNormal/=L;
	
	simd::Vector3f res=getReflectDir3f(viewDir, microNormal);

	float Dval=0.0f;
	float partialProb=0.0f;
	float partialBrdf=_beckmannBRDF(viewDir, res, microNormal, microNormalLocal, roughness, nm[2], partialProb, Dval);
	partialProb*=L*L2; // take anisotropy in consideration
	prob=(Dval>=1e-6f) ? partialProb*Dval*2.0f*pi() : LARGE_FLOAT; // compute full probability and apply vray specific corrections
	brdfDivByProb=(partialProb>=1e-6f) ? partialBrdf/partialProb : 0.0f;

	return res;
}

float VUtils::beckmannDirProb3f(const simd::Vector3f &dir, float roughness, const simd::Vector3f &normal, const simd::Vector3f &viewDir, const simd::Matrix3x3f &inm) {
	float prob=0.0f;

	simd::Vector3f h=normalize(dir-viewDir);
	float Dval=evalBeckmannMicrofacetDistribution(dotf(h, normal), roughness);
	float hn=dotf(h, normal);
	if (hn>0.0f) {
		prob=Dval*hn;

		float ho=dotf(h, dir);
		prob*=ho>0.0f? 0.25f/ho : 0.0f;
	}

	// vray specific corrections
	prob*=2.0f*pi();
	return prob;
}
