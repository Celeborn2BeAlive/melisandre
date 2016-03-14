#pragma once

#include "glm.hpp"
#include "types.hpp"
#include "maths.hpp"

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
inline auto sqr_distance(const T& lhs, const U& rhs) {
    auto v = rhs - lhs;
    return dot(v, v);
}

template<typename T>
inline auto sqr_length(const T& v) {
    return dot(v, v);
}

template<typename T, typename U>
inline auto abs_dot(const T& lhs, const U& rhs) {
    return abs(dot(lhs, rhs));
}

}