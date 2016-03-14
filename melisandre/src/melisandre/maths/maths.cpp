#include "maths.hpp"
#include "types.hpp"
#include "geometry.hpp"
#include "constants.hpp"
#include "aabb.hpp"
#include "colors.hpp"

using namespace mls;

// Compilation tests
static void maths_foo() {
    sqr_distance(real3{}, real3{});
    sqr_length(real3{});
    abs_dot(real3{}, real3{});

    aabb<real3> box3d;
    aabb<real2> box2d;
}