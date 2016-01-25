#include <gtest/gtest.h>

#include <melisandre/maths/sampling/shapes.hpp>
#include "../../utils.hpp"

namespace mls {

static const size_t SAMPLING_GRID_WIDTH = 4096;
static const size_t SAMPLING_GRID_HEIGHT = 2048;

TEST(SphericalTriangleSamplingTest, UnitLengthTest) {
    auto A = real3(1, 0, 0), B = real3(0, 1, 0), C = normalize(real3(0, -1, 1));

    for(const auto& uv: makeUVGridTest(SAMPLING_GRID_WIDTH, SAMPLING_GRID_HEIGHT)) {
        auto dirSample = uniformSampleSphericalTriangle(uv.x, uv.y, A, B, C);
        ASSERT_FLOAT_EQ(length(dirSample.value()), 1);
    }
}

}
