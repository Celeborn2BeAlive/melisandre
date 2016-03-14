#pragma once

#include "sample.hpp"
#include <iostream>
#include <algorithm>
//#include <melisandre/sys/threads.hpp>

namespace mls {

inline size_t getDistribution1DBufferSize(size_t size) {
    return size + 1;
}

// Build a 1D distribution for sampling
// - function(i) must returns the weight associating to the i-th element
// - size must be the number of elements
// - pCDF must point to a buffer containing getDistribution1DBufferSize(size) values and will
// be fill with the CDF of the distribution
// - if pSum != nullptr, then *pSum will be equal to the sum of the weights
//
// note: it is safe for function(i) to returns pCDF[i], so the buffer can be used to
// contain preprocessed weights as long as function(i) doesn't used pCDF[j] for j < i
template<typename Functor>
void buildDistribution1D(const Functor& function, real* pCDF, size_t size,
                         real* pSum = nullptr) {
    // Compute CDF
    real sum = 0.f;
    for(auto i = 0u; i < size; ++i) {
        auto tmp = function(i); // Call the function before erarsing pCDF[i] in case the function use this value
        pCDF[i] = sum; // Erase pCDF[i] with the partial sum
        sum += tmp; // Then update sum with the temporary value
    }
    pCDF[size] = sum;

    if(sum > 0.f) {
        // Normalize the CDF
        for (auto i = 0u; i <= size; ++i) {
            pCDF[i] = pCDF[i] / sum; // DON'T MULTIPLY BY rcpSum => can produce numerical errors
        }
    } else {
        pCDF[size] = 0.f;
    }

    if(pSum) {
        *pSum = sum;
    }
}

line_sample sampleContinuousDistribution1D(const real* pCDF, size_t size, real s1D);

discrete_1d_sample sampleDiscreteDistribution1D(const real* pCDF, size_t size, real s1D);

real pdfContinuousDistribution1D(const real* pCDF, size_t size, real x);

real pdfDiscreteDistribution1D(const real* pCDF, uint32_t idx);

real cdfDiscreteDistribution1D(const real* pCDF, uint32_t idx);

inline const bool isDiscreteCDF(const real* pCDF, size_t size) {
    return pCDF[size] > 0.f;
}

template<typename GetCDFPtrFunction>
real pdfCombinedDiscreteDistribution1D(size_t distributionCount, const GetCDFPtrFunction& getCDFPtr, uint32_t idx) {
    auto computeUnormalizedCDF = [&](uint32_t idx) {
        real sum = 0.f;
        for(auto i = 0u; i < distributionCount; ++i) {
            sum += getCDFPtr(i)[idx];
        }
        return sum;
    };
    return (computeUnormalizedCDF(idx + 1) - computeUnormalizedCDF(idx)) / distributionCount;
}

template<typename GetCDFPtrFunction>
discrete_1d_sample sampleCombinedDiscreteDistribution(std::size_t distributionCount, const GetCDFPtrFunction& getCDFPtr, std::size_t sampleCount, real s1D) {
    struct IteratorData {
        uint32_t m_nDistributionCount = 0u;
        const GetCDFPtrFunction* m_pGetCDFPtr = nullptr;
        uint32_t m_nSampleCount = 0u;
        real m_fRcpDistributionCount = 1.f / m_nDistributionCount;

        IteratorData(size_t distributionCount, const GetCDFPtrFunction* getCDFPtr, std::size_t sampleCount):
            m_nDistributionCount(distributionCount), m_pGetCDFPtr(getCDFPtr), m_nSampleCount(sampleCount) {
        }
    };

    struct iterator: public std::iterator<std::forward_iterator_tag, real, std::ptrdiff_t, const real*, const real> {
        iterator(const IteratorData* pData, std::size_t currentElement):
            m_pData(pData), m_nCurrentElement(currentElement) {
        }

        real computeCurrentValue() const {
            real sum = 0.f;
            for(auto i = 0u; i < m_pData->m_nDistributionCount; ++i) {
                sum += (*m_pData->m_pGetCDFPtr)(i)[m_nCurrentElement];
            }
            return sum * m_pData->m_fRcpDistributionCount;
        }

        iterator& operator++() {
            ++m_nCurrentElement;
            return *this;
        }

        reference operator*() const {
            return computeCurrentValue();
        }

        iterator operator++(int) {
            iterator copy { *this } ;
            ++(*this);
            return copy;
        }

        bool operator==(const iterator& rhs) {
            return m_nCurrentElement == rhs.m_nCurrentElement;
        }

        bool operator!=(const iterator& rhs) {
            return !(*this == rhs);
        }

        const IteratorData* m_pData = nullptr;
        std::size_t m_nCurrentElement = 0u;
    };

    IteratorData data { distributionCount, &getCDFPtr, sampleCount };
    iterator begin { &data, 0u }, end { &data, sampleCount };

    if(end.computeCurrentValue() == 0.f) {
        return discrete_1d_sample { 0u, 0.f };
    }

    auto it = std::upper_bound(begin, end, s1D);
    uint32_t i = clamp(int(it.m_nCurrentElement) - 1, 0, int(sampleCount) - 1);

    auto pdf = pdfCombinedDiscreteDistribution1D(distributionCount, getCDFPtr, i);

    return discrete_1d_sample(i, pdf);
}

}
