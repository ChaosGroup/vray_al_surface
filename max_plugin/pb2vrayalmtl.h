#ifndef _PB2VRAYALMTL_H__
#define _PB2VRAYALMTL_H__

namespace VRayALMaterial {

#define VRAYALMTL_CLASSID    Class_ID(0x7d080943, 0x54fe7f7d)

	// Paramblock2 name
	enum { mtl_params, };

	// Parameter map IDs
	enum {
		map_basic,
		map_diffuse,
		map_sss1,
		map_sss2,
		map_sss3,
		map_reflect1,
		map_reflect2,
		map_options,
		map_textures,
		NUM_PMAPS
	};

	// Paramblock2 parameter list
	enum {
		mtl_reflect_roughness1,
		mtl_reflect_color1,

		mtl_diffuse,
		mtl_opacity,

		mtl_sss_mix,

		mtl_sss_weight1,
		mtl_sss_color1,
		mtl_sss_radius1,

		mtl_sss_weight2,
		mtl_sss_color2,
		mtl_sss_radius2,

		mtl_sss_weight3,
		mtl_sss_color3,
		mtl_sss_radius3,

		mtl_diffuse_strength,
		mtl_sss_density_scale,

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

		mtl_sss_mode,
		mtl_reflect_distribution1,
		mtl_reflect_distribution2,

		mtl_diffuse_bump_tex, mtl_diffuse_bump_tex_on, mtl_diffuse_bump_tex_mult,
		mtl_reflect_bump1_tex, mtl_reflect_bump1_tex_on, mtl_reflect_bump1_tex_mult,
		mtl_reflect_bump2_tex, mtl_reflect_bump2_tex_on, mtl_reflect_bump2_tex_mult,

		mtl_reflect_max_depth,
		mtl_reflect_subdivs,
		mtl_sss_subdivs,
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

} // namespace VRayALMaterial

#endif //_PB2VRAYALMTL_H__
