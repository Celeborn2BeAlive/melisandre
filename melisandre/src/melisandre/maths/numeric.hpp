#pragma once

#include "glm.hpp"
#include "constants.hpp"

namespace mls {

template<typename T>
inline T rcp(const T& value) {
    return one<T>() / value;
}

template<typename T>
inline T sqr(const T& value) {
    return value * value;
}

using glm::abs;

using glm::radians;
using glm::degrees;

using glm::cos;
using glm::sin;
using glm::tan;
using glm::abs;
using glm::floor;
using glm::sign;
using glm::max;
using glm::min;
using glm::clamp;
using glm::fract;
using glm::pow;
using glm::sqrt;

using glm::isnan;
using glm::isinf;
using glm::isfinite;
using glm::isPowerOfTwo;

}