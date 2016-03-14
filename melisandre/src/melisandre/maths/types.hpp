#pragma once

#include "glm.hpp"

namespace mls {

    using real = float;

    template<typename T>
    using tcoords1 = glm::tvec1<T>;
    template<typename T>
    using tcoords2 = glm::tvec2<T>;
    template<typename T>
    using tcoords3 = glm::tvec3<T>;
    template<typename T>
    using tcoords4 = glm::tvec4<T>;

    using bool2 = glm::bvec2;
    using bool3 = glm::bvec3;
    using bool4 = glm::bvec4;

    using uint2 = glm::uvec2;
    using uint3 = glm::uvec3;
    using uint4 = glm::uvec4;

    using int2 = glm::ivec2;
    using int3 = glm::ivec3;
    using int4 = glm::ivec4;

    using size2 = tcoords2<std::size_t>;
    using size3 = tcoords3<std::size_t>;
    using size4 = tcoords4<std::size_t>;

    using float2 = glm::vec2;
    using float3 = glm::vec3;
    using float4 = glm::vec4;

    using double2 = glm::dvec2;
    using double3 = glm::dvec3;
    using double4 = glm::dvec4;

    using real2 = tcoords2<real>;
    using real3 = tcoords3<real>;
    using real4 = tcoords4<real>;

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

    using real2x2 = tmat2x2<real>;
    using real3x3 = tmat3x3<real>;
    using real4x4 = tmat4x4<real>;
    using real2x3 = tmat2x3<real>;
    using real3x2 = tmat3x2<real>;
    using real2x4 = tmat2x4<real>;
    using real4x2 = tmat4x2<real>;
    using real3x4 = tmat3x4<real>;
    using real4x3 = tmat4x3<real>;

    template<typename T>
    struct coords_traits {
        static const auto dimension = size_t{ 1 };
        using scalar_type = T;

        using coords1 = tcoords1<T>;
        using coords2 = tcoords2<T>;
        using coords3 = tcoords3<T>;
        using coords4 = tcoords4<T>;

        template<size_t dim>
        struct coords;

        template<>
        struct coords<1> {
            using type = tcoords1<T>;
        };

        template<>
        struct coords<2> {
            using type = tcoords2<T>;
        };

        template<>
        struct coords<3> {
            using type = tcoords3<T>;
        };

        template<>
        struct coords<4> {
            using type = tcoords4<T>;
        };
    };

    template<typename T>
    struct coords_traits<tcoords1<T>> {
        static const auto dimension = size_t{ 1 };
        using scalar_type = T;
    };

    template<typename T>
    struct coords_traits<tcoords2<T>> {
        static const auto dimension = size_t{ 2 };
        using scalar_type = T;
    };

    template<typename T>
    struct coords_traits<tcoords3<T>> {
        static const auto dimension = size_t{ 3 };
        using scalar_type = T;
    };

    template<typename T>
    struct coords_traits<tcoords4<T>> {
        static const auto dimension = size_t{ 4 };
        using scalar_type = T;
    };

    using glm::value_ptr;
}
