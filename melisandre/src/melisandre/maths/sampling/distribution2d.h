#pragma once

#include "distribution1d.h"

namespace mls
{
    inline size_t getDistribution2DBufferSize(size_t width, size_t height) {
        return height + 1 + height * (width + 1);
    }

    template<typename Functor>
    void buildDistribution2D(const Functor& function, real* pBuffer, size_t width, size_t height) {
        auto rowCDFSize = width + 1;
        auto colCDFSize = height + 1;

        auto ptr = pBuffer + colCDFSize;
        for (auto y = 0u; y < height; ++y) {
            buildDistribution1D([&](uint32_t x) {
                return function(x, y);
            }, ptr, width, pBuffer + y);
            ptr += rowCDFSize;
        }

        buildDistribution1D([&](uint32_t y) { return pBuffer[y]; }, pBuffer, height);
    }

    PlaneSample sampleContinuousDistribution2D(const real* pBuffer, size_t width, size_t height, const real2& s2D);

    Discrete2DSample sampleDiscreteDistribution2D(const real* pBuffer, size_t width, size_t height, const real2& s2D);

    real pdfContinuousDistribution2D(const real* pBuffer, size_t width, size_t height, const real2& point);

    real pdfDiscreteDistribution2D(const real* pBuffer, size_t width, size_t height, const uint2& pixel);
}
