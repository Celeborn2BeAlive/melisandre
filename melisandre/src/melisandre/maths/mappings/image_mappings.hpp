#pragma once

#include <melisandre/maths/numeric.hpp>
#include "image_mappings.hpp"

namespace mls {

inline uint32_t getPixelIndex(uint32_t x, uint32_t y, const uint2& imageSize) {
    return x + y * imageSize.x;
}

inline uint32_t getPixelIndex(const uint2& pixel, const uint2& imageSize) {
    return getPixelIndex(pixel.x, pixel.y, imageSize);
}

inline uint2 getPixel(uint32_t index, const uint2& imageSize) {
    return uint2(index % imageSize.x, index / imageSize.x);
}

inline uint2 getPixel(const float2& uv, const uint2& imageSize) {
    return clamp(uint2(uv * float2(imageSize)),
                 uint2(0),
                 uint2(imageSize.x - 1, imageSize.y - 1));
}

inline uint2 rasterPositionToPixel(const float2& rasterPosition,
                                   const uint2& imageSize) {
    return clamp(uint2(rasterPosition),
                 uint2(0),
                 uint2(imageSize.x - 1, imageSize.y - 1));
}

inline float2 getUV(const float2& rasterPosition,
                   const float2& imageSize) {
    return rasterPosition / imageSize;
}

inline float2 getUV(const float2& rasterPosition,
                   const uint2& imageSize) {
    return getUV(rasterPosition, float2(imageSize));
}

inline float2 getUV(uint32_t i, uint32_t j,
                   const uint2& imageSize) {
    return getUV(float2(i, j) + float2(0.5f),
                 imageSize);
}

inline float2 getUV(const uint2& pixel,
                   const uint2& imageSize) {
    return getUV(float2(pixel) + float2(0.5f),
                 imageSize);
}

inline float2 ndcToUV(const float2& ndc) {
    return 0.5f * (ndc + float2(1));
}

inline float2 uvToNDC(const float2& uv) {
    return 2.f * uv - float2(1);
}

inline float2 getNDC(const float2 rasterPosition, float2 imageSize) {
    return float2(-1) + 2.f * rasterPosition / imageSize;
}

}
