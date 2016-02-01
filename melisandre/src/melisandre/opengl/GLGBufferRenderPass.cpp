#include "GLGBufferRenderPass.hpp"

namespace mls {

GLGBufferRenderPass::GLGBufferRenderPass(const GLShaderManager& shaderManager):
    m_Program(shaderManager.buildProgram({ "geometryPass.vs", "geometryPass.fs" })){
}

void GLGBufferRenderPass::render(const Mat4f& projMatrix, const Mat4f& viewMatrix, float zFar,
            const GLScene& scene, GLGBuffer& gbuffer) {
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, GLsizei(gbuffer.getWidth()), GLsizei(gbuffer.getHeight()));
    gbuffer.bindForDrawing();

    m_Program.use();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Mat4f MVPMatrix = projMatrix * viewMatrix;
    Mat4f MVMatrix = viewMatrix;

    uMVPMatrix.set(MVPMatrix);
    uMVMatrix.set(MVMatrix);

    uZFar.set(zFar);

    scene.render(m_MaterialUniforms);
}

}
