#pragma once

#include "types.hpp"
#include "common.hpp"
#include "constants.hpp"
#include "mappings/image_mappings.hpp"
#include "mappings/paraboloid_mappings.hpp"
#include "mappings/dual_paraboloid_mappings.hpp"
#include "mappings/spherical_mappings.hpp"
#include "mappings/hemispherical_mappings.hpp"

namespace mls {

inline float3 getColor(uint32_t n) {
    return fract(
                sin(
                    float(n + 1) * float3(12.9898, 78.233, 56.128)
                    )
                * 43758.5453f
                );
}

/**
 * @brief cartesianToSpherical
 * @param direction a unit length 3D vector
 * @return Spherical coordinates (phi, theta) of direction with phi in [0, 2pi[ and theta in [-pi, pi]
 */
inline float2 cartesianToSpherical(const float3& direction) {
    return float2(atan2(direction.x, direction.z), asin(direction.y));
}

/**
 * @brief sphericalToCartesian
 * @param angles The spherical angles: angles.x is phi and angles.y is theta
 * @return Cartesian coordinates
 */
inline float3 sphericalToCartesian(const float2& angles) {
    auto cosTheta = cos(angles.y);
    return float3(sin(angles.x) * cosTheta,
                     sin(angles.y),
                     cos(angles.x) * cosTheta);
}

inline float3 getOrthogonalUnitVector(const float3& v) {
    if(abs(v.y) > abs(v.x)) {
        float rcpLength = 1.f / length(float2(v.x, v.y));
        return rcpLength * float3(v.y, -v.x, 0.f);
    }
    float rcpLength = 1.f / length(float2(v.x, v.z));
    return rcpLength * float3(v.z, 0.f, -v.x);
}

inline float3x3 frameY(const float3& yAxis) {
    float3x3 matrix;

    matrix[1] = float3(yAxis);
    if(abs(yAxis.y) > abs(yAxis.x)) {
        float rcpLength = 1.f / length(float2(yAxis.x, yAxis.y));
        matrix[0] = rcpLength * float3(yAxis.y, -yAxis.x, 0.f);
    } else {
        float rcpLength = 1.f / length(float2(yAxis.x, yAxis.z));
        matrix[0] = rcpLength * float3(yAxis.z, 0.f, -yAxis.x);
    }
    matrix[2] = cross(matrix[0], yAxis);
    return matrix;
}

// zAxis should be normalized
inline float3x3 frameZ(const float3& zAxis) {
    float3x3 matrix;

    matrix[2] = float3(zAxis);
    if(abs(zAxis.y) > abs(zAxis.x)) {
        float rcpLength = 1.f / length(float2(zAxis.x, zAxis.y));
        matrix[0] = rcpLength * float3(zAxis.y, -zAxis.x, 0.f);
    } else {
        float rcpLength = 1.f / length(float2(zAxis.x, zAxis.z));
        matrix[0] = rcpLength * float3(zAxis.z, 0.f, -zAxis.x);
    }
    matrix[1] = cross(zAxis, matrix[0]);
    return matrix;
}

inline float4x4 frameY(const float3& origin, const float3& yAxis) {
    float4x4 matrix;
    matrix[3] = float4(origin, 1);

    matrix[1] = float4(yAxis, 0);
    if(abs(yAxis.y) > abs(yAxis.x)) {
        float rcpLength = 1.f / length(float2(yAxis.x, yAxis.y));
        matrix[0] = rcpLength * float4(yAxis.y, -yAxis.x, 0.f, 0.f);
    } else {
        float rcpLength = 1.f / length(float2(yAxis.x, yAxis.z));
        matrix[0] = rcpLength * float4(yAxis.z, 0.f, -yAxis.x, 0.f);
    }
    matrix[2] = float4(cross(float3(matrix[0]), yAxis), 0);
    return matrix;
}

inline void faceForward(const float3& wi, float3& N) {
    if(dot(wi, N) < 0.f) {
        N = -N;
    }
}

inline float reduceMax(const float3& v) {
    return max(v.x, max(v.y, v.z));
}

inline bool reduceLogicalOr(const bool3& v) {
    return v.x || v.y || v.z;
}

inline bool reduceLogicalOr(const bool4& v) {
    return v.x || v.y || v.z || v.w;
}

// Return the index of the maximal component of the vector
inline uint32_t maxComponent(const float3& v) {
    auto m = reduceMax(v);
    if(m == v.x) {
        return 0u;
    }
    if(m == v.y) {
        return 1u;
    }
    return 2u;
}

inline bool isInvalidMeasurementEstimate(const float3& c) {
    return c.r < 0.f || c.g < 0.f || c.b < 0.f ||
            reduceLogicalOr(isnan(c)) || reduceLogicalOr(isinf(c));
}

inline bool isInvalidMeasurementEstimate(const float4& c) {
    return isInvalidMeasurementEstimate(float3(c));
}

inline float luminance(const float3& color) {
    return 0.212671f * color.r + 0.715160f * color.g + 0.072169f * color.b;
}

template<typename T> inline T sin2cos ( const T& x )  {
    return sqrt(max(T(0.f), T(1.f) - x * x));
}

template<typename T> inline T cos2sin ( const T& x )  {
    return sin2cos(x);
}

inline float4x4 getViewMatrix(const float3& origin) {
    return glm::translate(glm::mat4(1.f), -origin);
}

// Return a view matrix located at origin and looking toward up direction
inline float4x4 getViewMatrix(const float3& origin, const float3& up) {
    auto frame = frameZ(-up);
    float4x4 m(frame);
    m[3] = float4(origin, 1);
    return inverse(m);
}

inline bool viewportContains(const float2& position, const float4& viewport) {
    return position.x >= viewport.x && position.y >= viewport.y &&
            position.x < viewport.x + viewport.z && position.y < viewport.y + viewport.w;
}

inline bool viewportContains(const uint2& position, const uint4& viewport) {
    return position.x >= viewport.x && position.y >= viewport.y &&
            position.x < viewport.x + viewport.z && position.y < viewport.y + viewport.w;
}

inline float3 refract(const float3& I, const float3& N, float eta) {
    auto dotI = dot(I, N);

    auto sqrDotO = 1 - sqr(eta) * (1 - sqr(dotI));
    if(sqrDotO < 0.f) {
        return zero<float3>();
    }
    auto k = sqrt(sqrDotO);
    return -eta * I + (eta * dotI - sign(dotI) * k) * N;
}

inline float3 reflect(const float3& I, const float3& N) {
    return glm::reflect(-I, N);
}

// Same convention as OpenGL:
enum class CubeFace {
    POS_X = 0,
    NEG_X = 1,
    POS_Y = 2,
    NEG_Y = 3,
    POS_Z = 4,
    NEG_Z = 5,
    FACE_COUNT = 6
};

inline CubeFace getCubeFace(const float3& wi) {
    float3 absWi = abs(wi);

    float maxComponent = max(absWi.x, max(absWi.y, absWi.z));

    if(maxComponent == absWi.x) {
        if(wi.x > 0) {
            return CubeFace::POS_X;
        }
        return CubeFace::NEG_X;
    }

    if(maxComponent == absWi.y) {
        if(wi.y >= 0) {
            return CubeFace::POS_Y;
        }
        return CubeFace::NEG_Y;
    }

    if(maxComponent == absWi.z) {
        if(wi.z > 0) {
            return CubeFace::POS_Z;
        }
    }

    return CubeFace::NEG_Z;
}

}
