#pragma once

#include "glm.hpp"
#include "types.hpp"

namespace mls {

template<typename T>
inline T sqr(const T& value) {
    return value * value;
}

inline float distanceSquared(const float3& A, const float3& B) {
    auto v = B - A;
    return dot(v, v);
}

inline float lengthSquared(const float3& v) {
    return dot(v, v);
}

}
