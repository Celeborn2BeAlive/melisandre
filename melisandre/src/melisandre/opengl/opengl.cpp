#include <melisandre/opengl/opengl.hpp>

namespace mls {

    bool initOpenGL() {
        glewExperimental = GL_TRUE;
        auto glewResult = glewInit();
        if (glewResult == GLEW_OK) {
            return true;
        }
        // #todo Must log glewGetErrorString(glewResult);
        return false;
    }

}