#pragma once

#include <unordered_map>
#include <vector>
//#include <melisandre/scene/SceneGeometry.hpp>
#include "utils/GLutils.hpp"
#include <melisandre/system/memory.hpp>
#include <melisandre/image/Image.hpp>

namespace mls {

struct GLMaterialUniforms {
    GLUniform<Vec3f> uKd;
    GLUniform<Vec3f> uKs;
    GLUniform<float> uShininess;
    GLUniform<GLSLSampler2Df> uKdSampler;
    GLUniform<GLSLSampler2Df> uKsSampler;
    GLUniform<GLSLSampler2Df> uShininessSampler;

    GLMaterialUniforms(const GLProgram& program):
        uKd(program, "uMaterial.Kd"),
        uKs(program, "uMaterial.Ks"),
        uShininess(program, "uMaterial.shininess"),
        uKdSampler(program, "uMaterial.KdSampler"),
        uKsSampler(program, "uMaterial.KsSampler"),
        uShininessSampler(program, "uMaterial.shininessSampler") {
    }
};

//#define USEVERTEXBUFFERUNIFIEDMEMORY
#ifndef USEVERTEXBUFFERUNIFIEDMEMORY

//#define USEONEBUFFER
#ifdef USEONEBUFFER

#define USEMULTIDRAWINDIRECT
#ifdef USEMULTIDRAWINDIRECT

class GLScene {
public:
    GLScene(const SceneGeometry& geometry);

    void render(uint32_t numInstances = 1) const;

    void render(GLMaterialUniforms& uniforms, uint32_t numInstances = 1) const;
private:
    GLImmutableBuffer<TriangleMesh::Vertex> m_VBO;
    GLImmutableBuffer<TriangleMesh::Triangle> m_IBO;
    GLVertexArray m_VAO;

    struct DrawElementsIndirectCommand {
        GLuint count;
        GLuint instanceCount;
        GLuint firstIndex;
        GLuint baseVertex;
        GLuint baseInstance;
    };

    mutable GLImmutableBuffer<DrawElementsIndirectCommand> m_DrawCommands;

    std::vector<GLTexture2D> m_Textures;
    std::unordered_map<Shared<Image>, int> m_TextureCache;

    struct GLMaterial {
        Vec3f m_Kd;
        Vec3f m_Ks;
        float m_Shininess;

        int m_KdTextureID = -1;
        int m_KsTextureID = -1;
        int m_ShininessTextureID = -1;

        GLMaterial(const Material& material, int kdTextureID, int ksTextureID, int shininessTextureID);
    };

    std::vector<GLMaterial> m_Materials;
};

#else

class GLScene {
public:
    GLScene(const SceneGeometry& geometry);

    void render(uint32_t numInstances = 1) const;

    void render(GLMaterialUniforms& uniforms, uint32_t numInstances = 1) const;
private:
    GLImmutableBuffer<TriangleMesh::Vertex> m_VBO;
    GLImmutableBuffer<TriangleMesh::Triangle> m_IBO;
    GLVertexArray m_VAO;

    struct GLTriangleMesh {
        uint32_t m_nBaseVertex;
        uint32_t m_nIndexOffset;
        uint32_t m_nIndexCount;
        uint32_t m_MaterialID;

        GLTriangleMesh() = default;

        GLTriangleMesh(uint32_t baseVertex, uint32_t indexOffset,
                       uint32_t indexCount, uint32_t materialID);

        void render(uint32_t numInstances) const;
    };

    std::vector<GLTexture2D> m_Textures;
    std::unordered_map<Shared<Image>, int> m_TextureCache;

    struct GLMaterial {
        Vec3f m_Kd;
        Vec3f m_Ks;
        float m_Shininess;

        int m_KdTextureID = -1;
        int m_KsTextureID = -1;
        int m_ShininessTextureID = -1;

        GLMaterial(const Material& material, int kdTextureID, int ksTextureID, int shininessTextureID);
    };

    std::vector<GLTriangleMesh> m_TriangleMeshs;
    std::vector<GLMaterial> m_Materials;
};

#endif

#else

class GLScene {
public:
    struct Vertex {
        float3 m_Position;
        float3 m_Normal;
        float2 m_TexCoords;
    };
    using Triangle = uint3;

    //GLScene(const SceneGeometry& geometry);

    void render(size_t numInstances = 1) const;

    void render(GLMaterialUniforms& uniforms, uint32_t numInstances = 1) const;

    void addMaterial(Vec3f Kd, Vec3f Ks, float shininess, 
        const Image* KdTexture, const Image* KsTexture, const Image* shininessTexture);

    void addTriangleMesh(
        const Vertex* pVertices, size_t vertexCount,
        const Triangle* pTriangles, size_t triangleCount,
        size_t materialID);

    size_t getMaterialCount() const {
        return m_Materials.size();
    }

    size_t getTriangleMeshCount() const {
        return m_TriangleMeshs.size();
    }

private:
    struct TriangleMesh {
        GLBufferStorage<Vertex> m_VBO;
        GLBufferStorage<Triangle> m_IBO;
        GLVertexArray m_VAO;
        size_t m_MaterialID;

        TriangleMesh(const Vertex* pVertices, size_t vertexCount,
            const Triangle* pTriangles, size_t triangleCount, size_t materialID) :
            m_VBO(vertexCount, pVertices), m_IBO(triangleCount, pTriangles), m_MaterialID(materialID) {
            m_VAO.enableVertexAttrib(0);
            m_VAO.enableVertexAttrib(1);
            m_VAO.enableVertexAttrib(2);
            m_VAO.vertexAttribOffset(m_VBO.glId(), 0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                MLS_OFFSETOF(Vertex, m_Position));
            m_VAO.vertexAttribOffset(m_VBO.glId(), 1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                MLS_OFFSETOF(Vertex, m_Normal));
            m_VAO.vertexAttribOffset(m_VBO.glId(), 2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                MLS_OFFSETOF(Vertex, m_TexCoords));

            m_VAO.bind();
            m_IBO.bind(GL_ELEMENT_ARRAY_BUFFER);
            glBindVertexArray(0);
        }

        void render(size_t numInstances) const;
    };

    std::vector<GLTexture2D> m_Textures; // #todo: asset manager
    std::unordered_map<const Image*, size_t> m_TextureCache;

    struct Material {
        float3 m_Kd;
        float3 m_Ks;
        float m_Shininess;

        int m_KdTextureID = -1;
        int m_KsTextureID = -1;
        int m_ShininessTextureID = -1;

        Material(float3 kd, float3 ks, float shininess, int kdTextureID, int ksTextureID, int shininessTextureID) :
            m_Kd(kd), m_Ks(ks), m_Shininess(shininess), m_KdTextureID(kdTextureID), m_KsTextureID(ksTextureID), m_ShininessTextureID(shininessTextureID) {
        }
    };

    std::vector<TriangleMesh> m_TriangleMeshs;
    std::vector<Material> m_Materials;
};

void loadAssimpGLScene(const std::string& filePath, GLScene& scene);

GLScene loadAssimpGLScene(const std::string& filePath);

#endif

#else

class GLScene {
public:
    GLScene(const SceneGeometry& geometry);

    void render(uint32_t numInstances = 1) const;

    void render(GLMaterialUniforms& uniforms, uint32_t numInstances = 1) const;
private:
    struct GLTriangleMesh {
        GLImmutableBuffer<TriangleMesh::Vertex> m_VBO;
        GLImmutableBuffer<TriangleMesh::Triangle> m_IBO;
        uint32_t m_MaterialID;

        GLTriangleMesh() = default;

        GLTriangleMesh(GLTriangleMesh&& rvalue) :
            m_VBO(std::move(rvalue.m_VBO)),
            m_IBO(std::move(rvalue.m_IBO)),
            m_MaterialID(rvalue.m_MaterialID) {
        }

        GLTriangleMesh(const TriangleMesh& mesh);

        void render(uint32_t numInstances) const;
    };

    std::vector<GLTexture2D> m_Textures;
    std::unordered_map<Shared<Image>, int> m_TextureCache;

    struct GLMaterial {
        Vec3f m_Kd;
        Vec3f m_Ks;
        float m_Shininess;

        int m_KdTextureID = -1;
        int m_KsTextureID = -1;
        int m_ShininessTextureID = -1;

        GLMaterial(const Material& material, int kdTextureID, int ksTextureID, int shininessTextureID);
    };

    std::vector<GLTriangleMesh> m_TriangleMeshs;
    std::vector<GLMaterial> m_Materials;
};

#endif

}
