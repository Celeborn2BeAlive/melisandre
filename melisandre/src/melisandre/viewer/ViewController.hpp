#pragma once

#include <melisandre/maths/types.hpp>
#include <melisandre/maths/numeric.hpp>

namespace mls {

class ViewController {
public:
    void setViewMatrix(const real4x4& viewMatrix) {
        m_ViewMatrix = viewMatrix;
        m_RcpViewMatrix = inverse(viewMatrix);
    }

    const real4x4& getViewMatrix() const {
        return m_ViewMatrix;
    }

    bool moveLocal(real3 localTranslationVector, real3 localRotationAngles);

private:
    real4x4 m_ViewMatrix = real4x4(1);
    real4x4 m_RcpViewMatrix = real4x4(1);
};

}
