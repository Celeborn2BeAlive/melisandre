#pragma once

#include <memory>
#include <vector>
#include <functional>
#include <algorithm>

#include "RAII.hpp"

namespace mls {

template <typename CallbackType>
class EventDispatcher;

template <typename CallbackType>
class Listener final {
    std::shared_ptr<CallbackType> m_Ptr;

    friend class EventDispatcher<CallbackType>;
public:
    template <typename ...ArgTypes>
    Listener(ArgTypes... args) :
        m_Ptr(std::make_shared<CallbackType>(std::forward<ArgTypes>(args)...)) {
    }

    void disconnect() {
        m_Ptr.reset();
    }
};

/**
* Dispatches function on a number of callbacks and cleans up callbacks when
* they are dead.
* 
* Based on http://nercury.github.io/c++/interesting/2016/02/22/weak_ptr-and-event-cleanup.html
*/
template <typename CallbackType>
class EventDispatcher final {
private:
    std::vector<std::weak_ptr<CallbackType>> m_Callbacks;
    size_t m_nConcurrentDispatcherCount = 0;
public:
    Listener<CallbackType> addListener(CallbackType&& callback) {
        Listener<CallbackType> listener(callback);
        m_Callbacks.push_back(listener.m_Ptr);
        return listener;
    }

    template <typename ...ArgTypes>
    void dispatch(ArgTypes&& ... args) {
        {
            ++m_nConcurrentDispatcherCount;
            auto guard = RAIIGuard([&]() { --m_nConcurrentDispatcherCount; });

            for (auto i = size_t(0); i < m_Callbacks.size(); ++i) {
                auto weakPtr = m_Callbacks[i];
                if (auto callback = weakPtr.lock()) {
                    (*callback)(std::forward<ArgTypes>(args)...);
                }
            }
        }

        // Remove all callbacks that are gone, only if we are not dispatching.
        if (0 == m_nConcurrentDispatcherCount) {
            m_Callbacks.erase(
                std::remove_if(
                    begin(m_Callbacks),
                    end(m_Callbacks),
                    [](const auto& callback) { return callback.expired(); }
                    ),
                end(m_Callbacks)
                );
        }
    }
};

template<typename ReturnType, typename ...ArgsTypes>
class EventDispatcher<ReturnType(ArgsTypes...)> final {
    EventDispatcher<std::function<ReturnType(ArgsTypes...)>> m_Dispatcher;
public:
    template<typename CallbackType>
    auto addListener(CallbackType&& callback) {
        return m_Dispatcher.addListener(callback);
    }

    template <typename ...ArgTypes>
    void dispatch(ArgTypes&& ... args) {
        m_Dispatcher.dispatch(std::forward<ArgTypes>(args)...);
    }
};

}