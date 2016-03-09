#include <melisandre/melisandre.hpp>
#include <melisandre/opengl/opengl.hpp>
#include <melisandre/maths/types.hpp>

#include <melisandre/viewer/WindowManager.hpp>
#include <glm/glm.hpp>
#include <embree2/rtcore.h>

#include <iostream>

#pragma warning(push, 0)
#include <OpenEXR/ImfInputFile.h>
#include <OpenEXR/ImfOutputFile.h>
#include <OpenEXR/ImfArray.h>
#include <OpenEXR/ImfChannelList.h>
#include <OpenEXR/ImfIntAttribute.h>
#pragma warning(pop)

#include <melisandre/viewer/imgui/imgui.h>
#include <melisandre/maths/geometry.hpp>
#include <melisandre/opengl/GLScreenFramebuffer.hpp>

struct EXRSetGlobalThreadCountRAII {
    EXRSetGlobalThreadCountRAII() {
        Imf::setGlobalThreadCount(8);
    }

    ~EXRSetGlobalThreadCountRAII() {
        Imf::setGlobalThreadCount(0);
    }
};

struct Vec4f {
    float x, y, z, w;
};

using namespace mls;

void loadEXRImage(const std::string& filepath, bool useCache) {
    EXRSetGlobalThreadCountRAII raii;

    Imf::InputFile file(filepath.c_str());

    const auto dw = file.header().dataWindow();
    const auto dx = dw.min.x;
    const auto dy = dw.min.y;
    const auto width = dw.max.x - dw.min.x + 1;
    const auto height = dw.max.y - dw.min.y + 1;

    Imf::FrameBuffer frameBuffer;

    const auto basePtr = (char*) NULL;//(char*)(pImage->getPixels() - dx - dy * pImage->getWidth());
    const auto xStride = sizeof(4 * sizeof(float));
    const auto yStride = sizeof(4 * sizeof(float)) * 16;
    const auto xSampling = 1;
    const auto ySampling = 1;
    const auto fillValue = 0.0;

    frameBuffer.insert("R",
        Imf::Slice(Imf::FLOAT,
            basePtr + offsetof(float4, x),
            xStride, yStride,
            xSampling, ySampling,
            fillValue));
    frameBuffer.insert("G",
        Imf::Slice(Imf::FLOAT,
            basePtr + offsetof(float4, y),
            xStride, yStride,
            xSampling, ySampling,
            fillValue));
    frameBuffer.insert("B",
        Imf::Slice(Imf::FLOAT,
            basePtr + offsetof(float4, z),
            xStride, yStride,
            xSampling, ySampling,
            fillValue));
    frameBuffer.insert("rcpWeight",
        Imf::Slice(Imf::FLOAT,
            basePtr + offsetof(float4, w),
            xStride, yStride,
            xSampling, ySampling,
            fillValue));

    file.setFrameBuffer(frameBuffer);
    file.readPixels(dw.min.y, dw.max.y);

    return;
}


// Creating a node graph editor for ImGui
// Quick demo, not production code! This is more of a demo of how to use ImGui to create custom stuff.
// Better version by @daniel_collin here https://gist.github.com/emoon/b8ff4b4ce4f1b43e79f2
// See https://github.com/ocornut/imgui/issues/306
// v0.02
// Animated gif: https://cloud.githubusercontent.com/assets/8225057/9472357/c0263c04-4b4c-11e5-9fdf-2cd4f33f6582.gif

// NB: You can use math functions/operators on ImVec2 if you #define IMGUI_DEFINE_MATH_OPERATORS and #include "imgui_internal.h"
// Here we only declare simple +/- operators so others don't leak into the demo code.
static inline ImVec2 operator+(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y); }
static inline ImVec2 operator-(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x - rhs.x, lhs.y - rhs.y); }

// Dummy
struct Node
{
    int     ID;
    char    Name[32];
    ImVec2  Pos, Size;
    float   Value;
    ImVec4  Color;
    int     InputsCount, OutputsCount;

    Node(int id, const char* name, const ImVec2& pos, float value, const ImVec4& color, int inputs_count, int outputs_count) { ID = id; strncpy(Name, name, 31); Name[31] = 0; Pos = pos; Value = value; Color = color; InputsCount = inputs_count; OutputsCount = outputs_count; }

    ImVec2 GetInputSlotPos(int slot_no) const { return ImVec2(Pos.x, Pos.y + Size.y * ((float)slot_no + 1) / ((float)InputsCount + 1)); }
    ImVec2 GetOutputSlotPos(int slot_no) const { return ImVec2(Pos.x + Size.x, Pos.y + Size.y * ((float)slot_no + 1) / ((float)OutputsCount + 1)); }
};

struct NodeLink
{
    int     InputIdx, InputSlot, OutputIdx, OutputSlot;

    NodeLink(int input_idx, int input_slot, int output_idx, int output_slot) { InputIdx = input_idx; InputSlot = input_slot; OutputIdx = output_idx; OutputSlot = output_slot; }
};

// Really dumb data structure provided for the example.
// Note that we storing links are INDICES (not ID) to make example code shorter, obviously a bad idea for any general purpose code.
static void ShowExampleAppCustomNodeGraph(ImVector<Node>& nodes, ImVector<NodeLink>& links, size_t width, size_t height, bool* opened = nullptr)
{
    ImGui::SetNextWindowSize(ImVec2(width, height));
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    if (!ImGui::Begin("Example: Custom Node Graph", opened, ImVec2(width, height), -1.f, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
    {
        ImGui::End();
        return;
    }

    static bool inited = false;
    static ImVec2 scrolling = ImVec2(0.0f, 0.0f);
    static bool show_grid = true;
    static int node_selected = -1;
    if (!inited)
    {
        nodes.push_back(Node(0, "MainTex", ImVec2(40, 50), 0.5f, ImColor(255, 100, 100), 1, 1));
        nodes.push_back(Node(1, "BumpMap", ImVec2(40, 150), 0.42f, ImColor(200, 100, 200), 1, 1));
        nodes.push_back(Node(2, "Combine", ImVec2(270, 80), 1.0f, ImColor(0, 200, 100), 2, 2));
        links.push_back(NodeLink(0, 0, 2, 0));
        links.push_back(NodeLink(1, 0, 2, 1));
        inited = true;
    }

    // Draw a list of nodes on the left side
    bool open_context_menu = false;
    int node_hovered_in_list = -1;
    int node_hovered_in_scene = -1;
    ImGui::BeginChild("node_list", ImVec2(100, 0));
    ImGui::Text("Nodes");
    ImGui::Separator();
    for (int node_idx = 0; node_idx < nodes.Size; node_idx++)
    {
        Node* node = &nodes[node_idx];
        ImGui::PushID(node->ID);
        if (ImGui::Selectable(node->Name, node->ID == node_selected))
            node_selected = node->ID;
        if (ImGui::IsItemHovered())
        {
            node_hovered_in_list = node->ID;
            open_context_menu |= ImGui::IsMouseClicked(1);
        }
        ImGui::PopID();
    }
    ImGui::EndChild();

    ImGui::SameLine();
    ImGui::BeginGroup();

    const float NODE_SLOT_RADIUS = 4.0f;
    const ImVec2 NODE_WINDOW_PADDING(8.0f, 8.0f);

    // Create our child canvas
    ImGui::Text("Hold middle mouse button to scroll (%.2f,%.2f)", scrolling.x, scrolling.y);
    ImGui::SameLine(ImGui::GetWindowWidth() - 100);
    ImGui::Checkbox("Show grid", &show_grid);
    ImGui::Text("win_pos (%.2f,%.2f)", ImGui::GetCursorScreenPos().x, ImGui::GetCursorScreenPos().y);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1, 1));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleColor(ImGuiCol_ChildWindowBg, ImColor(60, 60, 70, 200));
    ImGui::BeginChild("scrolling_region", ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove);
    ImGui::PushItemWidth(120.0f);

    ImVec2 offset = ImGui::GetCursorScreenPos() - scrolling;
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    draw_list->ChannelsSplit(2);

    // Display grid
    if (show_grid)
    {
        ImU32 GRID_COLOR = ImColor(200, 200, 200, 40);
        float GRID_SZ = 64.0f;
        ImVec2 win_pos = ImGui::GetCursorScreenPos();
        ImVec2 canvas_sz = ImGui::GetWindowSize();
        for (float x = fmodf(offset.x, GRID_SZ); x < canvas_sz.x; x += GRID_SZ)
            draw_list->AddLine(ImVec2(x, 0.0f) + win_pos, ImVec2(x, canvas_sz.y) + win_pos, GRID_COLOR);
        for (float y = fmodf(offset.y, GRID_SZ); y < canvas_sz.y; y += GRID_SZ)
            draw_list->AddLine(ImVec2(0.0f, y) + win_pos, ImVec2(canvas_sz.x, y) + win_pos, GRID_COLOR);
    }

    // Display links
    draw_list->ChannelsSetCurrent(0); // Background
    for (int link_idx = 0; link_idx < links.Size; link_idx++)
    {
        NodeLink* link = &links[link_idx];
        Node* node_inp = &nodes[link->InputIdx];
        Node* node_out = &nodes[link->OutputIdx];
        ImVec2 p1 = offset + node_inp->GetOutputSlotPos(link->InputSlot);
        ImVec2 p2 = offset + node_out->GetInputSlotPos(link->OutputSlot);
        draw_list->AddBezierCurve(p1, p1 + ImVec2(+50, 0), p2 + ImVec2(-50, 0), p2, ImColor(200, 200, 100), 3.0f);
    }

    // Display nodes
    for (int node_idx = 0; node_idx < nodes.Size; node_idx++)
    {
        Node* node = &nodes[node_idx];
        ImGui::PushID(node->ID);
        ImVec2 node_rect_min = offset + node->Pos;

        // Display node contents first
        draw_list->ChannelsSetCurrent(1); // Foreground
        bool old_any_active = ImGui::IsAnyItemActive();
        ImGui::SetCursorScreenPos(node_rect_min + NODE_WINDOW_PADDING);
        ImGui::BeginGroup(); // Lock horizontal position
        ImGui::Text("%s", node->Name);
        ImGui::SliderFloat("##value", &node->Value, 0.0f, 1.0f, "Alpha %.2f");
        ImGui::ColorEdit3("##color", &node->Color.x);
        ImGui::EndGroup();

        // Save the size of what we have emitted and whether any of the widgets are being used
        bool node_widgets_active = (!old_any_active && ImGui::IsAnyItemActive());
        node->Size = ImGui::GetItemRectSize() + NODE_WINDOW_PADDING + NODE_WINDOW_PADDING;
        ImVec2 node_rect_max = node_rect_min + node->Size;

        // Display node box
        draw_list->ChannelsSetCurrent(0); // Background
        ImGui::SetCursorScreenPos(node_rect_min);
        ImGui::InvisibleButton("node", node->Size);
        if (ImGui::IsItemHovered())
        {
            node_hovered_in_scene = node->ID;
            open_context_menu |= ImGui::IsMouseClicked(1);
        }
        bool node_moving_active = ImGui::IsItemActive();
        if (node_widgets_active || node_moving_active)
            node_selected = node->ID;
        if (node_moving_active && ImGui::IsMouseDragging(0))
            node->Pos = node->Pos + ImGui::GetIO().MouseDelta;

        ImU32 node_bg_color = (node_hovered_in_list == node->ID || node_hovered_in_scene == node->ID || (node_hovered_in_list == -1 && node_selected == node->ID)) ? ImColor(75, 75, 75) : ImColor(60, 60, 60);
        draw_list->AddRectFilled(node_rect_min, node_rect_max, node_bg_color, 4.0f);
        draw_list->AddRect(node_rect_min, node_rect_max, ImColor(100, 100, 100), 4.0f);
        for (int slot_idx = 0; slot_idx < node->InputsCount; slot_idx++)
            draw_list->AddCircleFilled(offset + node->GetInputSlotPos(slot_idx), NODE_SLOT_RADIUS, ImColor(150, 150, 150, 150));
        for (int slot_idx = 0; slot_idx < node->OutputsCount; slot_idx++)
            draw_list->AddCircleFilled(offset + node->GetOutputSlotPos(slot_idx), NODE_SLOT_RADIUS, ImColor(150, 150, 150, 150));

        ImGui::PopID();
    }
    draw_list->ChannelsMerge();

    // Open context menu
    if (!ImGui::IsAnyItemHovered() && ImGui::IsMouseHoveringWindow() && ImGui::IsMouseClicked(1))
    {
        node_selected = node_hovered_in_list = node_hovered_in_scene = -1;
        open_context_menu = true;
    }
    if (open_context_menu)
    {
        ImGui::OpenPopup("context_menu");
        if (node_hovered_in_list != -1)
            node_selected = node_hovered_in_list;
        if (node_hovered_in_scene != -1)
            node_selected = node_hovered_in_scene;
    }

    // Draw context menu
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
    if (ImGui::BeginPopup("context_menu"))
    {
        Node* node = node_selected != -1 ? &nodes[node_selected] : NULL;
        ImVec2 scene_pos = ImGui::GetMousePosOnOpeningCurrentPopup() - offset;
        if (node)
        {
            ImGui::Text("Node '%s'", node->Name);
            ImGui::Separator();
            if (ImGui::MenuItem("Rename..", NULL, false, false)) {}
            if (ImGui::MenuItem("Delete", NULL, false, false)) {}
            if (ImGui::MenuItem("Copy", NULL, false, false)) {}
        }
        else
        {
            if (ImGui::MenuItem("Add")) { nodes.push_back(Node(nodes.Size, "New node", scene_pos, 0.5f, ImColor(100, 100, 200), 2, 2)); }
            if (ImGui::MenuItem("Paste", NULL, false, false)) {}
        }
        ImGui::EndPopup();
    }
    ImGui::PopStyleVar();

    // Scrolling
    if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemActive() && ImGui::IsMouseDragging(2, 0.0f))
        scrolling = scrolling - ImGui::GetIO().MouseDelta;

    ImGui::PopItemWidth();
    ImGui::EndChild();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
    ImGui::EndGroup();

    ImGui::End();
}

using namespace mls;

#include <melisandre/system/logging.hpp>
#include <melisandre/opengl/GLScene.hpp>
#include <melisandre/opengl/GLShaderManager.hpp>
#include <melisandre/opengl/GLGBuffer.hpp>
#include <melisandre/opengl/GLFlatShadingPass.hpp>
#include <melisandre/opengl/GLGBufferRenderPass.hpp>

#include <melisandre/viewer/ViewController.hpp>

#include <melisandre/utils/EventDispatcher.hpp>

static const size_t FB_WIDTH = 1920;
static const size_t FB_HEIGHT = 1080;

int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cerr << "LAWL, set argv[0] to the path of a 3D model bawlosse" << std::endl;
        return EXIT_FAILURE;
    }

    std::cerr << "ImGui::GetVersion() = " << ImGui::GetVersion() << std::endl;
    mls::initLogging(argc, argv);

    //Initialization flag
    bool success = true;

    WindowManager windowManager{ 4, 5 };

    auto window2 = windowManager.createWindow("SDL Tutorial2", 1280, 720, WindowManager::WINDOW_RESIZABLE);
    auto window1 = windowManager.createWindow("\"The night is dark and full of terrors...but the fire burns them all away.\" - Melisandre", 1280, 720, WindowManager::WINDOW_RESIZABLE);
    

    mls::initOpenGL();

    GLuint vbo;
    glGenBuffers(1, &vbo);
    GLScene::Vertex triangle[] = {
        { float3(-1, -1, -5), float3(0, 0, 1), float2(0, 0) },
        { float3(1, -1, -6.5), float3(0, 0, 1), float2(0, 0) },
        { float3(0, 1, -6.5), float3(0, 0, 1), float2(0, 0) }
    };
    //glm::vec3 vertex[] = {
    //    glm::vec3(-1, -1, -5),
    //    glm::vec3(1, -1, -5),
    //    glm::vec3(1,  1, -5),
    //    glm::vec3(-1, -1, -5),
    //    glm::vec3(1,  1, -5),
    //    glm::vec3(-1,  1, -5)
    //};
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangle), triangle, GL_STATIC_DRAW);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLScene::Vertex),
        (const GLvoid*) MLS_OFFSETOF(GLScene::Vertex, m_Position));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GLScene::Vertex),
        (const GLvoid*) MLS_OFFSETOF(GLScene::Vertex, m_Normal));

    auto gProgramID = glCreateProgram();

    //Create vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);

    //Get vertex source
    const GLchar* vertexShaderSource[] =
    {
        "#version 330\nlayout(location = 0) in vec2 LVertexPos2D; void main() { gl_Position = vec4( LVertexPos2D.x, LVertexPos2D.y, 0, 1 ); }"
    };

    //Set vertex source
    glShaderSource(vertexShader, 1, vertexShaderSource, NULL);

    //Compile vertex source
    glCompileShader(vertexShader);

    //Check vertex shader for errors
    GLint vShaderCompiled = GL_FALSE;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &vShaderCompiled);
    if (vShaderCompiled != GL_TRUE)
    {
        printf("Unable to compile vertex shader %d!\n", vertexShader);
        return -1;
    }

    //Attach vertex shader to program
    glAttachShader(gProgramID, vertexShader);


    //Create fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    //Get fragment source
    const GLchar* fragmentShaderSource[] =
    {
        "#version 330\nout vec4 LFragment; void main() { LFragment = vec4( 1.0, 0.0, 1.0, 1.0 ); }"
    };

    //Set fragment source
    glShaderSource(fragmentShader, 1, fragmentShaderSource, NULL);

    //Compile fragment source
    glCompileShader(fragmentShader);

    //Check fragment shader for errors
    GLint fShaderCompiled = GL_FALSE;
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &fShaderCompiled);
    if (fShaderCompiled != GL_TRUE)
    {
        printf("Unable to compile fragment shader %d!\n", fragmentShader);
        return -1;
    }

    //Attach fragment shader to program
    glAttachShader(gProgramID, fragmentShader);


    //Link program
    glLinkProgram(gProgramID);

    //Check for errors
    GLint programSuccess = GL_TRUE;
    glGetProgramiv(gProgramID, GL_LINK_STATUS, &programSuccess);
    if (programSuccess != GL_TRUE)
    {
        printf("Error linking program %d!\n", gProgramID);
        return -1;
    }

    ViewController viewController;

    RTCDevice device = rtcNewDevice();
    RTCScene scene = rtcDeviceNewScene(device, RTC_SCENE_STATIC, RTC_INTERSECT1);

    GLShaderManager shaderManager;
    shaderManager.addDirectory(FilePath{ argv[0] }.directory() + FilePath{ "glsl" });

    GLGBufferRenderPass gbufferRenderPass{ shaderManager };
    GLFlatShadingPass flatShadingPass{ shaderManager };

    GLGBuffer gBuffer;
    gBuffer.init(FB_WIDTH, FB_HEIGHT);

    auto zFar = 3000.f;
    auto zNear = zFar / 1000.f;

    GLScreenTriangle screenTriangle;

    GLScreenFramebuffer screenFramebuffer;
    //std::vector<float4> green(1280 * 720, float4(0, 1, 0, 1));
    screenFramebuffer.init(FB_WIDTH, FB_HEIGHT);

    GLScene glScene = loadAssimpGLScene(argv[1]);

    //GLScene glScene;
    auto matOffset = glScene.getMaterialCount();
    glScene.addMaterial(float3(1, 1, 1), float3(0), 0.f, nullptr, nullptr, nullptr);

    GLScene::Triangle indices[] = { uint3(0, 1, 2) };
    glScene.addTriangleMesh(triangle, 3, indices, 1, matOffset);

    LOG(INFO) << "Mesh count = " << glScene.getTriangleMeshCount() << std::endl;

    auto done = false;

    bool show_window1 = true;

    ImVector<Node> nodes;
    ImVector<NodeLink> links;

    int2 mousePrevPosition;

    auto closedListener = windowManager.onWindowClosed([&](auto windowID) {
        done = true;
    });

    auto mousePressedListener = windowManager.onMouseButtonPressed([&](auto buttonID) {
        mousePrevPosition = windowManager.getMousePosition();
        std::cerr << "pressed" << std::endl;
    });

    while (!done) {
        windowManager.handleEvents();

        //// WINDOW 1: GRAPH
        windowManager.setCurrentWindow(window1);
        auto win1Size = windowManager.getWindowSize(window1);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glViewport(0, 0, win1Size.x, win1Size.y);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ShowExampleAppCustomNodeGraph(nodes, links, win1Size.x, win1Size.y);

        glUseProgram(gProgramID);

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        ImGui::Render();

        windowManager.swapCurrentWindow();


        // DRAW ON SCREEN FB
        auto projMatrix = perspective(45.f, gBuffer.getRatio(), zNear, zFar);
        gbufferRenderPass.render(projMatrix, viewController.getViewMatrix(), zFar, glScene, gBuffer);

        glDisable(GL_DEPTH_TEST);

        screenFramebuffer.bindForDrawing();
        glViewport(0, 0, screenFramebuffer.getSize().x, screenFramebuffer.getSize().y);

        flatShadingPass.render(gBuffer, inverse(projMatrix), viewController.getViewMatrix(), screenTriangle);

        //// WINDOW 2: SCENE
        windowManager.setCurrentWindow(window2);
        auto renderWinSize = windowManager.getWindowSize(window2);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui::ShowTestWindow();

        ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiSetCond_FirstUseEver);
        ImGui::Begin("Another Window", &show_window1);
        ImGui::Text("Hello");
        ImGui::End();

        screenFramebuffer.blitOnDefaultFramebuffer(uint4(0, 0, renderWinSize.x, renderWinSize.y), GL_COLOR_BUFFER_BIT, GL_LINEAR);

        glViewport(0, 0, renderWinSize.x, renderWinSize.y);

        ImGui::Render();

        windowManager.swapCurrentWindow();

        auto fSpeed = 3.f;
        real3 localTranslationVector(0);
        real3 localRotationVector(0);

        // Update view controller
        if (windowManager.getFocusedWindow() == window2) {
            if (!windowManager.guiHasKeyboardFocus()) {
                if (windowManager.isKeyPressed("z")) {
                    localTranslationVector.z -= fSpeed;
                }
                if (windowManager.isKeyPressed("s")) {
                    localTranslationVector.z += fSpeed;
                }
                if (windowManager.isKeyPressed("d")) {
                    localTranslationVector.x += fSpeed;
                }
                if (windowManager.isKeyPressed("q")) {
                    localTranslationVector.x -= fSpeed;
                }
                if (windowManager.isKeyPressed("a")) {
                    localRotationVector.z += 0.01;
                }
                if (windowManager.isKeyPressed("e")) {
                    localRotationVector.z -= 0.01;
                }
            }

            if (!windowManager.guiHasMouseFocus()) {
                if (windowManager.isMouseButtonPressed(WindowManager::MOUSE_BUTTON_LEFT)) {
                    auto mousePosition = windowManager.getMousePosition();
                    auto windowSize = windowManager.getWindowSize(window2);
                    
                    auto mouseOffset = mousePrevPosition - mousePosition;
                    localRotationVector.x += 0.01 * mouseOffset.x;
                    localRotationVector.y += 0.01 * mouseOffset.y;

                    mousePrevPosition = mousePosition;
                }
            }
        }
        viewController.moveLocal(localTranslationVector, localRotationVector);
    }
        

    return 0;
}