#include "maths.hpp"
#include "types.hpp"
#include "geometry.hpp"
#include "constants.hpp"
#include "bbox.hpp"
#include "colors.hpp"
#include "common.hpp"

using namespace mls;

// Compilation tests
static void maths_foo() {
    distanceSquared(real3{}, real3{});
    lengthSquared(real3{});
    absDot(real3{}, real3{});
}