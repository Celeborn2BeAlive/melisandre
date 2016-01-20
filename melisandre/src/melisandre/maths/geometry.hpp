#pragma once

#include "glm.hpp"

namespace mls {

using glm::distance;
using glm::length;
using glm::dot;
using glm::cross;
using glm::normalize;

using glm::translate;
using glm::rotate;
using glm::scale;
using glm::perspective;
using glm::ortho;
using glm::lookAt;
using glm::inverse;
using glm::determinant;

template<typename T, typename U>
inline float distanceSquared(const float3& A, const float3& B) {
    auto v = B - A;
    return dot(v, v);
}

inline float lengthSquared(const float3& v) {
    return dot(v, v);
}

}