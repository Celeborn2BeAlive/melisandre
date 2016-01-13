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

Sample2f sampleContinuousDistribution2D(const float* pBuffer, size_t width, size_t height, const float2& s2D) {
    auto sy = sampleContinuousDistribution1D(pBuffer, height, s2D.y);
    int idx = clamp(int(sy.value), 0, int(height) - 1);

    auto sx = sampleContinuousDistribution1D(pBuffer + height + 1 + idx * (width + 1), width, s2D.x);
    return Sample2f(float2(sx.value, sy.value), sx.pdf * sy.pdf);
}

Sample2u sampleDiscreteDistribution2D(const float* pBuffer, size_t width, size_t height, const float2& s2D) {
    auto sy = sampleDiscreteDistribution1D(pBuffer, height, s2D.y);
    int idx = sy.value;

    auto sx = sampleDiscreteDistribution1D(pBuffer + height + 1 + idx * (width + 1), width, s2D.x);
    return Sample2u(uint2(sx.value, sy.value), sx.pdf * sy.pdf);
}

float pdfContinuousDistribution2D(const float* pBuffer, size_t width, size_t height, const float2& point) {
    int idx = clamp(int(point.y), 0, int(height) - 1);
    return pdfContinuousDistribution1D(pBuffer + height + 1 + idx * (width + 1), width, point.x) *
            pdfContinuousDistribution1D(pBuffer, height, point.y);
}

float pdfDiscreteDistribution2D(const float* pBuffer, size_t width, size_t height, const uint2& pixel) {
    return pdfDiscreteDistribution1D(pBuffer + height + 1 + pixel.y * (width + 1), pixel.x) *
            pdfDiscreteDistribution1D(pBuffer, pixel.y);
}

}
