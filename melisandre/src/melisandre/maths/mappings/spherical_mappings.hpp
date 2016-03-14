#pragma once

#include <melisandre/maths/numeric.hpp>
#include <melisandre/maths/constants.hpp>

#include "image_mappings.hpp"

namespace mls {

inline float2 getSphericalAngles(const float2& uv) {
    return uv * float2(two_pi<float>(), pi<float>());
}

// Standard spherical mapping [0,1]x[0,1] -> UnitSphere
inline float3 sphericalMapping(const float2& uv, float& sinTheta) {
    auto phiTheta = getSphericalAngles(uv);
    sinTheta = sin(phiTheta.y);

    return float3(cos(phiTheta.x) * sinTheta,
                 sin(phiTheta.x) * sinTheta,
                 cos(phiTheta.y));
}

inline float3 sphericalMapping(const float2& uv) {
    float sinTheta;
    return sphericalMapping(uv, sinTheta);
}

inline float sphericalMappingJacobian(const float2& uv, float& sinTheta) {
    auto phiTheta = getSphericalAngles(uv);
    sinTheta = sin(phiTheta.y);
    return abs(two_pi<float>() * pi<float>() * sinTheta);
}

inline float sphericalMappingJacobian(const float2& uv) {
    float sinTheta;
    return sphericalMappingJacobian(uv, sinTheta);
}

inline float2 rcpSphericalMapping(const float3& wi, float& sinTheta) {
    sinTheta = sqrt(sqr(wi.x) + sqr(wi.y));
    float2 phiTheta;

    phiTheta.x = atan2(wi.y / sinTheta, wi.x / sinTheta);
    phiTheta.y = atan2(sinTheta, wi.z);

    if(phiTheta.x < 0.f) {
        phiTheta.x += two_pi<float>();
    }

    if(phiTheta.y < 0.f) {
        phiTheta.y += two_pi<float>();
    }

    return phiTheta * float2(one_over_two_pi<float>(), one_over_pi<float>());
}

inline float2 rcpSphericalMapping(const float3& wi) {
    float sinTheta;
    return rcpSphericalMapping(wi, sinTheta);
}

inline float rcpSphericalMappingJacobian(const float3& wi, float& sinTheta) {
    sinTheta = sqrt(sqr(wi.x) + sqr(wi.y));
    return sinTheta == 0.f ? 0.f : abs(1.f / (two_pi<float>() * pi<float>() * sinTheta));
}

inline float rcpSphericalMappingJacobian(const float3& wi) {
    float sinTheta;
    return rcpSphericalMappingJacobian(wi, sinTheta);
}

}
