#pragma once

#include <melisandre/maths/maths.hpp>
#include "Sample.hpp"

namespace mls {

////////////////////////////////////////////////////////////////////////////////
/// Sampling of Sphere
////////////////////////////////////////////////////////////////////////////////

inline real3 directionFromPhiCosThetaSinTheta(real phi, real cosTheta, real sinTheta) {
    return real3 {
        cos(phi) * sinTheta,
        sin(phi) * sinTheta,
        cosTheta
    };
}

/*! Uniform sphere sampling. */
inline DirectionSample uniformSampleSphere(real u, real v) {
    const auto phi = two_pi<real>() * u;
    const auto cosTheta = 1 - 2 * v, sinTheta = cos2sin(cosTheta);
    return DirectionSample { directionFromPhiCosThetaSinTheta(phi, cosTheta, sinTheta), one_over_four_pi<real>() };
}

/*! Computes the probability density for the uniform sphere sampling. */
inline real uniformSampleSpherePDF(const real3& direction) {
    return one_over_four_pi<real>();
}

inline real uniformSampleSpherePDF() {
    return one_over_four_pi<real>();
}

inline real2 rcpUniformSampleSphere(const real3& direction) {
    auto cosTheta = direction.z;
    auto v = 0.5 * (cosTheta - 1);
    auto phi = atan2(direction.y, direction.x);
    auto u = phi * one_over_two_pi<real>();
    return real2(u, v);
}

/*! Cosine weighted sphere sampling. Up direction is the z direction. */
inline DirectionSample cosineSampleSphere(real u, real v) {
    const real phi = two_pi<real>() * u;
    const real vv = 2.0f * (v - 0.5f);
    const real cosTheta = sign(vv) * sqrt(abs(vv));
    const real sinTheta = cos2sin(cosTheta);
    return DirectionSample(directionFromPhiCosThetaSinTheta(phi, cosTheta, sinTheta), 2.0f * cosTheta * one_over_pi<real>());
}

/*! Computes the probability density for the cosine weighted sphere sampling. */
inline real cosineSampleSpherePDF(const real3& s) {
    return abs(s.z) * two_over_pi<real>();
}

/*! Cosine weighted sphere sampling. Up direction is provided as argument. */
inline DirectionSample cosineSampleSphere(real u, real v, const real3& N) {
    DirectionSample s = cosineSampleSphere(u, v);
    return DirectionSample(frameZ(N) * s.value(), s.density());
}

/*! Computes the probability density for the cosine weighted sphere sampling. */
inline real cosineSampleSpherePDF(const real3& s, const real3& N) {
    return abs(dot(s, N)) * two_over_pi<real>();
}

////////////////////////////////////////////////////////////////////////////////
/// Sampling of Hemisphere
////////////////////////////////////////////////////////////////////////////////

/*! Uniform hemisphere sampling. Up direction is the z direction. */
inline DirectionSample uniformSampleHemisphere(real u, real v) {
    const real phi = two_pi<real>() * u;
    const real cosTheta = v, sinTheta = cos2sin(v);
    return DirectionSample(directionFromPhiCosThetaSinTheta(phi, cosTheta, sinTheta), one_over_two_pi<real>());
}

/*! Computes the probability density for the uniform hemisphere sampling. */
inline real uniformSampleHemispherePDF(const real3& s) {
    return s.z < 0.0f ? 0.0f : one_over_two_pi<real>();
}

/*! Uniform hemisphere sampling. Up direction is provided as argument. */
inline DirectionSample uniformSampleHemisphere(real u, real v, const real3& N) {
    DirectionSample s = uniformSampleHemisphere(u, v);
    return DirectionSample(frameZ(N) * s.value(), s.density());
}

/*! Computes the probability density for the uniform hemisphere sampling. */
inline real uniformSampleHemispherePDF(const real3& s, const real3& N) {
    return dot(s, N) < 0.0f ? 0.0f : one_over_two_pi<real>();
}

/*! Cosine weighted hemisphere sampling. Up direction is the z direction. */
// The pdf can be 0 if v = 0
inline DirectionSample cosineSampleHemisphere(real u, real v) {
    const real phi = two_pi<real>() * u;
    const real cosTheta = sqrt(v), sinTheta = sqrt(1.0f - v);
    return DirectionSample(directionFromPhiCosThetaSinTheta(phi, cosTheta, sinTheta), cosTheta * one_over_pi<real>());
}

inline real2 rcpCosineSampleHemisphere(const real3& d) {
    auto cosTheta = d.z;
    auto v = sqr(cosTheta);
    auto phi = atan2(d.y, d.x);
    auto u = phi * one_over_two_pi<real>();
    return real2(u, v);
}

/*! Computes the probability density for the cosine weighted hemisphere sampling. */
inline real cosineSampleHemispherePDF(const real3& s) {
    return max(0.f, s.z * one_over_pi<real>());
}

/*! Cosine weighted hemisphere sampling. Up direction is provided as argument. */
inline DirectionSample cosineSampleHemisphere(real u, real v, const real3& N) {
    DirectionSample s = cosineSampleHemisphere(u, v);
    return DirectionSample(frameZ(N) * real3(s.value()), s.density());
}

inline real2 rcpCosineSampleHemisphere(const real3& d, const real3& N) {
    return rcpCosineSampleHemisphere(inverse(frameZ(N)) * d);
}

/*! Computes the probability density for the cosine weighted hemisphere sampling. */
inline real cosineSampleHemispherePDF(const real3& s, const real3& N) {
    auto z = dot(s, N);
    return max(0.f, z * one_over_pi<real>());
}

/*! Samples hemisphere with power cosine distribution. Up direction
 *  is the z direction. */
inline DirectionSample powerCosineSampleHemisphere(real u, real v, real exp) {
    const real phi = two_pi<real>() * u;
    const real cosTheta = pow(v, 1.f / (exp + 1));
    const real sinTheta = cos2sin(cosTheta);
    return DirectionSample(directionFromPhiCosThetaSinTheta(phi, cosTheta, sinTheta),
                    (exp + 1.0f) * pow(cosTheta, exp) * one_over_two_pi<real>());
}

/*! Computes the probability density for the power cosine sampling of the hemisphere. */
inline real powerCosineSampleHemispherePDF(const real3& s, real exp) {
    return s.z < 0.0f ? 0.0f : (exp + 1.0f) * pow(s.z, exp) * one_over_two_pi<real>();
}

/*! Samples hemisphere with power cosine distribution. Up direction
 *  is provided as argument. */
inline DirectionSample powerCosineSampleHemisphere(real u, real v, const real3& N, real exp) {
    DirectionSample s = powerCosineSampleHemisphere(u,v,exp);
    return DirectionSample(frameZ(N) * real3(s.value()), s.density());
}

/*! Computes the probability density for the power cosine sampling of the hemisphere. */
inline real powerCosineSampleHemispherePDF(const real3& s, const real3& N, real exp) {
    auto z = dot(s, N);
    return z < 0.0f ? 0.0f : (exp + 1.0f) * pow(z, exp) * one_over_two_pi<real>();
}

////////////////////////////////////////////////////////////////////////////////
/// Sampling of Spherical Cone
////////////////////////////////////////////////////////////////////////////////

/*! Uniform sampling of spherical cone. Cone direction is the z
 *  direction. */
inline DirectionSample uniformSampleCone(real u, real v, real angle) {
    const real phi = two_pi<real>() * u;
    const real cosTheta = 1.0f - v * (1.0f - cos(angle));
    const real sinTheta = cos2sin(cosTheta);
    return DirectionSample(directionFromPhiCosThetaSinTheta(phi, cosTheta, sinTheta),
                    1.f / (four_pi<real>() * sqr(sin(0.5f * angle))));
}

/*! Computes the probability density of uniform spherical cone sampling. */
inline real uniformSampleConePDF(const real3& s, real angle) {
    return s.z < cos(angle) ? 0.0f : 1.f / (four_pi<real>() * sqr(sin(0.5f * angle)));
}

/*! Uniform sampling of spherical cone. Cone direction is provided as argument. */
inline DirectionSample uniformSampleCone(real u, real v, real angle, const real3& N) {
    DirectionSample s = uniformSampleCone(u, v, angle);
    return DirectionSample(frameZ(N) * s.value(), s.density());
}

/*! Computes the probability density of uniform spherical cone sampling. */
inline real uniformSampleConePDF(const real3& s, real angle, const real3& N) {
    return dot(s, N) < cos(angle) ? 0.0f : 1.f / (four_pi<real>() * sqr(sin(0.5f * angle)));
}

////////////////////////////////////////////////////////////////////////////////
/// Sampling of Triangle
////////////////////////////////////////////////////////////////////////////////

/*! Uniform sampling of triangle. */
inline real3 uniformSampleTriangle(real u, real v, const real3& A, const real3& B, const real3& C) {
    real su = sqrt(u);
    return C + (1.0f - su) * (A - C) + (v * su) * (B - C);
}

inline real2 uniformSampleTriangleUVs(real u, real v, const real3& A, const real3& B, const real3& C) {
    real su = sqrt(u);
    return real2(1.f - su, v * su);
}

// Uniform sample a spherical triangle.
// A, B and C must be located on the unit sphere centered on (0, 0, 0)
// Implements the algorithm described in "Stratified Sampling of Spherical Triangles" [Arvo95]
DirectionSample uniformSampleSphericalTriangle(real e1, real e2,
                                        const real3& A, const real3& B, const real3& C);

real sphericalTriangleArea(const real3& A, const real3& B, const real3& C);

inline real uniformSampleSphericalTrianglePDF(const real3& A, const real3& B, const real3& C) {
    return 1.f / sphericalTriangleArea(A, B, C);
}

////////////////////////////////////////////////////////////////////////////////
/// Sampling of Disk
////////////////////////////////////////////////////////////////////////////////

/*! Uniform sampling of disk. */
inline real2 uniformSampleDisk(const real2& sample, real radius) {
  const real r = sqrt(sample.x);
  const real theta = two_pi<real>() * sample.y;
  return radius * r * real2(cos(theta), sin(theta));
}

inline real uvToDualParaboloidPDF(real pdfWrtUV) {
    // TODO: I can't understand why i need to multiply
    // by two_pi but else it does not work
    return two_pi<real>() * pdfWrtUV * 0.125f;
}

inline real dualParaboloidJacobian(const real2& uv) {
    auto N = getDualParaboloidNormal(uv);
    auto ndc = real2(N);
    if(dot(ndc, ndc) > 1.f) {
        return 0.f;
    }
    auto d = dot(N, N);
    return 8.f / (d * d * d);
}

inline real dualParaboloidToSolidAnglePDF(real pdfWrtDualParaboloid, const real2& uv) {
    auto jacobian = dualParaboloidJacobian(uv);
    if(jacobian == 0.f) {
        return 0.f;
    }
    return pdfWrtDualParaboloid / jacobian;
}

inline real uvToSphericalAnglesPDF(real pdfWrtUV) {
    return pdfWrtUV * one_over_two_pi<real>() * one_over_pi<real>();
}

inline real sphericalAnglesToSolidAnglePDF(real pdfWrtPhiTheta, real rcpSinTheta) {
    return pdfWrtPhiTheta * rcpSinTheta;
}

inline real solidAngleToAreaPDF(real pdfWrtSolidAngle,
                                 real rcpSqrDist,
                                 real normalDotProduct) {
    return pdfWrtSolidAngle * max(0.f, normalDotProduct) * rcpSqrDist;
}

inline real areaToSolidAnglePDF(real pdfWrtArea,
                                 real sqrDist,
                                 real normalDotProduct) {
    return normalDotProduct <= 0.f ?
                0.f : pdfWrtArea * sqrDist / normalDotProduct;
}

}
