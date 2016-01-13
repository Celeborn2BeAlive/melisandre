#pragma once

#include <vector>
#include <melisandre/maths/types.hpp>

namespace mls {

inline std::vector<float2> makeUVGridTest(size_t width, size_t height) {
    std::vector<float2> uvGrid;
    uvGrid.reserve(width * height);

    float2 delta(1.f / width, 1.f / height);

    for(auto j = size_t(0); j < height; ++j) {
        for(auto i = size_t(0); i < width; ++i) {
            uvGrid.emplace_back(float2(i + 0.5f, j + 0.5f) * delta);
        }
    }

    return uvGrid;
}

}
