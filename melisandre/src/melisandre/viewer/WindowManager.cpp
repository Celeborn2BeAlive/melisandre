#include <melisandre/viewer/WindowManager.hpp>
#include <melisandre/system/logging.hpp>

#include <SDL/SDL.h>

#include <cassert>

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imgui_impl_sdl.h"

namespace mls
{
    using SDLWindowRAII = std::unique_ptr<SDL_Window, void(*)(SDL_Window*)>;

    struct WindowManager::Implementation {
        struct GUIStateRAII {
            ImGuiState m_State;

            ~GUIStateRAII() {
                ImGui_ImplSdl_Shutdown(&m_State);
            }
        };

        struct Window {
            SDLWindowRAII m_pWindow;
            Uint32 m_nWindowID;
            GUIStateRAII m_GUIState;

            explicit Window(SDL_Window* pSDLWin) :
                m_pWindow { pSDLWin, SDL_DestroyWindow },
                m_nWindowID{ SDL_GetWindowID(pSDLWin) } {
            }

            ImGuiState* getGUIState() {
                return reinterpret_cast<ImGuiState*>(&m_GUIState);
            }
        };

        struct SDLRAII {
            SDLRAII() {
                if (SDL_Init(SDL_INIT_VIDEO) < 0) {
                    throw std::runtime_error("SDL_Init failed: " + std::string(SDL_GetError()));
                }
            }
            ~SDLRAII() {
                SDL_Quit();
            }
            SDLRAII(const SDLRAII&) = delete;
            SDLRAII(SDLRAII&&) = delete;
            SDLRAII& operator =(const SDLRAII&) = delete;
            SDLRAII& operator =(SDLRAII&&) = delete;
        };

        SDLRAII m_SDLHandle;

        // Implementation detail: contains pointers to SDL2 windows, store void* to avoid including SDL2 in hpp
        std::vector<Window> m_SDLWindows;
        void* m_pSDLGLContext = nullptr; // One context to rule them all (the windows)
        SDL_Window* m_pCurrentSDLWindow = nullptr;

        SDL_Event m_LastPolledEvent;

        WindowManager::WindowID m_FocusedWindow = 0;

        bool pollEvent(WindowManager::WindowID& targetedWindow) {
            while (SDL_PollEvent(&m_LastPolledEvent)) {
                if (m_LastPolledEvent.type == SDL_WINDOWEVENT) {
                    for (auto i = size_t(0); i < m_SDLWindows.size(); ++i) {
                        const auto& window = m_SDLWindows[i];
                        if (window.m_nWindowID == m_LastPolledEvent.window.windowID) {
                            targetedWindow = i;
                            break;
                        }
                    }

                    if (m_LastPolledEvent.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
                        m_FocusedWindow = targetedWindow;
                    }
                }

                auto guiState = m_SDLWindows[m_FocusedWindow].getGUIState();
                ImGui_ImplSdl_ProcessEvent(guiState, &m_LastPolledEvent);
                if (guiState->IO.WantCaptureMouse || guiState->IO.WantCaptureKeyboard) {
                    continue;
                }

                return true;
            }
            return false;
        }

        SDL_Window* getSDLWindow(WindowManager::WindowID id) {
            assert(id < m_SDLWindows.size());
            return m_SDLWindows[id].m_pWindow.get();
        }
    };

	WindowManager::WindowManager(int glMajorVersion, int glMinorVersion):
        m_pImpl { std::make_unique<Implementation>() }
	{
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, glMajorVersion);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, glMinorVersion);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	}

    WindowManager::~WindowManager()
    {
        // Empty destructor, to allow the call to destructor of WindowManagerPrivateData
        // inside this .cpp
    }
	
    WindowManager::WindowID WindowManager::createWindow(const char* title, size_t width, size_t height)
	{
        return createWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height);
	}
	
    WindowManager::WindowID WindowManager::createWindow(const char* title, int x, int y, size_t width, size_t height)
	{
        auto pWindow = SDL_CreateWindow(title, x, y, int(width), int(height), SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
        if (pWindow == nullptr) {
            throw std::runtime_error("SDL_CreateWindow failed: " + std::string(SDL_GetError()));
        }
        if (!m_pImpl->m_pSDLGLContext) {
            m_pImpl->m_pSDLGLContext = SDL_GL_CreateContext(pWindow);
            if (SDL_GL_SetSwapInterval(1) < 0)
            {
                printf("Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());
            }
        }

        m_pImpl->m_SDLWindows.emplace_back(pWindow);
        ImGui_ImplSdl_Init(m_pImpl->m_SDLWindows.back().getGUIState(), pWindow);

        return m_pImpl->m_SDLWindows.size() - 1;
	}
	
	void WindowManager::setCurrentWindow(WindowID windowID)
	{
        assert(m_pImpl);
        auto pWindow = m_pImpl->getSDLWindow(windowID);
        if (SDL_GL_MakeCurrent(pWindow, m_pImpl->m_pSDLGLContext) < 0) {
            throw std::runtime_error("SDL_GL_MakeCurrent failed: " + std::string(SDL_GetError()));
        }
        m_pImpl->m_pCurrentSDLWindow = pWindow;

        ImGui_ImplSdl_NewFrame(m_pImpl->m_SDLWindows[windowID].getGUIState(), m_pImpl->m_pCurrentSDLWindow);
	}
	
	void WindowManager::swapCurrentWindow()
	{
        SDL_GL_SwapWindow(m_pImpl->m_pCurrentSDLWindow);
	}

    WindowManager::WindowID WindowManager::getFocusedWindow() const {
        return m_pImpl->m_FocusedWindow;
    }

    bool WindowManager::guiHasKeyboardFocus() const {
        auto state = m_pImpl->m_SDLWindows[m_pImpl->m_FocusedWindow].getGUIState();
        return state->IO.WantCaptureKeyboard;
    }

    bool WindowManager::guiHasMouseFocus() const {
        auto state = m_pImpl->m_SDLWindows[m_pImpl->m_FocusedWindow].getGUIState();
        return state->IO.WantCaptureMouse;
    }

    bool WindowManager::guiHasFocus() const {
        return guiHasKeyboardFocus() || guiHasMouseFocus();
    }

    void WindowManager::handleEvents()
    {
        WindowID targetedWindow;
        while (m_pImpl->pollEvent(targetedWindow)) {
            auto* event = &m_pImpl->m_LastPolledEvent;
            
            if (event->type == SDL_WINDOWEVENT) {
                switch (event->window.event) {
                    default:
                        SDL_Log("Window %d got unknown event %d",
                            event->window.windowID, event->window.event);
                        break;
                    case SDL_WINDOWEVENT_CLOSE:
                        SDL_Log("Window %d closed", event->window.windowID);
                        m_WindowClosedEventDispatcher.dispatch(targetedWindow);
                        break;
                    case SDL_WINDOWEVENT_SHOWN:
                        SDL_Log("Window %d shown", event->window.windowID);
                        break;
                    case SDL_WINDOWEVENT_HIDDEN:
                        SDL_Log("Window %d hidden", event->window.windowID);
                        break;
                    case SDL_WINDOWEVENT_EXPOSED:
                        SDL_Log("Window %d exposed", event->window.windowID);
                        break;
                    case SDL_WINDOWEVENT_MOVED:
                        SDL_Log("Window %d moved to %d,%d",
                            event->window.windowID, event->window.data1,
                            event->window.data2);
                        break;
                    case SDL_WINDOWEVENT_RESIZED:
                        SDL_Log("Window %d resized to %dx%d",
                            event->window.windowID, event->window.data1,
                            event->window.data2);
                        break;
                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                        SDL_Log("Window %d size changed to %dx%d",
                            event->window.windowID, event->window.data1,
                            event->window.data2);
                        break;
                    case SDL_WINDOWEVENT_MINIMIZED:
                        SDL_Log("Window %d minimized", event->window.windowID);
                        break;
                    case SDL_WINDOWEVENT_MAXIMIZED:
                        SDL_Log("Window %d maximized", event->window.windowID);
                        break;
                    case SDL_WINDOWEVENT_RESTORED:
                        SDL_Log("Window %d restored", event->window.windowID);
                        break;
                    case SDL_WINDOWEVENT_ENTER:
                        SDL_Log("Mouse entered window %d",
                            event->window.windowID);
                        break;
                    case SDL_WINDOWEVENT_LEAVE:
                        SDL_Log("Mouse left window %d", event->window.windowID);
                        break;
                    case SDL_WINDOWEVENT_FOCUS_GAINED:
                        SDL_Log("Window %d gained keyboard focus",
                            event->window.windowID);
                        break;
                    case SDL_WINDOWEVENT_FOCUS_LOST:
                        SDL_Log("Window %d lost keyboard focus",
                            event->window.windowID);
                        break;
                }
            }
            else {
                switch (event->type) {
                    default:
                        break;
                    case SDL_MOUSEBUTTONDOWN:
                    {
                        static int mapping[] = {
                            0,
                            MOUSE_BUTTON_LEFT,
                            MOUSE_BUTTON_MIDDLE,
                            MOUSE_BUTTON_RIGHT
                        };
                        m_MouseButtonPressedEventDispatcher.dispatch(mapping[event->button.button]);
                    }
                    break;
                }
            }
        }
    }

    bool WindowManager::isKeyPressed(const char* name) const {
        auto keyCode = SDL_GetKeyFromName(name);
        auto scanCode = SDL_GetScancodeFromKey(keyCode);
        auto state = SDL_GetKeyboardState(nullptr);
        assert(scanCode < SDL_NUM_SCANCODES);
        return state[scanCode];
    }

    bool WindowManager::isMouseButtonPressed(MouseButton button) const {
        static int mapping[] = {
            SDL_BUTTON_LEFT,
            SDL_BUTTON_MIDDLE,
            SDL_BUTTON_RIGHT
        };

        return SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON(mapping[button]);
    }

    int2 WindowManager::getMousePosition() const {
        int x, y;
        SDL_GetMouseState(&x, &y);
        return int2(x, y);
    }

    size2 WindowManager::getWindowSize(WindowID windowID) const {
        int w, h;
        SDL_GetWindowSize(m_pImpl->getSDLWindow(windowID), &w, &h);
        return size2(w, h);
    }
}
