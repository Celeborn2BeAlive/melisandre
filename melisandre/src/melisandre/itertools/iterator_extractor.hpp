#pragma once

namespace mls {

template <typename T>
struct iterator_extractor { 
    using type = typename T::iterator; 
};

template <typename T>
struct iterator_extractor<T const> { 
    using type = typename T::const_iterator;
};

}
