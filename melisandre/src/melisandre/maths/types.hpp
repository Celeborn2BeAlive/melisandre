#pragma once

#include "glm.hpp"

namespace mls {

    using real = float;

    template<typename T>
    using tvec1 = glm::tvec1<T>;
    template<typename T>
    using tvec2 = glm::tvec2<T>;
    template<typename T>
    using tvec3 = glm::tvec3<T>;
    template<typename T>
    using tvec4 = glm::tvec4<T>;

    using bool2 = glm::bvec2;
    using bool3 = glm::bvec3;
    using bool4 = glm::bvec4;

    using uint2 = glm::uvec2;
    using uint3 = glm::uvec3;
    using uint4 = glm::uvec4;

    using int2 = glm::ivec2;
    using int3 = glm::ivec3;
    using int4 = glm::ivec4;

    using size2 = tvec2<std::size_t>;
    using size3 = tvec3<std::size_t>;
    using size4 = tvec4<std::size_t>;

    using float2 = glm::vec2;
    using float3 = glm::vec3;
    using float4 = glm::vec4;

    using double2 = glm::dvec2;
    using double3 = glm::dvec3;
    using double4 = glm::dvec4;

    template<typename T>
    using tmat2x2 = glm::tmat2x2<T>;
    template<typename T>
    using tmat3x3 = glm::tmat3x3<T>;
    template<typename T>
    using tmat4x4 = glm::tmat4x4<T>;
    template<typename T>
    using tmat2x3 = glm::tmat2x3<T>;
    template<typename T>
    using tmat3x2 = glm::tmat3x2<T>;
    template<typename T>
    using tmat4x2 = glm::tmat4x2<T>;
    template<typename T>
    using tmat4x3 = glm::tmat4x3<T>;
    template<typename T>
    using tmat2x4 = glm::tmat2x4<T>;
    template<typename T>
    using tmat3x4 = glm::tmat3x4<T>;

    using float2x2 = glm::mat2;
    using float3x3 = glm::mat3;
    using float4x4 = glm::mat4;
    using float2x3 = glm::mat2x3;
    using float3x2 = glm::mat3x2;
    using float2x4 = glm::mat2x4;
    using float4x2 = glm::mat4x2;
    using float3x4 = glm::mat3x4;
    using float4x3 = glm::mat4x3;

    using double2x2 = glm::dmat2;
    using double3x3 = glm::dmat3;
    using double4x4 = glm::dmat4;
    using double2x3 = glm::dmat2x3;
    using double3x2 = glm::dmat3x2;
    using double2x4 = glm::dmat2x4;
    using double4x2 = glm::dmat4x2;
    using double3x4 = glm::dmat3x4;
    using double4x3 = glm::dmat4x3;

    template<typename T>
    struct VecTraits {
        static const auto Dimension = size_t{ 1 };
        using Scalar = T;

        using vec1 = tvec1<T>;
        using vec2 = tvec2<T>;
        using vec3 = tvec3<T>;
        using vec4 = tvec4<T>;

        template<size_t dim>
        struct vec;

        template<>
        struct vec<1> {
            using type = tvec1<T>;
        };

        template<>
        struct vec<2> {
            using type = tvec2<T>;
        };

        template<>
        struct vec<3> {
            using type = tvec3<T>;
        };

        template<>
        struct vec<4> {
            using type = tvec4<T>;
        };
    };

    template<typename T>
    struct VecTraits<tvec1<T>> {
        static const auto Dimension = size_t{ 1 };
        using Scalar = T;
    };

    template<typename T>
    struct VecTraits<tvec2<T>> {
        static const auto Dimension = size_t{ 2 };
        using Scalar = T;
    };

    template<typename T>
    struct VecTraits<tvec3<T>> {
        static const auto Dimension = size_t{ 3 };
        using Scalar = T;
    };

    template<typename T>
    struct VecTraits<tvec4<T>> {
        static const auto Dimension = size_t{ 4 };
        using Scalar = T;
    };
}
