#include "shapes.hpp"

namespace mls {

real sphericalTriangleArea(const real3& A, const real3& B, const real3& C) {
    auto normalAB = normalize(cross(A, B));
    auto normalBC = normalize(cross(B, C));
    auto normalCA = normalize(cross(C, A));

    auto cosAlpha = -dot(normalAB, normalCA);
    auto alpha = acos(cosAlpha); // angle at A
    auto cosBeta = -dot(normalBC, normalAB);
    auto beta = acos(cosBeta); // angle at B
    auto cosGamma = -dot(normalBC, normalCA);
    auto gamma = acos(cosGamma); // angle at C

    // Spherical triangle area
    return alpha + beta + gamma - pi<real>();
}

direction_sample uniformSampleSphericalTriangle(real e1, real e2, const real3& A, const real3& B, const real3& C) {
    static auto normalizedOrthogonalComponent = [](const real3& x, const real3& y) {
        return normalize(x - dot(x, y) * y);
    };

    // Compute spherical edge length (in radians)
    //auto cosA = dot(B ,C);
    //auto a = acos(cosA); // length between B and C
    //auto cosB = dot(A, C);
    //auto b = acos(cosB); // length between A and C
    auto cosC = dot(A, B);
    //auto c = acos(cosC); // length between A and B

    // Compute dihedral angles
    auto normalAB = normalize(cross(A, B));
    auto normalBC = normalize(cross(B, C));
    auto normalCA = normalize(cross(C, A));

    auto cosAlpha = -dot(normalAB, normalCA);
    auto alpha = acos(cosAlpha); // angle at A
    auto cosBeta = -dot(normalBC, normalAB);
    auto beta = acos(cosBeta); // angle at B
    auto cosGamma = -dot(normalBC, normalCA);
    auto gamma = acos(cosGamma); // angle at C

    // Spherical triangle area
    auto area = alpha + beta + gamma - pi<real>();
    if(area == 0.f) {
        return direction_sample{ zero<real3>(), 0.f };
    }

    auto newArea = e1 * area; // Sample sub-triangle

    auto s = sin(newArea - alpha);
    auto t = cos(newArea - alpha);

    auto sinAlpha = sin(alpha);
    auto u = t - cosAlpha;
    auto v = s + sinAlpha * cosC;

    real q = ((v * t - u * s) * cosAlpha - v) / ((v * s + u * t) * sinAlpha);

    auto newC = q * A + sqrt(1 - sqr(q)) * normalizedOrthogonalComponent(C, A);

    auto z = 1 - e2 * (1 - dot(newC, B));

    auto P = z * B + sqrt(1 - sqr(z)) * normalizedOrthogonalComponent(newC, B);

    return direction_sample{ P, 1.f / area };
}

}
