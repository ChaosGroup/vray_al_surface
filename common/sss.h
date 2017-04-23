#pragma once

#include "vrayinterface.h"

#define SSS_MAX_SAMPLES 16
#define SSS_MAX_RADIUS 25.0f
#define SSS_ALBEDO_LUT_SZ 256
#define SSS_MAX_PROFILES 9

struct ScatteringProfileDirectional
{
	ScatteringProfileDirectional(void) {}
	ScatteringProfileDirectional(float Rd, float scale);
	ScatteringProfileDirectional(float sigma_s, float sigma_a, float g);

	const static float eta;
	const static float C_phi;
	const static float C_phi_inv;
	const static float C_E;
	const static float _3C2;
	const static float A;
	const static float _albedo_lut[SSS_ALBEDO_LUT_SZ];

	// numerically calculate Rd from alpha_prime according to the better dipole model.
	// see http://graphics.pixar.com/library/TexturingBetterDipole/paper.pdf
	// {
	float computeRd(float alpha_prime_c)
	{
		float _4A = (1.0f + _3C2) / C_phi;
		float sigma_tr_D = sqrtf((1.0f-alpha_prime_c) * (2.0f-alpha_prime_c) / 3.0f);
		float ex = expf(-_4A * sigma_tr_D);
		return 0.5f * VR::sqr(alpha_prime_c)
					* expf(-sqrtf(3.0f*(1.0f - alpha_prime_c)/(2.0f-alpha_prime_c))) 
					* (C_E * (1.0f+ex) + C_phi/sigma_tr_D * (1.0f-ex));
	}

	float computeAlphaPrime(float rd)
	{
		int i, niter = 50;
		float x0 = 0, x1 = 1;
		float xmid, fmid;

		for (i=0; i < niter; ++i)
		{
			xmid = 0.5f * (x0+x1);
			fmid = computeRd(xmid);
			fmid < rd ? x0 = xmid : x1 = xmid;
		}

		return xmid;
	}
	// }

	float de;
	float safe_radius;
	float sigma_t;
	float sigma_t_prime;
	float sigma_tr;
	float D;
	float zr;
	float alpha_prime;
	float albedo;
	float pdf;
};

struct DiffusionSample {
	VR::Vector P;     //< sampled point
	VR::Vector N;     //< normal at sample
	VR::Vector Ng;    //< geometric normal at sample
	VR::Color R;        //< diffusion result at sample
	VR::Vector S;     //< vector from shading point to sample
	float r;        //< distance from shading point to sample
	float b;        //< bounce attenuation factor
};

struct DirectionalMessageData
{
	int sss_depth;
	float maxdist;
	VR::Vector Po;
	VR::Vector No;
	VR::Vector wo;
	ScatteringProfileDirectional sp[9];
	VR::Color weights[9];
	int numComponents;
	bool directional;
	DiffusionSample samples[SSS_MAX_SAMPLES];
	int sssSurfaceID;
};

inline float diffusionSampleDistance(float u1, float sigma)
{
	return -logf(1.0f - VR::Min(0.999999f,u1)) / sigma;
}

inline float diffusionSampleDisk(float u1, float u2, float sigma, float& dx, float& dy, float& r)
{
	r = -logf(1.0f - VR::Min(0.999999f,u1)) / sigma;
	float phi = u2 * VR::pi()*2.0f;
	dx = r * cosf(phi);
	dy = r * sinf(phi);

	// return pdf
	return sigma * expf(-sigma * r);
}

inline float diffusionPdf(float r, float sigma)
{
	return sigma * expf(-sigma * r);
}

inline float dipoleProfileRd(float r, float sigma_tr, float zr, float zv)
{
	float dr = sqrtf(r*r + zr*zr);
	float dv = sqrtf(r*r + zv*zv);
	dr = VR::Max(dr, zr);
	dv = VR::Max(dv, zv);

	float inv_dr3 = 1.0f / (dr*dr*dr);
	float inv_dv3 = 1.0f / (dv*dv*dv);

	float sigma_tr_dr = sigma_tr * dr;
	float sigma_tr_dv = sigma_tr * dv;

	return zr*(sigma_tr_dr+1.0f) * expf(-sigma_tr_dr) * inv_dr3 + zv*(sigma_tr_dv+1.0f) * expf(-sigma_tr_dv) * inv_dv3;
}

inline VR::Color dipoleProfileRd(float r, const VR::Color& sigma_tr, const VR::Color& zr, const VR::Color& zv)
{
	return VR::Color(
		dipoleProfileRd(r, sigma_tr.r, zr.r, zv.r),
		dipoleProfileRd(r, sigma_tr.g, zr.g, zv.g),
		dipoleProfileRd(r, sigma_tr.b, zr.b, zv.b)
	);
}

// Directional dipole profile evaluation
// see: http://www.ci.i.u-tokyo.ac.jp/~hachisuka/dirpole.pdf
// and: http://www.ci.i.u-tokyo.ac.jp/~hachisuka/dirpole.cpp
inline float Sp_d(const VR::Vector x, const VR::Vector w, const float r, const VR::Vector n, const float sigma_tr, const float D, const float Cp_norm, 
					const float Cp, const float Ce) 
{
	// evaluate the profile
	const float s_tr_r = sigma_tr * r;
	const float s_tr_r_one = 1.0f + s_tr_r;
	const float x_dot_w = VR::dotf(x, w);
	const float r_sqr = r * r;

	const float t0 = Cp_norm * (1.0f / (4.0f * VR::pi() * VR::pi())) * expf(-s_tr_r) / (r * r_sqr);
	const float t1 = r_sqr / D + 3.0f * s_tr_r_one * x_dot_w;
	const float t2 = 3.0f * D * s_tr_r_one * VR::dotf(w, n);
	const float t3 = (s_tr_r_one + 3.0f * D * (3.0f * s_tr_r_one + s_tr_r * s_tr_r) / r_sqr * x_dot_w) * VR::dotf(x, n);

	const float Sp = t0 * (Cp * t1 - Ce * (t2 - t3));
	vassert(VR::fastfinite(Sp));
	return VR::Max(Sp, 0.0f);
	return Sp;
}

inline float directionalDipole(VR::Vector xi, VR::Vector ni, VR::Vector xo, VR::Vector no, VR::Vector wi, VR::Vector wo, ScatteringProfileDirectional& sp)
{
	vassert(sp.D > 0.0f);
	// distance
	VR::Vector xoxi = xo - xi;
	float r = VR::length(xoxi);
	if (r == 0.0f) return 0.0f;

	// modified normal
	VR::Vector ni_s = VR::crossf(VR::normalize(xoxi), VR::normalize(VR::crossf(ni, xoxi)));

	// directions of ray sources
	float nnt = 1.0f / sp.eta;
	float ddn = -VR::dotf(wi, ni);
	VR::Vector wr = VR::normalize(wi * -nnt - ni * (ddn * nnt + sqrtf(1.0f - nnt*nnt * (1.0f - ddn*ddn))));
	VR::Vector wv = wr - ni_s * (2.0f * VR::dotf(wr, ni_s));

	// distance to real sources
	const float cos_beta = -sqrtf(VR::Max(0.f,(r * r - VR::dotf(xoxi, wr) * VR::dotf(xoxi, wr)) / (r * r + sp.de * sp.de)));
	float dr;
	const float mu0 = -VR::dotf(ni, wr);
	if (mu0 > 0.0) 
	{
		dr = sqrtf((sp.D * mu0) * ((sp.D * mu0) - sp.de * cos_beta * 2.0f) + r * r);
	} 
	else 
	{
		dr = sqrtf(1.0f / (3.0f * sp.sigma_t * 3.0f * sp.sigma_t) + r * r);
	}

	VR::Vector xoxv = xo - (xi + ni_s * (2.0f * sp.A * sp.de));
	const float dv = VR::length(xoxv);

	const float real = Sp_d(xoxi, wr, dr, no, sp.sigma_tr, sp.D, sp.C_phi_inv, sp.C_phi, sp.C_E);
	const float virt = Sp_d(xoxv, wv, dv, no, sp.sigma_tr, sp.D, sp.C_phi_inv, sp.C_phi, sp.C_E);
	const float result = real - virt;
	vassert(VR::fastfinite(result));
	return VR::Max(0.0f, result); 
}

// Called at secondary hits of sub-surface sample rays. Return the index
// in dmd where the result is stored, or -1 if no result is computed.
int alsIrradiateSample(
	VR::VRayContext &rc,
	DirectionalMessageData *dmd,
	const VR::Color &diffuse,
	VR::ShadeResult &result // The SSS result is returned in result.color, along with any light select elements.
);

// Called for a direct hit of a ray with a SSS material surface.
void alsDiffusion(
	VR::VRayContext &rc,
	DirectionalMessageData *dmd,
	bool directional,
	int numComponents,
	float sssMix,
	const VR::Color &diffuse,
	int nsamples,
	VR::ShadeResult &result // The SSS result is returned here in result.color along with any light select elements.
);
