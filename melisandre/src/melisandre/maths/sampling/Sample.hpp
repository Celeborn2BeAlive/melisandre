#pragma once

#include <ostream>
#include <melisandre/maths/types.hpp>

#include "measures.hpp"

namespace mls {

template<typename T, typename Measure = UnknownMeasure, typename RealType = real>
struct Sample {
    Sample() = default;

    template<typename U, typename Real>
    explicit Sample(U&& value, Real&& density): 
        m_Value(std::forward<U>(value)),
        m_Density(std::forward<Real>(density)) {
    }

    explicit operator bool() const {
        return density > RealType{ 0 };
    }

    const T& value() const {
        return m_Value;
    }

    RealType density() const {
        return m_Density;
    }

    RealType rcpDensity() const {
        return m_Density ? 1.f / m_Density : 0;
    }

private:
    T m_Value;
    RealType m_Density = RealType{ 0 };
};

using DirectionSample = Sample<real3, SolidAngleMeasure>;
using PlaneSample = Sample<real2, PlaneMeasure>;
using LineSample = Sample<real, LineMeasure>;
using Discrete1DSample = Sample<size_t, DiscreteMeasure>;
using Discrete2DSample = Sample<size2, DiscreteMeasure>;

template<typename T, typename Measure, typename RealType>
inline std::ostream& operator <<(std::ostream& out, const Sample<T, Measure, RealType>& s) {
    out << "[ " << s.value() << ", pdf = " << s.density() << " ] ";
    return out;
}

}
