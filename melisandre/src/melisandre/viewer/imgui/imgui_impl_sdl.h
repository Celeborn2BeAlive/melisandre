// ImGui SDL2 binding with OpenGL
// You can copy and use unmodified imgui_impl_* files in your project. 
// If you use this binding you'll need to call 4 functions: ImGui_ImplXXXX_Init(), ImGui_ImplXXXX_NewFrame(), ImGui::Render() and ImGui_ImplXXXX_Shutdown().
// See main.cpp for an example of using this.
// https://github.com/ocornut/imgui

struct SDL_Window;
typedef union SDL_Event SDL_Event;
struct ImGuiState;

IMGUI_API void        ImGui_ImplSdl_Init(ImGuiState* state, SDL_Window *window);
IMGUI_API void        ImGui_ImplSdl_Shutdown(ImGuiState* state);
IMGUI_API void        ImGui_ImplSdl_NewFrame(ImGuiState* state, SDL_Window *window);
IMGUI_API bool        ImGui_ImplSdl_ProcessEvent(ImGuiState* state, SDL_Event* event);
