#pragma once

#include <GL/glew.h>

namespace mls {

    // Must be called after obtaining an OpenGL context (through the opening of a window)
    // and before issuing any GL command.
    bool initOpenGL();

}