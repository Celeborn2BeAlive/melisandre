#pragma once

#include <ostream>
#include <melisandre/maths/types.hpp>

namespace mls {

template<typename T>
struct Sample {
    T value;
    float pdf = 0.f;

    Sample() = default;

    template<typename U>
    Sample(U&& value, float pdf): value(std::forward<U>(value)), pdf(pdf) {
    }

    /*
    explicit operator bool() const {
        return pdf > 0.f;
    }*/
};

using Sample1f = Sample<float>;
using Sample2f = Sample<float2>;
using Sample3f = Sample<float3>;
using Sample1u = Sample<uint32_t>;
using Sample2u = Sample<uint2>;

template<typename T>
inline std::ostream& operator <<(std::ostream& out, const Sample<T>& s) {
    out << "[ " << s.value << ", pdf = " << s.pdf << " ] ";
    return out;
}

}
