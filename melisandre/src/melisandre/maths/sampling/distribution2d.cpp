// ======================================================================== //
// Copyright 2009-2012 Intel Corporation                                    //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#include "distribution2d.h"
#include <vector>
#include <iostream>
#include <melisandre/maths/maths.hpp>

namespace mls {

PlaneSample sampleContinuousDistribution2D(const real* pBuffer, size_t width, size_t height, const real2& s2D) {
    assert(height > 0);
    auto sy = sampleContinuousDistribution1D(pBuffer, height, s2D.y);
    auto idx = clamp(size_t(sy.value()), size_t{ 0 }, height - 1);

    auto sx = sampleContinuousDistribution1D(pBuffer + height + 1 + idx * (width + 1), width, s2D.x);
    return PlaneSample(real2(sx.value(), sy.value()), sx.density() * sy.density());
}

Discrete2DSample sampleDiscreteDistribution2D(const real* pBuffer, size_t width, size_t height, const real2& s2D) {
    auto sy = sampleDiscreteDistribution1D(pBuffer, height, s2D.y);
    auto idx = sy.value();

    auto sx = sampleDiscreteDistribution1D(pBuffer + height + 1 + idx * (width + 1), width, s2D.x);
    return Discrete2DSample(size2(sx.value(), sy.value()), sx.density() * sy.density());
}

real pdfContinuousDistribution2D(const real* pBuffer, size_t width, size_t height, const real2& point) {
    assert(height > 0);
    auto idx = clamp(size_t(point.y), size_t{ 0 }, height - 1);
    return pdfContinuousDistribution1D(pBuffer + height + 1 + idx * (width + 1), width, point.x) *
            pdfContinuousDistribution1D(pBuffer, height, point.y);
}

real pdfDiscreteDistribution2D(const real* pBuffer, size_t width, size_t height, const uint2& pixel) {
    return pdfDiscreteDistribution1D(pBuffer + height + 1 + pixel.y * (width + 1), pixel.x) *
            pdfDiscreteDistribution1D(pBuffer, pixel.y);
}

}
