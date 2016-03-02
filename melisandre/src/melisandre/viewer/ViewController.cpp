#include "ViewController.hpp"

namespace mls {

bool ViewController::moveLocal(real3 localTranslationVector, real3 localRotationAngles) {
    auto worldSpacePosition = real3(m_RcpViewMatrix[3]);

    auto hasChanged = (localTranslationVector != real3(0) || localRotationAngles != real3(0));

    worldSpacePosition += real3(m_RcpViewMatrix * real4(localTranslationVector, 0));

    auto newRcpViewMatrix = m_RcpViewMatrix;

    if (localRotationAngles != real3(0)) {
        newRcpViewMatrix = rotate(newRcpViewMatrix, localRotationAngles.z, real3(0, 0, 1));
        newRcpViewMatrix = rotate(newRcpViewMatrix, localRotationAngles.x, real3(0, 1, 0));
        newRcpViewMatrix = rotate(newRcpViewMatrix, localRotationAngles.y, real3(1, 0, 0));
    }

    auto frontVector = -real3(newRcpViewMatrix[2]);
    auto leftVector = -real3(newRcpViewMatrix[0]);
    auto upVector = cross(frontVector, leftVector);

    if (hasChanged) {
        setViewMatrix(lookAt(worldSpacePosition, worldSpacePosition + frontVector, upVector));
    }

    return hasChanged;
}

}
