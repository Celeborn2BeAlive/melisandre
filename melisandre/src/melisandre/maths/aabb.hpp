#pragma once

#include "maths.hpp"

namespace mls {

template<typename CoordsType>
struct aabb
{
public:
    using coords_type = CoordsType;
    using scalar_type = typename coords_traits<coords_type>::scalar_type;
    static const auto dimension = coords_traits<coords_type>::dimension;
    
    aabb() = default;

    aabb(const coords_type& lower, const coords_type& upper):
        m_Lower(lower),
        m_Upper(upper)
    {}

    aabb(const coords_type& v):
        aabb(v, v)
    {}

    void grow(const aabb& other) 
    { 
        m_Lower = min(m_Lower, other.m_Lower);
        m_Upper = max(m_Upper, other.m_Upper);
    }

    void grow(const coords_type& other)
    {
        m_Lower = min(m_Lower, other);
        m_Upper = max(m_Upper, other);
    }

    bool empty() const
    {
        for (auto i = 0u; i < dimension; i++) {
            if (m_Lower[i] > m_Upper[i]) {
                return true;
            }
        }
        return false;
    }

    const coords_type& lower() const
    {
        return m_Lower;
    }

    const coords_type& upper() const
    {
        return m_Upper;
    }

private:
    coords_type m_Lower = coords_type(std::numeric_limits<scalar_type>::max());
    coords_type m_Upper = coords_type(std::numeric_limits<scalar_type>::lowest());
};

template<typename T>
inline bool empty(const aabb<T>& box)
{
    return box.empty();
}

/*! computes the center of the box */
template<typename T>
inline const T center (const aabb<T>& box)
{
    return 0.5 * (box.lower() + box.upper());
}
template<typename T> inline const T center2(const aabb<T>& box) { return box.lower() + box.upper(); }

/*! computes the size of the box */
template<typename T> inline const T size(const aabb<T>& box) { return box.size(); }

///*! computes the volume of a bounding box */
//template<typename T> inline const T volume( const BBox<T>& b ) { return reduce_mul(size(b)); }

///*! computes the surface area of a bounding box */
//template<typename T> inline const T     area( const BBox<Vec2<T> >& b ) { const auto d = size(b); return d.x*d.y; }
//template<typename T> inline const T     area( const BBox<Vec3<T> >& b ) { const auto d = size(b); return T(2)*(d.x*(d.y+d.z)+d.y*d.z); }
//template<typename T> inline const T halfArea( const BBox<Vec3<T> >& b ) { const auto d = size(b); return d.x*(d.y+d.z)+d.y*d.z; }

/*! merges bounding boxes and points */
template<typename T> inline const aabb<T> merge( const aabb<T>& a, const       T& b ) { return aabb<T>(min(a.lower(), b    ), max(a.upper(), b    )); }
template<typename T> inline const aabb<T> merge( const       T& a, const aabb<T>& b ) { return aabb<T>(min(a    , b.lower()), max(a    , b.upper())); }
template<typename T> inline const aabb<T> merge( const aabb<T>& a, const aabb<T>& b ) { return aabb<T>(min(a.lower(), b.lower()), max(a.upper(), b.upper())); }
template<typename T> inline const aabb<T> merge( const aabb<T>& a, const aabb<T>& b, const aabb<T>& c ) { return merge(a,merge(b,c)); }
template<typename T> inline const aabb<T>& operator+=( aabb<T>& a, const aabb<T>& b ) { return a = merge(a,b); }
template<typename T> inline const aabb<T>& operator+=( aabb<T>& a, const       T& b ) { return a = merge(a,b); }

/*! Merges four boxes. */
template<typename T> inline aabb<T> merge(const aabb<T>& a, const aabb<T>& b, const aabb<T>& c, const aabb<T>& d) {
  return merge(merge(a,b),merge(c,d));
}

/*! Merges eight boxes. */
template<typename T> inline aabb<T> merge(const aabb<T>& a, const aabb<T>& b, const aabb<T>& c, const aabb<T>& d,
                                                 const aabb<T>& e, const aabb<T>& f, const aabb<T>& g, const aabb<T>& h) {
  return merge(merge(a,b,c,d),merge(e,f,g,h));
}

/*! Comparison Operators */
template<typename T> inline bool operator==( const aabb<T>& a, const aabb<T>& b ) { return a.lower() == b.lower() && a.upper() == b.upper(); }
template<typename T> inline bool operator!=( const aabb<T>& a, const aabb<T>& b ) { return a.lower() != b.lower() || a.upper() != b.upper(); }

/*! scaling */
template<typename T> inline aabb<T> operator *( const float& a, const aabb<T>& b ) { return aabb<T>(a*b.lower(),a*b.upper()); }

/*! intersect bounding boxes */
template<typename T> inline const aabb<T> intersect( const aabb<T>& a, const aabb<T>& b ) { return aabb<T>(max(a.lower(), b.lower()), min(a.upper(), b.upper())); }
template<typename T> inline const aabb<T> intersect( const aabb<T>& a, const aabb<T>& b, const aabb<T>& c ) { return intersect(a,intersect(b,c)); }

/*! tests if bounding boxes (and points) are disjoint (empty intersection) */
template<typename T> inline bool disjoint( const aabb<T>& a, const aabb<T>& b )
{ const T d = min(a.upper(), b.upper()) - max(a.lower(), b.lower()); for ( size_t i = 0 ; i < aabb<T>::dim ; i++ ) if ( d[i] < typename coords_traits<T>::scalar_type(0) ) return true; return false; }
template<typename T> inline bool disjoint( const aabb<T>& a, const  T& b )
{ const T d = min(a.upper(), b)       - max(a.lower(), b);       for ( size_t i = 0 ; i < aabb<T>::dim ; i++ ) if ( d[i] < typename coords_traits<T>::scalar_type(0) ) return true; return false; }
template<typename T> inline bool disjoint( const  T& a, const aabb<T>& b )
{ const T d = min(a, b.upper())       - max(a, b.lower());       for ( size_t i = 0 ; i < aabb<T>::dim ; i++ ) if ( d[i] < typename coords_traits<T>::scalar_type(0) ) return true; return false; }

/*! tests if bounding boxes (and points) are conjoint (non-empty intersection) */
template<typename T> inline bool conjoint( const aabb<T>& a, const aabb<T>& b )
{ const T d = min(a.upper(), b.upper()) - max(a.lower(), b.lower()); for ( size_t i = 0 ; i < aabb<T>::dim ; i++ ) if ( d[i] < typename coords_traits<T>::scalar_type(0) ) return false; return true; }
template<typename T> inline bool conjoint( const aabb<T>& a, const  T& b )
{ const T d = min(a.upper(), b)       - max(a.lower(), b);       for ( size_t i = 0 ; i < aabb<T>::dim ; i++ ) if ( d[i] < typename coords_traits<T>::scalar_type(0) ) return false; return true; }
template<typename T> inline bool conjoint( const  T& a, const aabb<T>& b )
{ const T d = min(a, b.upper())       - max(a, b.lower());       for ( size_t i = 0 ; i < aabb<T>::dim ; i++ ) if ( d[i] < typename coords_traits<T>::scalar_type(0) ) return false; return true; }

/*! subset relation */
template<typename T> inline bool subset( const aabb<T>& a, const aabb<T>& b )
{
  for ( size_t i = 0 ; i < aabb<T>::dim ; i++ ) if ( a.lower()[i]*1.00001f < b.lower()[i] ) return false;
  for ( size_t i = 0 ; i < aabb<T>::dim ; i++ ) if ( a.upper()[i] > b.upper()[i]*1.00001f ) return false;
  return true;
}

/*! output operator */
template<typename T> inline std::ostream& operator<<(std::ostream& cout, const aabb<T>& box) {
  return cout << "[" << box.lower() << "; " << box.upper() << "]";
}

template<typename T>
inline void bounding_sphere(const aabb<T>& bbox, T& c,
                           typename coords_traits<T>::scalar_type& radius) {
    c = center(bbox);
    radius = length(size(bbox)) * 0.5f;
}

/*! default template instantiations */
typedef aabb<float2> BBox2f;
typedef aabb<float3> BBox3f;

typedef aabb<int2> BBox2i;
typedef aabb<int3> BBox3i;

}
