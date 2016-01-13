#pragma once

#include "types.hpp"

namespace mls {

    struct Col3f : public float3 {
        Col3f() = default;

        template<typename T>
        Col3f(T&& v) : float3(std::move(v)) {
        }
    };

    struct Col4f : public float4 {
        Col4f() = default;

        template<typename T>
        Col4f(T&& v) : float4(std::move(v)) {
        }
    };

    inline float3 badColor() {
        return float3(199, 21, 133) / 255.f;
    }

}