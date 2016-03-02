#pragma once

namespace mls {

template<typename DestructorType>
class RAIIUniqueDestructor {
    DestructorType m_Destructor;
public:
    template<typename DestructorT>
    RAIIUniqueDestructor(DestructorT&& d) :
        m_Destructor{ std::forward<DestructorT>(d) } {
    }

    ~RAIIUniqueDestructor() {
        m_Destructor();
    }

    RAIIUniqueDestructor(const RAIIUniqueDestructor&) = delete;
    RAIIUniqueDestructor& operator =(const RAIIUniqueDestructor&) = delete;

    RAIIUniqueDestructor(RAIIUniqueDestructor&&) = default;
    RAIIUniqueDestructor& operator =(RAIIUniqueDestructor&&) = default;
};

template<typename DestructorType>
auto RAIIGuard(DestructorType&& d) -> RAIIUniqueDestructor<DestructorType> {
    return{ std::forward<DestructorType>(d) };
}

}