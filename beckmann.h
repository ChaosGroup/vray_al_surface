#ifndef __BECKMANN_H__
#define __BECKMANN_H__

#include "utils.h"

namespace VUtils {

/// Given the incoming and outgoing directions the function computes the Beckmann BRDF value and the sampling probability for beckmannDir().
/// @param[in] viewDir The direction of the incoming/camera direction.
/// @param[in] lightDir The direction of the outgoing/light direction.
/// @param[in] roughness The roughness of the BRDF, between 0 and 1.
/// @param[in] normal The macronormal at the point of interest.
/// @param[in] nm Local to world space transformation matrix.
/// @param[in] inm World to local space transformation matrix.
/// @param[out] prob The probability to reflect exactly in the given outgoing direction
/// @return The brdf value for the given incoming and outgoing direction
float beckmannBRDF(const Vector &viewDir, const Vector &lightDir, float roughness, const Vector &normal, const Matrix &nm, const Matrix &inm, float &prob);

/// The function generates a reflection/outgoing direction given the incoming direction, the random
/// numbers and the Beckmann importance sampling equations. It also computes the probability and the 
/// brdf_divided_by_the_probability for the generated direction. 
/// @param[in] uc, vc Random numbers
/// @param[in] roughness The glossiness of the BRDF, between 0 and 1.
/// @param[in] viewDir The direction of the incoming/camera direction.
/// @param[in] nm Local to world space transformation matrix.
/// @param[out] prob The probability of the generated direction.
/// @param[out] brdfDivByProb The computed brdf value divided by the probability.
/// @return A reflection vector generated using the incoming direction, two random numbers and ggx importance sampling equations
Vector beckmannDir(float u, float v, float roughness, const Vector &viewDir, const Matrix &nm, float &prob, float &brdfDivByProb);

/// The functions computes the probability for a ray coming from direction viewDir to be reflected exactly in direction dir by beckmannDir().
/// @param[in] dir The direction of the outgoing/light direction.
/// @param[in] roughness The glossiness of the BRDF, between 0 and 1.
/// @param[in] normal The macronormal at the point of interest.
/// @param[in] viewDir The direction of the incoming/camera direction.
/// @param[in] inm World to local space transformation matrix.
/// @return The probability for a ray coming from direction viewDir to be reflected exactly in direction dir.
float beckmannDirProb(const Vector &dir, float roughness, const Vector &normal, const Vector &viewDir, const Matrix &inm);

} // namespace VUtils

#endif