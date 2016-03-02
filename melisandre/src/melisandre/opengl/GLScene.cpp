#include "GLScene.hpp"

#include <melisandre/system/logging.hpp>
#include <assimp/Importer.hpp>
#include <assimp/ProgressHandler.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <melisandre/maths/constants.hpp>

namespace mls {

#ifndef USEVERTEXBUFFERUNIFIEDMEMORY

#ifdef USEONEBUFFER

GLScene::GLScene(const SceneGeometry& geometry) {
    uint32_t vertexCount = 0;
    uint32_t triangleCount = 0;
    for (const auto& mesh : geometry.m_TriangleMeshs) {
        vertexCount += mesh.m_Vertices.size();
        triangleCount += mesh.m_Triangles.size();
    }

    m_VBO = genBufferStorage<TriangleMesh::Vertex>(vertexCount, nullptr, GL_MAP_WRITE_BIT);
    m_IBO = genBufferStorage<TriangleMesh::Triangle>(triangleCount, nullptr, GL_MAP_WRITE_BIT);

    m_DrawCommands = genBufferStorage<DrawElementsIndirectCommand>(geometry.m_TriangleMeshs.size(),
                                                                   nullptr, GL_MAP_WRITE_BIT);

    auto pDrawCommand = m_DrawCommands.map(GL_MAP_WRITE_BIT);
    auto pVertex = m_VBO.map(GL_MAP_WRITE_BIT);
    auto pTriangle = m_IBO.map(GL_MAP_WRITE_BIT);

    uint32_t baseVertex = 0u;
    uint32_t indexOffset = 0u;
    for (const auto& mesh : geometry.m_TriangleMeshs) {
        std::copy(begin(mesh.m_Vertices), end(mesh.m_Vertices), pVertex);
        pVertex += mesh.m_Vertices.size();

        std::copy(begin(mesh.m_Triangles), end(mesh.m_Triangles), pTriangle);
        pTriangle += mesh.m_Triangles.size();

        pDrawCommand->count = mesh.m_Triangles.size() * 3;
        pDrawCommand->firstIndex = indexOffset;
        pDrawCommand->baseVertex = baseVertex;
        pDrawCommand->baseInstance = 0;
        pDrawCommand->instanceCount = 1;

        baseVertex += mesh.m_Vertices.size();
        indexOffset += mesh.m_Triangles.size() * 3;

        ++pDrawCommand;
    }

    m_VBO.unmap();
    m_IBO.unmap();
    m_DrawCommands.unmap();

    m_VAO.enableVertexAttrib(0);
    m_VAO.enableVertexAttrib(1);
    m_VAO.enableVertexAttrib(2);
    m_VAO.vertexAttribOffset(m_VBO.glId(), 0, 3, GL_FLOAT, GL_FALSE, sizeof(TriangleMesh::Vertex),
        MLS_OFFSETOF(TriangleMesh::Vertex, position));
    m_VAO.vertexAttribOffset(m_VBO.glId(), 1, 3, GL_FLOAT, GL_FALSE, sizeof(TriangleMesh::Vertex),
        MLS_OFFSETOF(TriangleMesh::Vertex, normal));
    m_VAO.vertexAttribOffset(m_VBO.glId(), 2, 2, GL_FLOAT, GL_FALSE, sizeof(TriangleMesh::Vertex),
        MLS_OFFSETOF(TriangleMesh::Vertex, texCoords));

    m_VAO.bind();
        m_IBO.bind(GL_ELEMENT_ARRAY_BUFFER);
    glBindVertexArray(0);

    m_Materials.reserve(geometry.m_Materials.size() + 1);

    m_Textures.emplace_back();
    Image white(1, 1);
    white.fill(Vec4f(1));
    fillTexture(m_Textures.back(), white);
    m_Textures.back().generateMipmap();
    m_Textures.back().setMinFilter(GL_NEAREST_MIPMAP_NEAREST);
    m_Textures.back().setMagFilter(GL_LINEAR);
    m_Textures.back().makeTextureHandleResident();

    static auto getTextureID = [&](const Shared<Image>& image) -> int {
        if(!image) {
            return 0;
        }

        auto it = m_TextureCache.find(image);
        if(it != end(m_TextureCache)) {
            return (*it).second;
        }

        uint32_t id = m_Textures.size();
        m_Textures.emplace_back();
        fillTexture(m_Textures.back(), *image);
        m_Textures.back().generateMipmap();
        m_Textures.back().setMinFilter(GL_NEAREST_MIPMAP_NEAREST);
        m_Textures.back().setMagFilter(GL_LINEAR);
        m_Textures.back().makeTextureHandleResident();

        if(id == 1) {
            std::cerr << GLuint64(m_Textures.back().getTextureHandle()) << std::endl;
        }

        m_TextureCache[image] = id;

        return id;
    };
    for (const auto& material: geometry.m_Materials) {
        m_Materials.emplace_back(material, getTextureID(material.m_KdTexture),
                                 getTextureID(material.m_KsTexture),  getTextureID(material.m_ShininessTexture));
    }
}

GLScene::GLMaterial::GLMaterial(const Material &material, int kdTextureID, int ksTextureID, int shininessTextureID):
    m_Kd(material.m_Kd), m_Ks(material.m_Ks), m_Shininess(material.m_Shininess),
    m_KdTextureID(kdTextureID), m_KsTextureID(ksTextureID), m_ShininessTextureID(shininessTextureID) {
}

void GLScene::render(uint32_t numInstances) const {
    m_VAO.bind();

    auto pDrawCommand = m_DrawCommands.map(GL_MAP_WRITE_BIT);
    for(auto i = 0u; i < m_DrawCommands.size(); ++i) {
        pDrawCommand->instanceCount = numInstances;
        ++pDrawCommand;
    }
    m_DrawCommands.unmap();

    m_DrawCommands.bind(GL_DRAW_INDIRECT_BUFFER);

    glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT,
                                0, m_DrawCommands.size(), 0);

    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
    glBindVertexArray(0);
}

void GLScene::render(GLMaterialUniforms& uniforms, uint32_t numInstances) const {
    m_VAO.bind();

    auto pDrawCommand = m_DrawCommands.map(GL_MAP_WRITE_BIT);
    for(auto i = 0u; i < m_DrawCommands.size(); ++i) {
        pDrawCommand->instanceCount = numInstances;
        ++pDrawCommand;
    }
    m_DrawCommands.unmap();

    m_DrawCommands.bind(GL_DRAW_INDIRECT_BUFFER);

    glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT,
                                0, m_DrawCommands.size(), 0);

    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
    glBindVertexArray(0);
}

#else

static const aiVector3D aiZERO{ 0.f, 0.f, 0.f };

static void loadAssimpGLMesh(const aiMesh* aimesh, size_t materialOffset, GLScene& scene) {
    auto materialID = materialOffset + aimesh->mMaterialIndex;

    std::vector<GLScene::Vertex> vertices;
    vertices.reserve(aimesh->mNumVertices);

    for (size_t vertexIdx = 0; vertexIdx < aimesh->mNumVertices; ++vertexIdx) {
        const aiVector3D* pPosition = aimesh->HasPositions() ? &aimesh->mVertices[vertexIdx] : &aiZERO;
        const aiVector3D* pNormal = aimesh->HasNormals() ? &aimesh->mNormals[vertexIdx] : &aiZERO;
        const aiVector3D* pTexCoords = aimesh->HasTextureCoords(0) ? &aimesh->mTextureCoords[0][vertexIdx] : &aiZERO;

        vertices.emplace_back(
            GLScene::Vertex{
                float3(pPosition->x, pPosition->y, pPosition->z),
                float3(pNormal->x, pNormal->y, pNormal->z),
                float2(pTexCoords->x, pTexCoords->y)
        });
    }

    std::vector<GLScene::Triangle> triangles;
    triangles.reserve(aimesh->mNumFaces);

    for (size_t triangleIdx = 0; triangleIdx < aimesh->mNumFaces; ++triangleIdx) {
        const aiFace& face = aimesh->mFaces[triangleIdx];
        triangles.emplace_back(face.mIndices[0], face.mIndices[1], face.mIndices[2]);
    }

    scene.addTriangleMesh(vertices.data(), vertices.size(), triangles.data(), triangles.size(), materialID);
}

static void loadAssimpGLMaterial(const aiMaterial* aimaterial, const FilePath& rootDirectoryPath, GLScene& scene) {
    auto pLogger = getLogger("loadAssimpGLMaterial");

    aiColor3D color;

    aiString ainame;
    aimaterial->Get(AI_MATKEY_NAME, ainame);
    std::string name = ainame.C_Str();

    pLogger->verbose(1, "Load material %v", name);

    float3 kd = zero<float3>(), ks = zero<float3>();
    float shininess = 0.f;

    if (AI_SUCCESS == aimaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color)) {
        kd = Vec3f(color.r, color.g, color.b);
    }

    //aiString path;

    /*if (AI_SUCCESS == aimaterial->GetTexture(aiTextureType_DIFFUSE, 0, &path,
        nullptr, nullptr, nullptr, nullptr, nullptr)) {
        pLogger->verbose(1, "Load texture %v", (basePath + path.data));
        material.m_DiffuseReflectanceTexture = loadImage(basePath + path.data, true);
    }*/

    if (AI_SUCCESS == aimaterial->Get(AI_MATKEY_COLOR_SPECULAR, color)) {
        ks = Vec3f(color.r, color.g, color.b);
    }

    /*if (AI_SUCCESS == aimaterial->GetTexture(aiTextureType_SPECULAR, 0, &path,
        nullptr, nullptr, nullptr, nullptr, nullptr)) {
        pLogger->verbose(1, "Load texture %v", (basePath + path.data));
        material.m_GlossyReflectanceTexture = loadImage(basePath + path.data, true);
    }*/

    aimaterial->Get(AI_MATKEY_SHININESS, shininess);

    /*if (AI_SUCCESS == aimaterial->GetTexture(aiTextureType_SHININESS, 0, &path,
        nullptr, nullptr, nullptr, nullptr, nullptr)) {
        pLogger->verbose(1, "Load texture %v", (basePath + path.data));
        material.m_ShininessTexture = loadImage(basePath + path.data, true);
    }*/

    scene.addMaterial(kd, ks, shininess, nullptr, nullptr, nullptr);
}

void loadAssimpGLScene(const std::string& filePath, GLScene& scene) {
    auto pLogger = getLogger("loadAssimpGLScene");

    Assimp::Importer importer;

    pLogger->info("Loading geometry of %v with Assimp.", filePath);

    //importer.SetExtraVerbose(true); // TODO: add logger and check for sponza
    const aiScene* aiscene = importer.ReadFile(filePath.c_str(),
        aiProcess_Triangulate |
        aiProcess_GenNormals |
        aiProcess_FlipUVs);
    if (aiscene) {
        pLogger->info("Number of meshes = %v", aiscene->mNumMeshes);

        auto materialOffset = scene.getMaterialCount();

        FilePath path = FilePath{ filePath }.directory();

        for (size_t materialIdx = 0; materialIdx < aiscene->mNumMaterials; ++materialIdx) {
            loadAssimpGLMaterial(aiscene->mMaterials[materialIdx], path, scene);
        }

        for (size_t meshIdx = 0u; meshIdx < aiscene->mNumMeshes; ++meshIdx) {
            loadAssimpGLMesh(aiscene->mMeshes[meshIdx], materialOffset, scene);
        }
    }
    else {
        pLogger->error("Assimp loading error on file %v: %v", filePath, importer.GetErrorString());
    }
}

GLScene loadAssimpGLScene(const std::string& filePath) {
    GLScene scene;
    loadAssimpGLScene(filePath, scene);
    return scene;
}

//GLScene::GLScene(const SceneGeometry& geometry) {
//    m_TriangleMeshs.reserve(geometry.getMeshCount());
//    for (const auto& mesh : geometry.getMeshs()) {
//        m_TriangleMeshs.emplace_back(mesh);
//    }
//
//    m_Materials.reserve(geometry.getMaterialCount() + 1);
//
//    m_Textures.emplace_back();
//    Image white(1, 1);
//    white.fill(Vec4f(1));
//    fillTexture(m_Textures.back(), white);
//    m_Textures.back().generateMipmap();
//    m_Textures.back().setMinFilter(GL_NEAREST_MIPMAP_NEAREST);
//    m_Textures.back().setMagFilter(GL_LINEAR);
//
//    auto getTextureID = [&](const Shared<Image>& image) -> int {
//        if(!image) {
//            return 0;
//        }
//
//        auto it = m_TextureCache.find(image);
//        if(it != end(m_TextureCache)) {
//            return (*it).second;
//        }
//
//        uint32_t id = m_Textures.size();
//        m_Textures.emplace_back();
//        fillTexture(m_Textures.back(), *image);
//        m_Textures.back().generateMipmap();
//        m_Textures.back().setMinFilter(GL_NEAREST_MIPMAP_NEAREST);
//        m_Textures.back().setMagFilter(GL_LINEAR);
//
//        m_TextureCache[image] = id;
//
//        return id;
//    };
//    for (const auto& material: geometry.getMaterials()) {
//        m_Materials.emplace_back(material, getTextureID(material.m_DiffuseReflectanceTexture),
//                                 getTextureID(material.m_GlossyReflectanceTexture),  getTextureID(material.m_ShininessTexture));
//    }
//}
//
//GLScene::GLTriangleMesh::GLTriangleMesh(const TriangleMesh& mesh):
//    m_VBO(mesh.m_Vertices),
//    m_IBO(mesh.m_Triangles),
//    m_MaterialID(mesh.m_MaterialID) {
//
//    m_VAO.enableVertexAttrib(0);
//    m_VAO.enableVertexAttrib(1);
//    m_VAO.enableVertexAttrib(2);
//    m_VAO.vertexAttribOffset(m_VBO.glId(), 0, 3, GL_FLOAT, GL_FALSE, sizeof(TriangleMesh::Vertex),
//        MLS_OFFSETOF(TriangleMesh::Vertex, position));
//    m_VAO.vertexAttribOffset(m_VBO.glId(), 1, 3, GL_FLOAT, GL_FALSE, sizeof(TriangleMesh::Vertex),
//        MLS_OFFSETOF(TriangleMesh::Vertex, normal));
//    m_VAO.vertexAttribOffset(m_VBO.glId(), 2, 2, GL_FLOAT, GL_FALSE, sizeof(TriangleMesh::Vertex),
//        MLS_OFFSETOF(TriangleMesh::Vertex, texCoords));
//
//    m_VAO.bind();
//    m_IBO.bind(GL_ELEMENT_ARRAY_BUFFER);
//    glBindVertexArray(0);
//}

void GLScene::TriangleMesh::render(size_t numInstances) const {
    m_VAO.bind();
    glDrawElementsInstanced(GL_TRIANGLES, m_IBO.size() * 3,
        GL_UNSIGNED_INT, 0, numInstances);
}

//GLScene::GLMaterial::GLMaterial(const Material &material, int kdTextureID, int ksTextureID, int shininessTextureID):
//    m_Kd(material.m_DiffuseReflectance), m_Ks(material.m_GlossyReflectance), m_Shininess(material.m_Shininess),
//    m_KdTextureID(kdTextureID), m_KsTextureID(ksTextureID), m_ShininessTextureID(shininessTextureID) {
//}

void GLScene::render(size_t numInstances) const {
    for (const auto& mesh : m_TriangleMeshs) {
        mesh.render(numInstances);
    }
    glBindVertexArray(0);
}

void GLScene::render(GLMaterialUniforms& uniforms, uint32_t numInstances) const {
    for (const auto& mesh : m_TriangleMeshs) {
        const auto& material = m_Materials[mesh.m_MaterialID];
        uniforms.uKd.set(material.m_Kd);
        uniforms.uKs.set(material.m_Ks);
        uniforms.uShininess.set(material.m_Shininess);

        if (material.m_KdTextureID >= 0) {
            m_Textures[material.m_KdTextureID].bind(0u);
            uniforms.uKdSampler.set(0u);
        }

        if (material.m_KsTextureID >= 0) {
            m_Textures[material.m_KsTextureID].bind(1u);
            uniforms.uKsSampler.set(1u);
        }

        if (material.m_ShininessTextureID >= 0) {
            m_Textures[material.m_ShininessTextureID].bind(2u);
            uniforms.uShininessSampler.set(2u);
        }

        mesh.render(numInstances);
    }
    glBindVertexArray(0);
}

void GLScene::addMaterial(Vec3f Kd, Vec3f Ks, float shininess,
    const Image* KdTexture, const Image* KsTexture, const Image* shininessTexture) {
    m_Materials.emplace_back(Kd, Ks, shininess, -1, -1, -1);
}

void GLScene::addTriangleMesh(
    const Vertex* pVertices, size_t vertexCount,
    const Triangle* pTriangles, size_t triangleCount,
    size_t materialID) {
    m_TriangleMeshs.emplace_back(pVertices, vertexCount, pTriangles, triangleCount, materialID);
}

#endif

#else

GLScene::GLScene(const SceneGeometry& geometry) {
    m_TriangleMeshs.reserve(geometry.m_TriangleMeshs.size());
    for (const auto& mesh : geometry.m_TriangleMeshs) {
        m_TriangleMeshs.emplace_back(mesh);
    }

    m_Materials.reserve(geometry.m_Materials.size() + 1);

    m_Textures.emplace_back();
    Image white(16, 16);
    white.fill(Vec4f(1));
    fillTexture(m_Textures.back(), white);
    m_Textures.back().generateMipmap();
    m_Textures.back().setMinFilter(GL_NEAREST_MIPMAP_NEAREST);
    m_Textures.back().setMagFilter(GL_LINEAR);
    m_Textures.back().makeTextureHandleResident();

    static auto getTextureID = [&](const Shared<Image>& image) -> int {
        if(!image) {
            return 0;
        }

        auto it = m_TextureCache.find(image);
        if(it != end(m_TextureCache)) {
            return (*it).second;
        }

        uint32_t id = m_Textures.size();
        m_Textures.emplace_back();
        fillTexture(m_Textures.back(), *image);
        m_Textures.back().generateMipmap();
        m_Textures.back().setMinFilter(GL_NEAREST_MIPMAP_NEAREST);
        m_Textures.back().setMagFilter(GL_LINEAR);
        m_Textures.back().makeTextureHandleResident();

        m_TextureCache[image] = id;

        return id;
    };
    for (const auto& material: geometry.m_Materials) {
        m_Materials.emplace_back(material, getTextureID(material.m_KdTexture),
                                 getTextureID(material.m_KsTexture),  getTextureID(material.m_ShininessTexture));
    }
}

GLScene::GLTriangleMesh::GLTriangleMesh(const TriangleMesh& mesh):
    m_MaterialID(mesh.m_MaterialID),
    m_VBO(mesh.m_Vertices, 0, GL_READ_ONLY),
    m_IBO(mesh.m_Triangles, 0, GL_READ_ONLY) {
}

void GLScene::GLTriangleMesh::render(uint32_t numInstances) const {
    glBufferAddressRangeNV(GL_VERTEX_ATTRIB_ARRAY_ADDRESS_NV, 0,
                           GLuint64(m_VBO.getGPUAddress()) + MLS_OFFSETOF(TriangleMesh::Vertex, position),
                           m_VBO.byteSize() - MLS_OFFSETOF(TriangleMesh::Vertex, position));

    glBufferAddressRangeNV(GL_VERTEX_ATTRIB_ARRAY_ADDRESS_NV, 1,
                           GLuint64(m_VBO.getGPUAddress()) + MLS_OFFSETOF(TriangleMesh::Vertex, normal),
                           m_VBO.byteSize() - MLS_OFFSETOF(TriangleMesh::Vertex, normal));

    glBufferAddressRangeNV(GL_VERTEX_ATTRIB_ARRAY_ADDRESS_NV, 2,
                           GLuint64(m_VBO.getGPUAddress()) + MLS_OFFSETOF(TriangleMesh::Vertex, texCoords),
                           m_VBO.byteSize() - MLS_OFFSETOF(TriangleMesh::Vertex, texCoords));

    glBufferAddressRangeNV(GL_ELEMENT_ARRAY_ADDRESS_NV, 0,
                           GLuint64(m_IBO.getGPUAddress()),
                           m_IBO.byteSize());

    glDrawElementsInstanced(GL_TRIANGLES, m_IBO.size() * 3, GL_UNSIGNED_INT, 0, numInstances);
}

GLScene::GLMaterial::GLMaterial(const Material &material, int kdTextureID, int ksTextureID, int shininessTextureID):
    m_Kd(material.m_Kd), m_Ks(material.m_Ks), m_Shininess(material.m_Shininess),
    m_KdTextureID(kdTextureID), m_KsTextureID(ksTextureID), m_ShininessTextureID(shininessTextureID) {
}

void GLScene::render(uint32_t numInstances) const {
    glEnableClientState(GL_VERTEX_ATTRIB_ARRAY_UNIFIED_NV);
    glEnableClientState(GL_ELEMENT_ARRAY_UNIFIED_NV);

    glVertexAttribFormatNV(0, 3, GL_FLOAT, GL_FALSE, sizeof(TriangleMesh::Vertex));
    glVertexAttribFormatNV(1, 3, GL_FLOAT, GL_FALSE, sizeof(TriangleMesh::Vertex));
    glVertexAttribFormatNV(2, 2, GL_FLOAT, GL_FALSE, sizeof(TriangleMesh::Vertex));

    for (const auto& mesh : m_TriangleMeshs) {
        mesh.render(numInstances);
    }

    glDisableClientState(GL_VERTEX_ATTRIB_ARRAY_UNIFIED_NV);
    glDisableClientState(GL_ELEMENT_ARRAY_UNIFIED_NV);
}

void GLScene::render(GLMaterialUniforms& uniforms, uint32_t numInstances) const {
    glEnableClientState(GL_VERTEX_ATTRIB_ARRAY_UNIFIED_NV);
    glEnableClientState(GL_ELEMENT_ARRAY_UNIFIED_NV);

    glVertexAttribFormatNV(0, 3, GL_FLOAT, GL_FALSE, sizeof(TriangleMesh::Vertex));
    glVertexAttribFormatNV(1, 3, GL_FLOAT, GL_FALSE, sizeof(TriangleMesh::Vertex));
    glVertexAttribFormatNV(2, 3, GL_FLOAT, GL_FALSE, sizeof(TriangleMesh::Vertex));

    for (const auto& mesh : m_TriangleMeshs) {
        const auto& material = m_Materials[mesh.m_MaterialID];
        uniforms.uKd.set(material.m_Kd);
        uniforms.uKs.set(material.m_Ks);
        uniforms.uShininess.set(material.m_Shininess);

        if(material.m_KdTextureID >= 0) {
            uniforms.uKdSampler.set(m_Textures[material.m_KdTextureID].getTextureHandle());
        }

        if(material.m_KsTextureID >= 0) {
            uniforms.uKsSampler.set(m_Textures[material.m_KsTextureID].getTextureHandle());
        }

        if(material.m_ShininessTextureID >= 0) {
            uniforms.uShininessSampler.set(m_Textures[material.m_ShininessTextureID].getTextureHandle());
        }

        mesh.render(numInstances);
    }

    glDisableClientState(GL_VERTEX_ATTRIB_ARRAY_UNIFIED_NV);
    glDisableClientState(GL_ELEMENT_ARRAY_UNIFIED_NV);
}

#endif

}
