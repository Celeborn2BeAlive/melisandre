#pragma once

#include <algorithm>
#include "index.hpp"
#include "range.hpp"

namespace mls {

template<typename Functor>
inline void repeat(size_t count, Functor f) {
    while(count--) {
        f();
    }
}

template<typename InputContainer, typename OutputContainer, typename UnaryOperator>
inline void transform(InputContainer&& in, OutputContainer&& out, UnaryOperator&& op) {
    std::transform(begin(in), end(in), begin(out), op);
}

template<typename Container, typename Generator>
inline void emplace_n(Container&& c, size_t count, Generator&& generator) {
    c.reserve(c.size() + count);
    while(count--) {
        c.emplace_back(generator());
    }
}

template<typename Container, typename Generator>
inline void emplace_n_indexed(Container&& c, size_t count, Generator&& generator) {
    c.reserve(c.size() + count);
    for(auto i = 0u; i < count; ++i) {
        c.emplace_back(generator(i));
    }
}

}
