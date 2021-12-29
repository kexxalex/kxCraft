#pragma once


#include <GL/glew.h>
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

class Shader {
public:
    Shader(const char *shaderPath, bool &isOK);

    ~Shader();

    inline void Bind() const noexcept { glUseProgram(programID); }
    inline void Release() const noexcept { glUseProgram(0); }
    bool Reload() noexcept;

    bool Load() noexcept;

    [[nodiscard]] inline unsigned int getID() const noexcept { return programID; }

    void setBool(const char *name, bool value) noexcept;
    void setInt(const char *name, int value) noexcept;
    void setFloat(const char *name, float value) noexcept;
    void setDouble(const char *name, double value) noexcept;

    void setFloat3(const char *name, const glm::fvec3 &value) noexcept;
    void setDouble3(const char *name, const glm::dvec3 &value) noexcept;

    void setMatrixFloat4(const char *name, const glm::fmat4 &matrix) noexcept;

    std::unordered_map<std::string, GLint> uniforms;

private:
    std::string shaderName;
    GLuint programID{0};
};
