#include "sss.h"

const float ScatteringProfileDirectional::eta(1.3f);
const float ScatteringProfileDirectional::C_phi(0.175626f);
const float ScatteringProfileDirectional::C_phi_inv(1.03668f);
const float ScatteringProfileDirectional::C_E(0.27735f);
const float ScatteringProfileDirectional::_3C2(0.0611156f);
const float ScatteringProfileDirectional::A(2.05736f);
const float ScatteringProfileDirectional::_albedo_lut[SSS_ALBEDO_LUT_SZ] = {0.004660, 0.005024, 0.005285, 0.005503, 0.005697, 0.005875, 0.006041, 0.006197, 0.006345, 0.006488, 0.006625, 0.006758, 0.006887, 0.007013, 0.007136, 0.007256, 0.007374, 0.007490, 0.007604, 0.007716, 0.007826, 0.007935, 0.008043, 0.008149, 0.008255, 0.008358, 0.008461, 0.008564, 0.008665, 0.008765, 0.008865, 0.008964, 0.009063, 0.009160, 0.009257, 0.009354, 0.009450, 0.009546, 0.009641, 0.009735, 0.009829, 0.009923, 0.010016, 0.010110, 0.010203, 0.010295, 0.010387, 0.010479, 0.010571, 0.010662, 0.010754, 0.010845, 0.010935, 0.011026, 0.011116, 0.011207, 0.011297, 0.011387, 0.011477, 0.011566, 0.011656, 0.011746, 0.011835, 0.011925, 0.012014, 0.012103, 0.012192, 0.012281, 0.012370, 0.012459, 0.012548, 0.012637, 0.012726, 0.012815, 0.012904, 0.012993, 0.013082, 0.013171, 0.013260, 0.013349, 0.013438, 0.013527, 0.013616, 0.013705, 0.013794, 0.013883, 0.013972, 0.014061, 0.014150, 0.014240, 0.014329, 0.014418, 0.014508, 0.014597, 0.014687, 0.014777, 0.014866, 0.014956, 0.015046, 0.015136, 0.015226, 0.015317, 0.015407, 0.015497, 0.015588, 0.015678, 0.015769, 0.015860, 0.015951, 0.016042, 0.016133, 0.016224, 0.016315, 0.016407, 0.016498, 0.016590, 0.016682, 0.016774, 0.016866, 0.016958, 0.017051, 0.017143, 0.017236, 0.017329, 0.017421, 0.017514, 0.017608, 0.017701, 0.017794, 0.017888, 0.017982, 0.018076, 0.018170, 0.018264, 0.018359, 0.018453, 0.018548, 0.018643, 0.018738, 0.018833, 0.018928, 0.019024, 0.019120, 0.019215, 0.019311, 0.019408, 0.019504, 0.019601, 0.019698, 0.019795, 0.019892, 0.019990, 0.020087, 0.020185, 0.020283, 0.020381, 0.020479, 0.020577, 0.020676, 0.020774, 0.020873, 0.020973, 0.021072, 0.021172, 0.021272, 0.021372, 0.021472, 0.021573, 0.021673, 0.021774, 0.021875, 0.021976, 0.022078, 0.022180, 0.022282, 0.022383, 0.022486, 0.022588, 0.022690, 0.022794, 0.022897, 0.023001, 0.023104, 0.023208, 0.023311, 0.023417, 0.023521, 0.023626, 0.023730, 0.023837, 0.023942, 0.024047, 0.024153, 0.024260, 0.024366, 0.024472, 0.024579, 0.024687, 0.024794, 0.024901, 0.025010, 0.025118, 0.025226, 0.025335, 0.025444, 0.025554, 0.025663, 0.025773, 0.025883, 0.025992, 0.026104, 0.026214, 0.026326, 0.026436, 0.026548, 0.026661, 0.026772, 0.026886, 0.026997, 0.027111, 0.027225, 0.027338, 0.027452, 0.027567, 0.027682, 0.027795, 0.027911, 0.028027, 0.028143, 0.028259, 0.028376, 0.028491, 0.028609, 0.028726, 0.028844, 0.028962, 0.029082, 0.029201, 0.029320, 0.029439, 0.029559, 0.029678, 0.029800, 0.029921, 0.030041, 0.030164, 0.030286, 0.030409, 0.030531, 0.030655, 0.030778, 0.030903, 0.031028, 0.031151, 0.031277, 0.031403};

ScatteringProfileDirectional::ScatteringProfileDirectional(float Rd, float scale)
{
	// get temp reduced scattering coefficients from Rd
	const float ap = computeAlphaPrime(Rd * 0.439f);
	const float str = 1.0f / Rd;
	const float stp = str / sqrtf(3.0f * (1.0f - ap));

	// calculate actual scattering coefficients from the temps
	float sigma_s_prime = stp * ap;
	float sigma_a = stp - sigma_s_prime;

	// factor of 2.5 is eyeballed to roughly match the look of the cubic
	sigma_s_prime *= scale * VR::pi() * 2;
	sigma_a *= scale * VR::pi() * 2;

	const float sigma_s = sigma_s_prime;
	// safe_radius = 0.5f / sigma_s;
	safe_radius = 0.0f;

	sigma_t_prime = sigma_s_prime + sigma_a;
	sigma_t = sigma_s + sigma_a;

	alpha_prime = sigma_s_prime / sigma_t_prime;

	D = (2*sigma_t_prime) / (3*VR::sqr(sigma_t_prime));
	sigma_tr = sqrt(sigma_a / D);
	de = 2.131 * D / sqrt(alpha_prime);
	zr = 1.0f / sigma_t_prime;

	vassert(VR::fastfinite(D));
	vassert(D > 0.0f);
	vassert(VR::fastfinite(de));
	vassert(VR::fastfinite(sigma_tr));
	vassert(VR::fastfinite(sigma_t_prime));
	vassert(VR::fastfinite(alpha_prime));
	vassert(VR::fastfinite(zr));

	const float maxdist = zr * SSS_MAX_RADIUS;

	// grab the precalculated albedo for this Rd
	const int idx = int(Rd * (SSS_ALBEDO_LUT_SZ-1));
	albedo = _albedo_lut[idx];
	vassert(albedo > 1e-6f);
	vassert(albedo < 1.0f - 1e-6f);
	// printf("Rd: %f - albedo: %f\n", Rd, albedo);
}

ScatteringProfileDirectional::ScatteringProfileDirectional(float sigma_s, float sigma_a, float g)
{
	float sigma_s_prime = sigma_s * (1.0f - g);
	sigma_t_prime = sigma_s_prime + sigma_a;
	sigma_t = sigma_s + sigma_a;

	safe_radius = 0.5f / sigma_s;

	alpha_prime = sigma_s_prime / sigma_t_prime;

	D = (2*sigma_t_prime) / (3*VR::sqr(sigma_t_prime));
	sigma_tr = sqrt(sigma_a / D);
	de = 2.131 * D / sqrt(alpha_prime);
	zr = 1.0f / sigma_t_prime;

	vassert(VR::fastfinite(D));
	vassert(VR::fastfinite(de));
	vassert(VR::fastfinite(sigma_tr));
	vassert(VR::fastfinite(sigma_t_prime));
	vassert(VR::fastfinite(alpha_prime));
	vassert(VR::fastfinite(zr));

	const float maxdist = zr * SSS_MAX_RADIUS;
	albedo = 1.0f;
	// printf("FUCK\n");
}

void matchNormals(const VR::Vector Nref, VR::Vector &Nmatch)
{
	if (VR::dotf(Nref, Nmatch) < 0.0f)
	{
		Nmatch = -Nmatch;
	}
}

struct ALSIrradiateBRDF: VR::BRDFSampler {
	ALSIrradiateBRDF(DirectionalMessageData *data, const VR::Color &Rnond, DiffusionSample& samp):dmd(data), Rnond(Rnond), samp(samp) {}

	VR::Color getDiffuseColor(VR::Color &lightColor) VRAY_OVERRIDE {
		VR::Color res=lightColor*Rnond;
		lightColor.makeZero();
		return res;
	}

	VR::Color getLightMult(VR::Color &lightColor) VRAY_OVERRIDE {
		VR::Color res=lightColor*Rnond;
		lightColor.makeZero();
		return res;
	}

	VR::Color eval(const VR::VRayContext &rc, const VR::Vector &lightDir, VR::Color &shadowedLight, VR::Color &origLight, float probLight, int flags) VRAY_OVERRIDE {
		VR::Color L(0.0f, 0.0f, 0.0f);

		if (0!=(flags & FBRDF_DIFFUSE)) {
			float cs=VR::Max(VR::dotf(lightDir, rc.rayresult.normal), 0.0f);
			float probReflection=2.0f*cs;

			float k=VR::getReflectionWeight(probLight, probReflection); // MIS weight according to indirect sampling
			L=(0.5f*probReflection*k)*shadowedLight;

			if (!L.isBlack()) {
				if (dmd->directional) {
					VR::Color R(0.0f, 0.0f, 0.0f);
					for (int c=0; c < dmd->numComponents; ++c)
					{
						if (samp.r < dmd->sp[c].safe_radius)
							R += directionalDipole(rc.rayresult.wpoint, rc.rayresult.normal, dmd->Po, dmd->No, rc.rayresult.normal, dmd->No, dmd->sp[c]) * dmd->weights[c];
						else
							R += directionalDipole(rc.rayresult.wpoint, rc.rayresult.normal, dmd->Po, dmd->No, lightDir, dmd->wo, dmd->sp[c]) * dmd->weights[c]; 
					}
					L *= R;
				}
				else
				{
					L *= Rnond;
				}
			}
		}
		vassert(VR::fastfinite(L.sum()));

		shadowedLight.makeZero();
		origLight.makeZero();

		return L;
	}
private:
	DirectionalMessageData *dmd;
	const VR::Color &Rnond;
	DiffusionSample &samp;
};

void alsIrradiateSample(
	VR::VRayContext &rc,
	DirectionalMessageData *dmd,
	VR::Vector U, VR::Vector V,
	float sssMix
)
{
	if (dmd->sss_depth >= SSS_MAX_SAMPLES)
		return;
	
	//void *orig_op;
	//AiStateGetMsgPtr("als_sss_op", &orig_op);
	//
	//int als_context = ALS_CONTEXT_NONE;
	//AiStateGetMsgInt("als_context", &als_context);

	//AtRay ray;
	//AtScrSample scrs;

	// There are a few contexts in which we can be called here:
	// 1) Intersecting same object with same shader
	// 2) Intersecting same object with different shader
	// 3) Intersecting different object with same shader
	// 4) Intersecting different object with different shader
	// 5) Called from an indirect ray from another shader while tracing for SSS
	// 2 and 4 could possibly be because we're running from a layer shader. if we try to evaluate in that context we will simply crash
	// 5 will cause fireflies so we need to detect this and return

	// just return if this is a different object.
	// More elaborate checks may be needed; see VRayFastSSS2 source, considerPointForSSS().
	if (rc.parent && rc.rayresult.sb != rc.parent->rayresult.sb)
	{
		return;
	}

	// if we're in a layered context and running a different shader from the one that's tracing for sss, just return to let the other shader handle it
	//if (als_context == ALS_CONTEXT_LAYER && (dmd->shader_orig != sg->shader))
	//{
	//    return;
	//}

	DiffusionSample& samp = dmd->samples[dmd->sss_depth];
	samp.S = (rc.rayresult.wpoint - dmd->Po);
	samp.r = VR::length(samp.S);
	dmd->maxdist -= samp.r;
	samp.Rd.set(0.0f, 0.0f, 0.0f);

	// if we're not using trace sets to explicitly define what objects we want to trace against, then assume we only want to trace against ourselves
	// or if we're not doing sss in this shader, just continue the ray
	/*
	if ((!trace_set_enabled && orig_op != sg->Op) || sssMix == 0.0f)
	{
		if (dmd->sss_depth < SSS_MAX_SAMPLES && dmd->maxdist > 0.0f)
		{
			AiMakeRay(&ray, AI_RAY_SUBSURFACE, &sg->P, &sg->Rd, dmd->maxdist, sg);
			AiTrace(&ray, &scrs);
		}
		return;
	}*/

	// AiStateSetMsgInt("als_raytype", ALS_RAY_UNDEFINED);
	
	// assert(sg->P != dmd->Po);
	// put normals the right way round
	VR::Vector Nref = rc.rayresult.normal;
	matchNormals(Nref, rc.rayresult.gnormal);
	matchNormals(Nref, rc.rayresult.origNormal);

	// void* brdf_data = AiOrenNayarMISCreateData(sg, 0.0f);
	// AiLightsPrepare(sg);
	VR::Color result_direct(0.0f, 0.0f, 0.0f);

	VR::Color Rnond(0.0f, 0.0f, 0.0f);
	bool directional = dmd->directional;

	// In V-Ray, always compute dipole diffuse reflectance; we use this as approximation
	// when V-Ray needs to know the diffuse surface color.
	for (int c = 0; c < dmd->numComponents; ++c)
	{
		Rnond += directionalDipole(rc.rayresult.wpoint, rc.rayresult.normal, dmd->Po, dmd->No, rc.rayresult.normal, dmd->No, dmd->sp[c]) * dmd->weights[c];
	}

	// Calculate the direct lighting.
	ALSIrradiateBRDF brdf(dmd, Rnond, samp);
	result_direct=rc.vray->getSequenceData().directLightManager->eval(rc, brdf);

	/*
	while (AiLightsGetSample(sg))
	{
		// get the group assigned to this light from the hash table using the light's pointer
		int lightGroup = lightGroupMap[sg->Lp];
		float diffuse_strength = AiLightGetDiffuse(sg->Lp);

		// Does not using MIS here get us anything?
		// AtRGB L = AiEvaluateLightSample(sg, brdf_data, AiOrenNayarMISSample, AiOrenNayarMISBRDF, AiOrenNayarMISPDF);
		VR::Color L = sg->Li * VR::Max(VR::dotf(sg->Ld, sg->N), 0.0f) * (1.0f/VR::pi()) * sg->we * diffuse_strength;
		if (L.isBlack()) continue;
		if (directional)
		{
			VR::Color R(0.0f, 0.0f, 0.0f);
			for (int c=0; c < dmd->numComponents; ++c)
			{
				if (samp.r < dmd->sp[c].safe_radius)
					R += directionalDipole(sg->P, sg->N, dmd->Po, dmd->No, sg->N, dmd->No, dmd->sp[c]) * dmd->weights[c];
				else
					R += directionalDipole(sg->P, sg->N, dmd->Po, dmd->No, sg->Ld, dmd->wo, dmd->sp[c]) * dmd->weights[c]; 
			}
			L *= R;
			result_direct += L;
			vassert(VR::fastfinite(result_direct.sum()));
		}
		else
		{
			L *= Rnond;
			result_direct += L;
			vassert(VR::fastfinite(result_direct.sum()));
		}
	}*/

	// Calculate the indirect lighting.
	VR::Color result_indirect(0.0f, 0.0f, 0.0f);

	VR::VRayContext &nrc=rc.newSpawnContext(0, VR::Color(1.0f, 1.0f, 1.0f), VR::RT_INDIRECT | VR::RT_ENVIRONMENT, rc.rayresult.gnormal);

	for (int i=0; i<1; i++)
	{
		float samples[2];
		samples[0]=nrc.getDMCValue();
		samples[1]=nrc.getDMCValue();

		float stheta = sqrtf(float(samples[0]));
		float phi = float(VR::pi()*2.0f * samples[1]);

		VR::Vector dir;
		dir.x = stheta * cosf(phi);
		dir.y = stheta * sinf(phi);
		float cs=sqrtf(VR::Max(0.0f, 1.0f - float(samples[0])));
		dir.z = cs;
		
		dir=dir.x*U+dir.y*V+dir.z*rc.rayresult.normal;

		nrc.setTracedDir(dir);
		nrc.rayparams.rayProbability=2.0f*cs;

		VR::Color giColor=nrc.traceCurrentRay();

		VR::Color f;
		if (directional)
		{
			VR::Color R(0.0f, 0.0f, 0.0f);
			for (int c = 0; c < dmd->numComponents; ++c)
			{
				R += directionalDipole(rc.rayresult.wpoint, rc.rayresult.normal, dmd->Po, dmd->No, dir, dmd->wo, dmd->sp[c]) * dmd->weights[c];
			}
			f = R;
			result_indirect += giColor * R;
			vassert(VR::fastfinite(result_indirect.sum()));
		}
		else
		{
			f = Rnond;
			result_indirect += giColor * Rnond;
			vassert(VR::fastfinite(result_indirect.sum()));
		}
	}
	nrc.releaseContext();

	// TODO: this is guaranteed to be 1 in every case, right?
	// result_indirect *= AiSamplerGetSampleInvCount(sampit);
	samp.Rd = result_direct + result_indirect;

	vassert(VR::fastfinite(samp.Rd.sum()));

   
	samp.N = rc.rayresult.normal;
	samp.Ng = rc.rayresult.gnormal;
	samp.P = rc.rayresult.wpoint;

	dmd->sss_depth++;

	//AiStateSetMsgInt("als_raytype", ALS_RAY_SSS);
	//
	//if (dmd->sss_depth < SSS_MAX_SAMPLES && dmd->maxdist > 0.0f)
	//{
	//    
	//    AiMakeRay(&ray, AI_RAY_SUBSURFACE, &sg->P, &sg->Rd, dmd->maxdist, sg);
	//    AiTrace(&ray, &scrs);
	//}
}

VR::Color alsDiffusion(
	VR::VRayContext &rc,
	DirectionalMessageData *dmd,
	bool directional,
	int numComponents,
	float sssMix
)
{
	VR::Vector U, V;
	VR::computeTangentVectors(rc.rayresult.gnormal, U, V);

	numComponents=VR::Min(numComponents, SSS_MAX_PROFILES);
	float l = 0.0f;
	float inv_pdf_sum = 0.0f;
	float comp_pdf[9];
	float comp_cdf[9+1];
	comp_cdf[0] = 0.0f;
	int last_nonzero = numComponents;
	for (int i=0; i < numComponents; ++i)
	{
		// dmd->sp[i] = ScatteringProfileDirectional(Rd[i], sssDensityScale/radii[i]);
		float w = (dmd->weights[i]).maxComponentValue();
		float pdf = dmd->sp[i].alpha_prime;
		comp_pdf[i] = pdf * w;
		comp_cdf[i+1] = comp_cdf[i] + comp_pdf[i];
		inv_pdf_sum += comp_pdf[i];

		if (w > 0.0f)
		{
			// track the last non-zero weight we encounter so that we can ignore completely missing lobes
			last_nonzero = i+1;  
			// track the largest mean free path so we can set that to be our maximum raytracing distance
			l = VR::Max(l, dmd->sp[i].zr);
		} 
	}

	// set the number of components to be the number of non-zero-weight components
	numComponents = VR::Min(numComponents, last_nonzero);
	// set the maximum raytracing distance to be some multiple of the largest mean-free path
	// the choice of SSS_MAX_RADIUS is a quality/speed tradeoff. The default of 25 seems to work well for most cases
	const float R_max = l * SSS_MAX_RADIUS;

	// normalize the PDF and CDF
	inv_pdf_sum = 1.0f / inv_pdf_sum;
	for (int i=0; i < numComponents; ++i)
	{
		comp_pdf[i] *= inv_pdf_sum;
		comp_cdf[i+1] *= inv_pdf_sum;
	}

	// trick Arnold into thinking we're shooting from a different face than we actually are so he doesn't ignore intersections
	// void *old_fi = rc.rayresult.skipTag;
	// rc.rayresult.skipTag = NULL;

	VR::Color result_sss(0.0f, 0.0f, 0.0f);
	
	VR::Color Rd_sum(0.0f, 0.0f, 0.0f);
	int samplesTaken = 0;

	VR::Ray wi_ray;
	// VR::IntersectionData scrs;
	
	dmd->wo = -rc.rayparams.viewDir;
	dmd->numComponents = numComponents;
	dmd->directional = directional;
	
	VR::Vector axes[3] = { U, V, rc.rayresult.gnormal };
	double sss_samples_c = 0;

	VR::VRayContext &nrc=rc.newSpawnContext(0, VR::Color(1.0f, 1.0f, 1.0f), 0, rc.rayresult.gnormal);

	for (int i=0; i<1; i++)
	{
		float samples[2];
		samples[0]=nrc.getDMCValue();
		samples[1]=nrc.getDMCValue();

		float dx, dy;

		VR::Vector Wsss, Usss, Vsss, Usss_1, Vsss_1, Usss_2, Vsss_2;
		float c_axis = 1.0f, c_axis_1, c_axis_2;
		float axes_c[3] = {0.25f, 0.25f, 0.5f};
		int chosen_axis;
		if (samples[0] < 0.5f)
		{
			chosen_axis = 2;
			samples[0] *= 2.0f;
			c_axis = 0.5f;
			c_axis_1 = 0.25f;
			c_axis_2 = 0.25f;
			Wsss = rc.rayresult.gnormal;
			
			Usss = U;
			Vsss = V;

			Usss_1 = rc.rayresult.gnormal;
			Vsss_1 = V;

			Usss_2 = U;
			Vsss_2 = rc.rayresult.gnormal;
		}
		else if (samples[0] < 0.75f)
		{
			chosen_axis = 0;
			samples[0] = (samples[0] - 0.5f) * 4.0f;
			c_axis = 0.25f;
			c_axis_1 = 0.5f;
			c_axis_2 = 0.25f;
			Wsss = U;
			
			Usss = rc.rayresult.gnormal;
			Vsss = V;

			Usss_1 = U;
			Vsss_1 = V;

			Usss_2 = U;
			Vsss_2 = rc.rayresult.gnormal;
		}
		else
		{
			chosen_axis = 1;
			samples[0] = (1.0f-samples[0])* 4.0f;
			c_axis = 0.25f;
			c_axis_1 = 0.25f;
			c_axis_2 = 0.5f;
			Wsss = V;
			
			Usss = U;
			Vsss = rc.rayresult.gnormal;

			Usss_1 = rc.rayresult.gnormal;
			Vsss_1 = V;

			Usss_2 = U;
			Vsss_2 = V;
		}

		float r_disk[3];
		for (int i=0; i < numComponents; ++i)
		{
			if (samples[1] < comp_cdf[i+1])
			{
				samples[1] -= comp_cdf[i];
				samples[1] /= comp_pdf[i];
				diffusionSampleDisk(samples[0], samples[1], dmd->sp[i].sigma_tr, dx, dy, r_disk[0]);
				break;
			}
		}

		// if the sampled distance is greater than we'll consider anyway, don't bother tracing at all.
		if (r_disk[0] > R_max) continue;

		VR::Vector dir = -Wsss;
		float dz = R_max;
		VR::Vector origin = rc.rayresult.wpoint + Wsss*(dz*.25) + Usss * dx + Vsss * dy;
		float maxdist = R_max ;//* 2.0f;

		wi_ray.p=origin;
		wi_ray.dir=dir;
		// AiMakeRay(&wi_ray, AI_RAY_SUBSURFACE, &origin, &dir, maxdist, sg);
		VR::setTracedRay(nrc, wi_ray, 0.0f, maxdist);
		// AiMakeRay(&wi_ray, AI_RAY_SUBSURFACE, &origin, &dir, maxdist, sg);
		// AiStateSetMsgInt("als_raytype", ALS_RAY_SSS);
		// AiStateSetMsgPtr("als_sss_op", sg->Op);

		dmd->sss_depth = 0;
		dmd->maxdist = R_max;
		dmd->Po = rc.rayresult.wpoint;
		dmd->No = rc.rayresult.normal;
		float geom[3];
		
		memset(dmd->samples, 0, sizeof(DiffusionSample)*SSS_MAX_SAMPLES);
		dmd->sss_depth = 0;

		VR::IntersectionData isData;
		for (int i=0; i<SSS_MAX_SAMPLES && dmd->maxdist>0.0f; i++) {
			isData.clear();
			int res=nrc.vray->findIntersection(nrc, &isData);
			if (!res) break;

			// Shade the hit (result is stored in i-th element of the samples array in the dmd structure)
			nrc.setRayResult(res, &isData, i);
			alsIrradiateSample(nrc, dmd, U, V, sssMix);

			// Process the hit
			if (!dmd->samples[i].Rd.isBlack()) {
				geom[0] = fabsf(VR::dotf(dmd->samples[i].Ng, Wsss));
				geom[1] = fabsf(VR::dotf(dmd->samples[i].Ng, Usss));
				geom[2] = fabsf(VR::dotf(dmd->samples[i].Ng, Vsss));

				float r_u_1 = VR::dotf(dmd->samples[i].S, Usss_1);
				float r_v_1 = VR::dotf(dmd->samples[i].S, Vsss_1);
				r_disk[1] = sqrtf(VR::sqr(r_u_1)+VR::sqr(r_v_1));

				float r_u_2 = VR::dotf(dmd->samples[i].S, Usss_2);
				float r_v_2 = VR::dotf(dmd->samples[i].S, Vsss_2);
				r_disk[2] = sqrtf(VR::sqr(r_u_2)+VR::sqr(r_v_2));

				float pdf_sum = 0.0f;

				for (int c=0; c < numComponents; ++c)
				{
					pdf_sum += diffusionPdf(r_disk[0], dmd->sp[c].sigma_tr) * comp_pdf[c] * geom[0] * c_axis;
					pdf_sum += diffusionPdf(r_disk[1], dmd->sp[c].sigma_tr) * comp_pdf[c] * geom[1] * c_axis_1; 
					pdf_sum += diffusionPdf(r_disk[2], dmd->sp[c].sigma_tr) * comp_pdf[c] * geom[2] * c_axis_2;   
				}

				if (pdf_sum!=0.0f) {
					float f = r_disk[0] / pdf_sum;
					result_sss += dmd->samples[i].Rd * f;
				}
			}

			vassert(VR::fastfinite(result_sss.sum()));

			// Continue the ray
			nrc.rayparams.skipTag=nrc.rayresult.skipTag;
			nrc.rayparams.mint=nrc.rayresult.wpointCoeff;
		}
	}
	vassert(VR::fastfinite(result_sss.sum()));

	nrc.releaseContext();

	float w = 1.0f; // AiSamplerGetSampleInvCount(sampit);
	result_sss *= w;
	vassert(VR::fastfinite(result_sss.sum()));

	VR::Color norm_factor(0.0f, 0.0f, 0.0f);
	for (int c=0; c < numComponents; ++c)
	{
		norm_factor += dmd->sp[c].albedo * dmd->weights[c];
	}

	result_sss /= norm_factor;

	vassert(VR::fastfinite(result_sss.sum()));
	// rc.rayresult.skipTag = old_fi;

	// AiStateSetMsgInt("als_raytype", ALS_RAY_UNDEFINED);

	return result_sss;
}