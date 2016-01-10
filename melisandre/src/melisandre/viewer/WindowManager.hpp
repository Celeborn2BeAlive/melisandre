#pragma  once

#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>

namespace mls {
    
    class WindowManager {
    public:
        enum Event;

        using WindowID = uint64_t;

        using WindowCloseEventCallback = std::function<void(WindowID)>;

        explicit WindowManager(int glMajorVersion, int glMinorVersion);

        ~WindowManager();

        WindowID createWindow(const char* title, size_t width, size_t height);

        WindowID createWindow(const char* title, int x, int y, size_t width, size_t height);

        void setCurrentWindow(WindowID windowID);

        void swapCurrentWindow();

        Event pollEvent();

        enum Event {
            EVENT_NONE = 0,
            EVENT_WINDOW_CLOSE
        };

        // Only call this if pollEvent() returned WINDOW_CLOSE
        WindowID getClosedWindow() const;

    private:
        struct Implementation;
        std::unique_ptr<Implementation> m_pImpl;
    };
}