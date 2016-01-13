#pragma once

#include "../common.hpp"
#include "image_mappings.hpp"

namespace mls {

inline float3 getParaboloidNormal(const float2& uv) {
    return float3(getNDC(uv, float2(1.f, 1.f)), 1.f);
}

inline float3 paraboloidMapping(const float2& uv) {
    auto N = getParaboloidNormal(uv);
    auto scale = 2.f / dot(N, N);
    return scale * N - float3(0, 0, N.z);
}

inline float2 rcpParaboloidMapping(const float3& wi) {
    auto scaledN = wi + float3(0, 0, 1);
    auto N = scaledN / scaledN.z;
    return float2(0.5f * (N.x + 1.f), 0.5f * (N.y + 1.f));
}

inline float paraboloidMappingJacobian(const float2& uv) {
    auto N = getParaboloidNormal(uv);
    auto d = dot(N, N);
    return 16.f / (d * d);
}

}
