#include <gtest/gtest.h>
#include <melisandre/system/logging.hpp>

int main(int argc, char** argv) {
    mls::initLogging(argc, argv);
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
