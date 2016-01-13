#pragma once

#include "../common.hpp"
#include "image_mappings.hpp"

namespace mls {

inline float2 getHemisphericalAngles(const float2& uv) {
    return uv * float2(two_pi<float>(), 0.5f * pi<float>());
}

inline float3 hemisphericalMapping(const float2& uv, float &sinTheta) {
    auto phiTheta = getHemisphericalAngles(uv);
    sinTheta = sin(phiTheta.y);

    return float3(cos(phiTheta.x) * sinTheta,
                 sin(phiTheta.x) * sinTheta,
                 cos(phiTheta.y));
}

inline float3 hemisphericalMapping(const float2& uv) {
    float sinTheta;
    return hemisphericalMapping(uv, sinTheta);
}

inline float hemisphericalMappingJacobian(const float2& uv) {
    auto phiTheta = getHemisphericalAngles(uv);
    return sin(phiTheta.y) * sqr(pi<float>());
}

inline float2 rcpHemisphericalMapping(const float3& wi, float& sinTheta) {
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

    return phiTheta * float2(one_over_two_pi<float>(), two_over_pi<float>());
}

inline float rcpHemisphericalMappingJacobian(const float3& wi) {
    auto sinTheta = sqrt(sqr(wi.x) + sqr(wi.y));
    if(sinTheta == 0.f) {
        return 0.f;
    }
    return 1.f / (sinTheta * sqr(pi<float>()));
}

inline float2 rcpHemisphericalMapping(const float3& wi) {
    float sinTheta;
    return rcpHemisphericalMapping(wi, sinTheta);
}

}
