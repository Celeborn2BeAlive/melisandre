#pragma once

#include <ostream>

#include <melisandre/maths/types.hpp>
#include <melisandre/maths/numeric.hpp>
#include <melisandre/maths/constants.hpp>

#include "measures.hpp"

namespace mls {

template<typename ValueType, typename MeasureType = unknown_measure, typename RealType = real>
class sample {
public:
    using value_type = ValueType;
    using measure_type = MeasureType;
    using real_type = RealType;

    sample() = default;

    template<typename ValueType, typename RealType>
    explicit sample(ValueType&& value, RealType&& density):
        m_Value(std::forward<ValueType>(value)),
        m_RcpDensity(rcp(density)) {
    }

    explicit operator bool() const {
        return m_RcpDensity > zero<real_type>();
    }

    const value_type& value() const {
        return m_Value;
    }

    real_type density() const {
        return m_RcpDensity ? rcp(m_RcpDensity) : zero<real_type>();
    }

    real_type rcp_density() const {
        return m_RcpDensity;
    }

private:
    value_type m_Value;
    real_type m_RcpDensity = zero<real_type>();
};

using direction_sample = sample<real3, solid_angle_measure>;
using plane_sample = sample<real2, plane_measure>;
using line_sample = sample<real, line_measure>;
using discrete_1d_sample = sample<size_t, discrete_measure>;
using discrete_2d_sample = sample<size2, discrete_measure>;

template<typename T, typename Measure, typename RealType>
inline std::ostream& operator <<(std::ostream& out, const sample<T, Measure, RealType>& s) {
    out << "[ " << s.value() << ", pdf = " << s.density() << " ] ";
    return out;
}

}
