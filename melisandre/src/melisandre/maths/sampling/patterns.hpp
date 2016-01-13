#pragma once

#include <bonez/maths/maths.hpp>
#include "Random.hpp"
#include "Sample.hpp"

namespace mls {

struct JitteredDistribution1D {
public:
    JitteredDistribution1D() = default;

    JitteredDistribution1D(size_t width):
        m_fDelta(1.f / width) {
    }

    // Return the i-th sample of the distribution for a given random value
    float getSample(uint32_t i, float s1D) const {
        return (i + s1D) * m_fDelta;
    }
private:
    float m_fDelta = 0.f;
};

template<typename OutputIterator>
void generateDistribution(size_t count, const JitteredDistribution1D& distribution, RandomGenerator& rng,
                          OutputIterator it) {
    for(auto i = 0u; i < count; ++i) {
        (*it)++ = distribution.getSample(i, rng.getFloat());
    }
}

class JitteredDistribution2D {
public:
    JitteredDistribution2D() = default;

    JitteredDistribution2D(size_t gridWidth, size_t gridHeight):
        m_nGridWidth(gridWidth),
        m_fDelta(1.f / gridWidth, 1.f / gridHeight) {
    }

    float2 getSample(uint32_t x, uint32_t y, const float2& s2D) const {
        return (float2(x, y) + s2D) * m_fDelta;
    }

    // Return the i-th sample of the distribution for a given random value
    float2 getSample(uint32_t i, const float2& s2D) const {
        auto x = i % m_nGridWidth;
        auto y = i / m_nGridWidth;
        return getSample(x, y, s2D);
    }
private:
    uint32_t m_nGridWidth = 0u;
    float2 m_fDelta = zero<float2>();
};

template<typename OutputIterator>
void generateDistribution(size_t count, const JitteredDistribution2D& distribution, RandomGenerator& rng,
                          OutputIterator it) {
    for(auto i = 0u; i < count; ++i) {
        (*it)++ = distribution.getSample(i, rng.getFloat2());
    }
}

inline Sample1u uniformDiscreteSample(float s1D, std::size_t count) {
    return count ? Sample1u(clamp(std::size_t(s1D * (count - 1)),
                                  std::size_t(0),
                                  count), 1.f / count) : Sample1u(0, 0.f);
}

inline float uniformDiscretePdf(std::size_t count) {
    return count ? 1.f / count : 0.f;
}

}
