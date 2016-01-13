#pragma once

#include "types.hpp"

namespace mls{

    // A n-dimensional point
    template<size_t dimension, typename T>
    class TPoint : VecTraits<T>::template vec<dimension>::type {
    public:
    };

    using Point2f = TPoint<2, float>;
    using Point3f = TPoint<2, float>;
    using Point2d = TPoint<2, float>;
    using Point3d = TPoint<2, float>;
    using Point2 = TPoint<2, real>;
    using Point3 = TPoint<3, real>;

    // A n-dimensional vector
    template<size_t dimension, typename T>
    class TVector : VecTraits<T>::template vec<dimension>::type {
    public:
    };

    using Vector2f = TVector<2, float>;
    using Vector3f = TVector<2, float>;
    using Vector2d = TVector<2, float>;
    using Vector3d = TVector<2, float>;
    using Vector2 = TVector<2, real>;
    using Vector3 = TVector<3, real>;

    // A n-dimensional direction, i.e. unit vector
    template<size_t dimension, typename T>
    class TDirection : VecTraits<T>::template vec<dimension>::type {
    public:
    };

    using Direction2f = TDirection<2, float>;
    using Direction3f = TDirection<2, float>;
    using Direction2d = TDirection<2, float>;
    using Direction3d = TDirection<2, float>;
    using Direction2 = TDirection<2, real>;
    using Direction3 = TDirection<3, real>;
}