#pragma once

#include <melisandre/maths/maths.hpp>
#include "Sample.hpp"

namespace mls {

////////////////////////////////////////////////////////////////////////////////
/// Sampling of Sphere
////////////////////////////////////////////////////////////////////////////////

/*! Uniform sphere sampling. */
inline Sample3f uniformSampleSphere(float u, float v) {
    const float phi = two_pi<float>() * u;
    const float cosTheta = 1.0f - 2.0f * v, sinTheta = 2.0f * sqrt(v * (1.0f - v));
    return Sample3f(float3(cos(phi) * sinTheta,
                          sin(phi) * sinTheta,
                          cosTheta), one_over_four_pi<float>());
}

/*! Computes the probability density for the uniform shere sampling. */
inline float uniformSampleSpherePDF() {
    return one_over_four_pi<float>();
}

inline float2 rcpUniformSampleSphere(const float3& wi) {
    auto cosTheta = wi.z;
    auto v = 0.5f * (cosTheta - 1.f);
    auto phi = atan2(wi.y, wi.x);
    auto u = phi * one_over_two_pi<float>();
    return float2(u, v);
}

/*! Cosine weighted sphere sampling. Up direction is the z direction. */
inline Sample3f cosineSampleSphere(float u, float v) {
    const float phi = two_pi<float>() * u;
    const float vv = 2.0f * (v - 0.5f);
    const float cosTheta = sign(vv) * sqrt(abs(vv));
    const float sinTheta = cos2sin(cosTheta);
    return Sample3f(float3(cos(phi) * sinTheta,
                            sin(phi) * sinTheta,
                            cosTheta), 2.0f * cosTheta * one_over_pi<float>());
}

/*! Computes the probability density for the cosine weighted sphere sampling. */
inline float cosineSampleSpherePDF(const float3& s) {
    return abs(s.z) * two_over_pi<float>();
}

/*! Cosine weighted sphere sampling. Up direction is provided as argument. */
inline Sample3f cosineSampleSphere(float u, float v, const float3& N) {
    Sample3f s = cosineSampleSphere(u, v);
    return Sample3f(frameZ(N) * s.value, s.pdf);
}

/*! Computes the probability density for the cosine weighted sphere sampling. */
inline float cosineSampleSpherePDF(const float3& s, const float3& N) {
    return abs(dot(s, N)) * two_over_pi<float>();
}

////////////////////////////////////////////////////////////////////////////////
/// Sampling of Hemisphere
////////////////////////////////////////////////////////////////////////////////

/*! Uniform hemisphere sampling. Up direction is the z direction. */
inline Sample3f uniformSampleHemisphere(float u, float v) {
    const float phi = two_pi<float>() * u;
    const float cosTheta = v, sinTheta = cos2sin(v);
    return Sample3f(float3(cos(phi) * sinTheta,
                              sin(phi) * sinTheta,
                              cosTheta), one_over_two_pi<float>());
}

/*! Computes the probability density for the uniform hemisphere sampling. */
inline float uniformSampleHemispherePDF(const float3& s) {
    return s.z < 0.0f ? 0.0f : one_over_two_pi<float>();
}

/*! Uniform hemisphere sampling. Up direction is provided as argument. */
inline Sample3f uniformSampleHemisphere(float u, float v, const float3& N) {
    Sample3f s = uniformSampleHemisphere(u, v);
    return Sample3f(frameZ(N) * s.value, s.pdf);
}

/*! Computes the probability density for the uniform hemisphere sampling. */
inline float uniformSampleHemispherePDF(const float3& s, const float3& N) {
    return dot(s, N) < 0.0f ? 0.0f : one_over_two_pi<float>();
}

/*! Cosine weighted hemisphere sampling. Up direction is the z direction. */
// The pdf can be 0 if v = 0
inline Sample3f cosineSampleHemisphere(float u, float v) {
    const float phi = two_pi<float>() * u;
    const float cosTheta = sqrt(v), sinTheta = sqrt(1.0f - v);
    return Sample3f(float3(cos(phi) * sinTheta,
                          sin(phi) * sinTheta,
                          cosTheta), cosTheta * one_over_pi<float>());
}

inline float2 rcpCosineSampleHemisphere(const float3& d) {
    auto cosTheta = d.z;
    auto v = sqr(cosTheta);
    auto phi = atan2(d.y, d.x);
    auto u = phi * one_over_two_pi<float>();
    return float2(u, v);
}

/*! Computes the probability density for the cosine weighted hemisphere sampling. */
inline float cosineSampleHemispherePDF(const float3& s) {
    return max(0.f, s.z * one_over_pi<float>());
}

/*! Cosine weighted hemisphere sampling. Up direction is provided as argument. */
inline Sample3f cosineSampleHemisphere(float u, float v, const float3& N) {
    Sample3f s = cosineSampleHemisphere(u, v);
    return Sample3f(frameZ(N) * float3(s.value), s.pdf);
}

inline float2 rcpCosineSampleHemisphere(const float3& d, const float3& N) {
    return rcpCosineSampleHemisphere(inverse(frameZ(N)) * d);
}

/*! Computes the probability density for the cosine weighted hemisphere sampling. */
inline float cosineSampleHemispherePDF(const float3& s, const float3& N) {
    auto z = dot(s, N);
    return max(0.f, z * one_over_pi<float>());
}

/*! Samples hemisphere with power cosine distribution. Up direction
 *  is the z direction. */
inline Sample3f powerCosineSampleHemisphere(float u, float v, float exp) {
    const float phi = two_pi<float>() * u;
    const float cosTheta = pow(v, 1.f / (exp + 1));
    const float sinTheta = cos2sin(cosTheta);
    return Sample3f(float3(cos(phi) * sinTheta,
                              sin(phi) * sinTheta,
                              cosTheta),
                    (exp + 1.0f) * pow(cosTheta, exp) * one_over_two_pi<float>());
}

/*! Computes the probability density for the power cosine sampling of the hemisphere. */
inline float powerCosineSampleHemispherePDF(const float3& s, float exp) {
    return s.z < 0.0f ? 0.0f : (exp + 1.0f) * pow(s.z, exp) * one_over_two_pi<float>();
}

/*! Samples hemisphere with power cosine distribution. Up direction
 *  is provided as argument. */
inline Sample3f powerCosineSampleHemisphere(float u, float v, const float3& N, float exp) {
    Sample3f s = powerCosineSampleHemisphere(u,v,exp);
    return Sample3f(frameZ(N) * float3(s.value), s.pdf);
}

/*! Computes the probability density for the power cosine sampling of the hemisphere. */
inline float powerCosineSampleHemispherePDF(const float3& s, const float3& N, float exp) {
    auto z = dot(s, N);
    return z < 0.0f ? 0.0f : (exp + 1.0f) * pow(z, exp) * one_over_two_pi<float>();
}

////////////////////////////////////////////////////////////////////////////////
/// Sampling of Spherical Cone
////////////////////////////////////////////////////////////////////////////////

/*! Uniform sampling of spherical cone. Cone direction is the z
 *  direction. */
inline Sample3f uniformSampleCone(float u, float v, float angle) {
    const float phi = two_pi<float>() * u;
    const float cosTheta = 1.0f - v * (1.0f - cos(angle));
    const float sinTheta = cos2sin(cosTheta);
    return Sample3f(float3(cos(phi) * sinTheta,
                              sin(phi) * sinTheta,
                              cosTheta),
                    1.f / (four_pi<float>() * sqr(sin(0.5f * angle))));
}

/*! Computes the probability density of uniform spherical cone sampling. */
inline float uniformSampleConePDF(const float3& s, float angle) {
    return s.z < cos(angle) ? 0.0f : 1.f / (four_pi<float>() * sqr(sin(0.5f * angle)));
}

/*! Uniform sampling of spherical cone. Cone direction is provided as argument. */
inline Sample3f uniformSampleCone(float u, float v, float angle, const float3& N) {
    Sample3f s = uniformSampleCone(u, v, angle);
    return Sample3f(frameZ(N) * s.value, s.pdf);
}

/*! Computes the probability density of uniform spherical cone sampling. */
inline float uniformSampleConePDF(const float3& s, float angle, const float3& N) {
    return dot(s, N) < cos(angle) ? 0.0f : 1.f / (four_pi<float>() * sqr(sin(0.5f * angle)));
}

////////////////////////////////////////////////////////////////////////////////
/// Sampling of Triangle
////////////////////////////////////////////////////////////////////////////////

/*! Uniform sampling of triangle. */
inline float3 uniformSampleTriangle(float u, float v, const float3& A, const float3& B, const float3& C) {
    float su = sqrt(u);
    return C + (1.0f - su) * (A - C) + (v * su) * (B - C);
}

inline float2 uniformSampleTriangleUVs(float u, float v, const float3& A, const float3& B, const float3& C) {
    float su = sqrt(u);
    return float2(1.f - su, v * su);
}

// Uniform sample a spherical triangle.
// A, B and C must be located on the unit sphere centered on (0, 0, 0)
// Implements the algorithm described in "Stratefied Sampling of Spherical Triangles" [Arvo95]
Sample3f uniformSampleSphericalTriangle(float e1, float e2,
                                        const float3& A, const float3& B, const float3& C);

float sphericalTriangleArea(const float3& A, const float3& B, const float3& C);

inline float uniformSampleSphericalTrianglePDF(const float3& A, const float3& B, const float3& C) {
    return 1.f / sphericalTriangleArea(A, B, C);
}

////////////////////////////////////////////////////////////////////////////////
/// Sampling of Disk
////////////////////////////////////////////////////////////////////////////////

/*! Uniform sampling of disk. */
inline float2 uniformSampleDisk(const float2& sample, float radius) {
  const float r = sqrt(sample.x);
  const float theta = two_pi<float>() * sample.y;
  return radius * r * float2(cos(theta), sin(theta));
}

inline float uvToDualParaboloidPDF(float pdfWrtUV) {
    // TODO: I can't understand why i need to multiply
    // by two_pi but else it doesnt work
    return two_pi<float>() * pdfWrtUV * 0.125f;
}

inline float dualParaboloidJacobian(const float2& uv) {
    auto N = getDualParaboloidNormal(uv);
    auto ndc = float2(N);
    if(dot(ndc, ndc) > 1.f) {
        return 0.f;
    }
    auto d = dot(N, N);
    return 8.f / (d * d * d);
}

inline float dualParaboloidToSolidAnglePDF(float pdfWrtDualParaboloid, const float2& uv) {
    auto jacobian = dualParaboloidJacobian(uv);
    if(jacobian == 0.f) {
        return 0.f;
    }
    return pdfWrtDualParaboloid / jacobian;
}

inline float uvToSphericalAnglesPDF(float pdfWrtUV) {
    return pdfWrtUV * one_over_two_pi<float>() * one_over_pi<float>();
}

inline float sphericalAnglesToSolidAnglePDF(float pdfWrtPhiTheta, float rcpSinTheta) {
    return pdfWrtPhiTheta * rcpSinTheta;
}

inline float solidAngleToAreaPDF(float pdfWrtSolidAngle,
                                 float rcpSqrDist,
                                 float normalDotProduct) {
    return pdfWrtSolidAngle * max(0.f, normalDotProduct) * rcpSqrDist;
}

inline float areaToSolidAnglePDF(float pdfWrtArea,
                                 float sqrDist,
                                 float normalDotProduct) {
    return normalDotProduct <= 0.f ?
                0.f : pdfWrtArea * sqrDist / normalDotProduct;
}

}
