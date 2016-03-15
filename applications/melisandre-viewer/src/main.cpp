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

#include <melisandre/viewer/gui.hpp>
#include <melisandre/maths/geometry.hpp>
#include <melisandre/opengl/GLScreenFramebuffer.hpp>

#include "ComputeGraph.hpp"

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

using namespace mls;

#include <melisandre/system/logging.hpp>
#include <melisandre/opengl/GLScene.hpp>
#include <melisandre/opengl/GLShaderManager.hpp>
#include <melisandre/opengl/GLGBuffer.hpp>
#include <melisandre/opengl/GLFlatShadingPass.hpp>
#include <melisandre/opengl/GLGBufferRenderPass.hpp>

#include <melisandre/viewer/ViewController.hpp>

#include <melisandre/utils/EventDispatcher.hpp>

struct handler {
    void notify() {};
};

template<typename T>
EventDispatcher<void()> addInput(const std::string& name, T& ref) {

}

template<typename T>
handler addOutput(const std::string& name, T& ref, std::vector<EventDispatcher<void()>> dependencies = {}) {

}

class GLGBufferRenderNode: public ComputeNode {
public:
    static const size_t nodeID;

    GLGBufferRenderNode(ComputeGraph& graph) :
        ComputeNode { graph }
    {
        
    }

    void compute() {
        if (pScene && gbufferRenderPass) {
            gbufferRenderPass->render(projMatrix, viewMatrix, zFar, *pScene, gBuffer);
            gBufferOutput.notify();
        }
    }

private:
    // Inputs:
    float zFar;  // Output of viewcontrollernode must be connected here
    float4x4 projMatrix = float4x4(1); // Output of viewcontrollernode must be connected here
    float4x4 viewMatrix = float4x4(1); // Output of viewcontrollernode must be connected here
    const GLScene* pScene = nullptr;
    const GLShaderManager* shaderManager = nullptr;

    // Intermediate:
    Unique<GLGBufferRenderPass> gbufferRenderPass;

    // Outputs:
    GLGBuffer gBuffer;
    uint64_t computeTime; // Example of a statistic

    EventDispatcher<void()> zFarInput = addInput("zFar", zFar);
    EventDispatcher<void()> projMatrixInput = addInput("projMatrix", projMatrix);
    EventDispatcher<void()> viewMatrixInput = addInput("viewMatrix", viewMatrix);
    EventDispatcher<void()> sceneInput = addInput("scene", pScene);
    EventDispatcher<void()> shaderManagerInput = addInput("shaderManager", shaderManager);

    // Compliqué...
    std::function<void(void)> listener = shaderManagerInput.addListener([this]() {
        gbufferRenderPass = makeUnique<GLGBufferRenderPass>(*shaderManager);
    });

    handler gBufferOutput = addOutput("GBuffer", gBuffer, { zFarInput, projMatrixInput, viewMatrixInput, sceneInput }); // a dependency list
    handler computeTimeOutput = addOutput("computeTime", computeTime);
};

const size_t GLGBufferRenderNode::nodeID = addNodeType<GLGBufferRenderNode>("GLGBufferRenderNode");

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


    int currentItem = 0;

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
        ImGui::PlotLines("Sin", [](void* pData, int nIdx) { return mls::sin(nIdx * 0.2f);  }, nullptr, 100);

        static const char* labels[] = {
            "label1", "label20", "label43", "label78"
        };

        ImGui::Combo("Choice", &currentItem, [](void* pLabels, int nIdx, const char** pOutText) {
            *pOutText = ((const char**)pLabels)[nIdx];
            return true;
        }, labels, std::size(labels));
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