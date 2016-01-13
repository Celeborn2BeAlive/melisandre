#include <gtest/gtest.h>

#include <melisandre/maths/mappings/spherical_mappings.hpp>
#include <vector>

#include "../../utils.hpp"

namespace mls {

static const size_t SAMPLING_GRID_WIDTH = 4096;
static const size_t SAMPLING_GRID_HEIGHT = 2048;

TEST(SphericalMappingTest, UnitLengthTest) {
    for(const auto& uv: makeUVGridTest(SAMPLING_GRID_WIDTH, SAMPLING_GRID_HEIGHT)) {
        auto wi = sphericalMapping(uv);
        ASSERT_FLOAT_EQ(length(wi), 1.f);
    }
}

TEST(SphericalMappingTest, ReciprocityTest) {
    for(const auto& uv: makeUVGridTest(SAMPLING_GRID_WIDTH, SAMPLING_GRID_HEIGHT)) {
        float sinTheta1, sinTheta2;
        auto wi = sphericalMapping(uv, sinTheta1);
        auto uvTest = rcpSphericalMapping(wi, sinTheta2);
        ASSERT_FLOAT_EQ(uv.x, uvTest.x);
        ASSERT_FLOAT_EQ(uv.y, uvTest.y);
        ASSERT_FLOAT_EQ(sinTheta1, sinTheta2);
    }
}

TEST(SphericalMappingTest, JacobianTest) {
    for(const auto& uv: makeUVGridTest(SAMPLING_GRID_WIDTH, SAMPLING_GRID_HEIGHT)) {
        auto jacobian = sphericalMappingJacobian(uv);
        auto wi = sphericalMapping(uv);
        auto rcpJacobian = rcpSphericalMappingJacobian(wi);

        if(jacobian == 0.f) {
            ASSERT_FLOAT_EQ(rcpJacobian, 0.f);
        } else {
            ASSERT_FLOAT_EQ(1.f / jacobian, rcpJacobian);
        }
    }
}

TEST(SphericalMappingTest, IntegrationTest) {
    float jacobianSum = 0.f;
    // Integration of the jacobian over the grid, should be equal to the area of the unit sphere: 4.pi
    for(const auto& uv: makeUVGridTest(SAMPLING_GRID_WIDTH, SAMPLING_GRID_HEIGHT)) {
        auto jacobian = sphericalMappingJacobian(uv);
        jacobianSum += jacobian;
    }
    jacobianSum /= SAMPLING_GRID_WIDTH * SAMPLING_GRID_HEIGHT;
    ASSERT_NEAR(jacobianSum, four_pi<float>(), 0.1f);
}

}
