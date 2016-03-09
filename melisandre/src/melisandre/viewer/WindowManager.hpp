#pragma  once

#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>

#include <melisandre/maths/types.hpp>
#include <melisandre/utils/EventDispatcher.hpp>

namespace mls {
    
    class WindowManager {
    public:
        enum WindowFlag {
            WINDOW_RESIZABLE = 1 << 0,
            WINDOW_FLAG_COUNT
        };

        enum MouseButton {
            MOUSE_BUTTON_LEFT = 0,
            MOUSE_BUTTON_MIDDLE = 1,
            MOUSE_BUTTON_RIGHT = 2,
            MOUSE_BUTTON_COUNT
        };

        using WindowID = uint64_t;

        explicit WindowManager(int glMajorVersion, int glMinorVersion);

        ~WindowManager();

        WindowID createWindow(const char* title, size_t width, size_t height, uint64_t windowFlagBits = 0);

        WindowID createWindow(const char* title, int x, int y, size_t width, size_t height, uint64_t windowFlagBits = 0);

        void setCurrentWindow(WindowID windowID);

        void swapCurrentWindow();

        WindowID getFocusedWindow() const;

        bool guiHasFocus() const;

        bool guiHasKeyboardFocus() const;

        bool guiHasMouseFocus() const;

        bool isKeyPressed(const char* name) const;

        bool isMouseButtonPressed(MouseButton button) const;

        int2 getMousePosition() const;

        size2 getWindowSize(WindowID windowID) const;

        void handleEvents();

        template<typename Callback>
        auto onWindowClosed(Callback&& callback) {
            return m_WindowClosedEventDispatcher.addListener(std::forward<Callback>(callback));
        }

        template<typename Callback>
        auto onMouseButtonPressed(Callback&& callback) {
            return m_MouseButtonPressedEventDispatcher.addListener(std::forward<Callback>(callback));
        }

    private:
        struct Implementation;
        std::unique_ptr<Implementation> m_pImpl;

        EventDispatcher<void(WindowID)> m_WindowClosedEventDispatcher;
        EventDispatcher<void(int)> m_MouseButtonPressedEventDispatcher;
    };
}