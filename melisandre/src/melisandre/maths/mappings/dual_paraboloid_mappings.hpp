#pragma once

#include <melisandre/maths/numeric.hpp>
#include "image_mappings.hpp"

namespace mls {

inline float3 getDualParaboloidNormal(const float2& uv) {
    if(uv.x < 0.5f) {
        return float3(getNDC(uv, float2(0.5f, 1.f)), 1.f);
    }
    auto ndc = getNDC(uv - float2(0.5f, 0.f), float2(0.5f, 1.f));
    ndc.x = -ndc.x;
    return float3(ndc, -1.f);
}

inline float3 dualParaboloidMapping(const float2& uv) {
    auto N = getDualParaboloidNormal(uv);
    auto scale = 2.f / dot(N, N);
    return scale * N - float3(0, 0, N.z);
}

inline float2 rcpDualParaboloidMapping(const float3& wi) {
    if(wi.z > 0.f) {
        // left
        auto scaledN = wi + float3(0, 0, 1);
        auto N = scaledN / scaledN.z;
        return float2(0.25f * (N.x + 1.f), 0.5f * (N.y + 1.f));
    }
    //right
    auto scaledN = wi + float3(0, 0, -1);
    auto N = -scaledN / scaledN.z;
    return float2(0.5f + 0.25f * (-N.x + 1.f), 0.5f * (N.y + 1.f));
}

}
