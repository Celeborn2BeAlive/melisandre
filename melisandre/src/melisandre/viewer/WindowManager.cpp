#include <melisandre/viewer/WindowManager.hpp>

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
        WindowManager::WindowID m_nLastEventWindow = std::numeric_limits<WindowManager::WindowID>::max();

        void updateLastEventWindow() {
            for (auto i = size_t(0); i < m_SDLWindows.size(); ++i) {
                const auto& window = m_SDLWindows[i];
                if (window.m_nWindowID == m_LastPolledEvent.window.windowID) {
                    m_nLastEventWindow = i;
                    break;
                }
            }
        }

        SDL_Window* getSDLWindow(WindowManager::WindowID id) {
            assert(id < m_SDLWindows.size());
            return m_SDLWindows[id].m_pWindow.get();
        }

        ImGuiState* getLastSDLWindowEventGUIState() {
            return m_SDLWindows[m_nLastEventWindow].getGUIState();
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

    WindowManager::Event WindowManager::pollEvent()
    {
        if (!SDL_PollEvent(&m_pImpl->m_LastPolledEvent)) {
            return EVENT_NONE;
        }

        if (m_pImpl->m_LastPolledEvent.type == SDL_WINDOWEVENT) {
            m_pImpl->updateLastEventWindow();
            switch (m_pImpl->m_LastPolledEvent.window.event)
            {
            default:
                return EVENT_NONE;
            case SDL_WINDOWEVENT_CLOSE:
                return EVENT_WINDOW_CLOSE;
            }
        }

        if (ImGui_ImplSdl_ProcessEvent(m_pImpl->getLastSDLWindowEventGUIState(), &m_pImpl->m_LastPolledEvent)) {
            return EVENT_NONE;
        }

        return EVENT_NONE;
    }

    WindowManager::WindowID WindowManager::getClosedWindow() const
    {
        assert(m_pImpl->m_LastPolledEvent.type == SDL_WINDOWEVENT 
            && m_pImpl->m_LastPolledEvent.window.event == SDL_WINDOWEVENT_CLOSE);
        return m_pImpl->m_nLastEventWindow;
    }
}
