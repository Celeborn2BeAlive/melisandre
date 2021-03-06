#include "GLShaderManager.hpp"
#include <melisandre/system/files.hpp>

#include <stdexcept>

namespace mls {

GLShaderManager::GLShaderManager() {
    m_ExtToType["vs.glsl"] = GL_VERTEX_SHADER;
    m_ExtToType["gs.glsl"] = GL_GEOMETRY_SHADER;
    m_ExtToType["fs.glsl"] = GL_FRAGMENT_SHADER;
    m_ExtToType["cs.glsl"] = GL_COMPUTE_SHADER;
}

GLShaderManager::GLShaderManager(const FilePath& dirPath):
    GLShaderManager() {
    addDirectory(dirPath);
}

void GLShaderManager::addDirectory(const FilePath& dirPath) {
    m_pLogger->info("Compile shaders from directory %v...", dirPath);

    if (!isDirectory(dirPath)) {
        m_pLogger->error("%v is not a directory", dirPath.c_str());
    }

    recursiveCompileShaders(FilePath{ "" }, dirPath);
}

void GLShaderManager::recursiveCompileShaders(const FilePath& relativePath, const FilePath& dir) {
    for (const auto& file : getContainedFiles(dir)) {
        auto completePath = dir + file;
        if (isRegularFile(completePath)) {
            if (file.endsWith(".xs.glsl")) {
                auto src = loadShaderSource(completePath.str());
                auto baseName = (relativePath + file).str();
                baseName = baseName.substr(0, baseName.size() - 2);

                for (const auto& pair : m_ExtToType) {
                    auto name = baseName + pair.first;
                    m_pLogger->info("Compile shader %v...", name);
                    try {
                        m_ShadersMap.emplace(name, compileShader(pair.second, src));
                    }
                    catch (...) {
                        m_pLogger->error("Error compiling shader %v", completePath);
                        throw;
                    }
                }
            }
            else {
                for (auto pair : m_ExtToType) {
                    if (file.endsWith(pair.first)) {
                        m_pLogger->info("Compile shader %v...", relativePath + file);
                        try {
                            m_ShadersMap.emplace((relativePath + file).str(), compileShader(pair.second, loadShaderSource(completePath.str())));
                        }
                        catch (...) {
                            m_pLogger->error("Error compiling shader %v", completePath);
                            throw;
                        }
                    }
                }
            }
        }
        else if (isDirectory(completePath)) {
            recursiveCompileShaders(relativePath + file, completePath);
        }
    }
}

const GLShader& GLShaderManager::getShader(const std::string& name) const {
    FilePath path(name);
    auto it = m_ShadersMap.find(path.str());
    if(it == std::end(m_ShadersMap)) {
        m_pLogger->error("Shader %v not found.", name);
        throw std::runtime_error("Shader " + name + " not found");
    }
    return (*it).second;
}

GLProgram GLShaderManager::buildProgram(const std::vector<std::string>& shaders) const {
    GLProgram program;
    for (const auto& shader : shaders) {
        program.attachShader(getShader(shader));
    }

    if (!program.link()) {
        std::cerr << program.getInfoLog() << std::endl;
        throw std::runtime_error(program.getInfoLog());
    }

    return program;
}

}
