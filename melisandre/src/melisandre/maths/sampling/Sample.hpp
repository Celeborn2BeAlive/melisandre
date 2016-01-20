#pragma once

#include <ostream>
#include <melisandre/maths/types.hpp>

namespace mls {

struct UnknownMeasure {

};

template<typename T, typename Measure = UnknownMeasure, typename RealType = real>
struct Sample {
    T value;
    RealType density = 0.f;

    Sample() = default;

    template<typename U, typename Real>
    Sample(U&& value, Real&& density): value(std::forward<U>(value)), density(std::forward<Real>(density)) {
    }

    explicit operator bool() const {
        return density > 0.f;
    }
};

using Sample1f = Sample<float>;
using Sample2f = Sample<float2>;
using Sample3f = Sample<float3>;
using Sample1u = Sample<uint32_t>;
using Sample2u = Sample<uint2>;

template<typename T>
inline std::ostream& operator <<(std::ostream& out, const Sample<T>& s) {
    out << "[ " << s.value << ", pdf = " << s.density << " ] ";
    return out;
}

}
