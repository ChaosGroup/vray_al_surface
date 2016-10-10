#ifndef __VRAYALSURFACE_DEFAULT_VALUES_H__
#define __VRAYALSURFACE_DEFAULT_VALUES_H__

// Note that in the 3ds Max material, Color is the 3ds Max Color class, whereas in the
// standalone shader, it is the VR::Color class.

const float defaultOpacity=1.0f;

#define defaultDiffuse Color(0.5f, 0.5f, 0.5f)
const float defaultDiffuseStrength=1.0f;

#define defaultReflect1 Color(1.0f, 1.0f, 1.0f)
const float defaultReflect1Strength=1.0f;
const float defaultReflect1Roughness=0.5f;
const float defaultReflect1IOR=1.4f;
const int defaultReflect1Distribution=0;

#define defaultReflect2 Color(1.0f, 1.0f, 1.0f)
const float defaultReflect2Strength=0.0f;
const float defaultReflect2Roughness=0.5f;
const float defaultReflect2IOR=1.4f;
const int defaultReflect2Distribution=0;

const int defaultSSSMode=0;
const float defaultSSSScale=1.0f;
const float defaultSSSMix=0.0f;

const float defaultSSSWeight1=1.0f;
#define defaultSSSColor1 Color(0.439f, 0.156f, 0.078f)
const float defaultSSSRadius1=1.5f;

const float defaultSSSWeight2=1.0f;
#define defaultSSSColor2 Color(0.439f, 0.08f, 0.018f)
const float defaultSSSRadius2=4.0f;

const float defaultSSSWeight3=1.0f;
#define defaultSSSColor3 Color(0.523f, 0.637f, 0.667f)
const float defaultSSSRadius3=0.75f;

const int defaultReflectMaxDepth=5;
const int defaultReflectSubdivs=8;
const int defaultSSSSubdivs=8;

#endif