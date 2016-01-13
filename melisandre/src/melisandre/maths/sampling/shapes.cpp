#include "shapes.hpp"

namespace mls {

float sphericalTriangleArea(const float3& A, const float3& B, const float3& C) {
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
    return alpha + beta + gamma - pi<float>();
}

Sample3f uniformSampleSphericalTriangle(float e1, float e2, const float3& A, const float3& B, const float3& C) {
    static auto normalizedOrthogonalComponent = [](const float3& x, const float3& y) {
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
    auto area = alpha + beta + gamma - pi<float>();
    if(area == 0.f) {
        return Sample3f(zero<float3>(), 0.f);
    }

    auto newArea = e1 * area; // Sample sub-triangle

    auto s = sin(newArea - alpha);
    auto t = cos(newArea - alpha);

    auto sinAlpha = sin(alpha);
    auto u = t - cosAlpha;
    auto v = s + sinAlpha * cosC;

    float q = ((v * t - u * s) * cosAlpha - v) / ((v * s + u * t) * sinAlpha);

    auto newC = q * A + sqrt(1 - sqr(q)) * normalizedOrthogonalComponent(C, A);

    auto z = 1 - e2 * (1 - dot(newC, B));

    auto P = z * B + sqrt(1 - sqr(z)) * normalizedOrthogonalComponent(newC, B);

    return Sample3f(P, 1.f / area);
}

}
