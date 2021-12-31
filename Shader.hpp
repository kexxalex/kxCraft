#pragma once


#include <GL/glew.h>
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

class Shader {
public:
    Shader() = default;
    Shader(std::string_view shaderPath, bool &isOK);

    Shader(const Shader &shader) = default;

    ~Shader();

    inline void Bind() const noexcept { glUseProgram(programID); }
    bool Reload() noexcept;

    bool Load() noexcept;

    [[nodiscard]] inline unsigned int getID() const noexcept { return programID; }

    void setBool(std::string_view name, bool value) noexcept;
    void setInt(std::string_view name, int value) noexcept;
    void setFloat(std::string_view name, float value) noexcept;
    void setDouble(std::string_view name, double value) noexcept;

    void setFloat3(std::string_view name, const glm::fvec3 &value) noexcept;
    void setDouble3(std::string_view name, const glm::dvec3 &value) noexcept;

    void setMatrixFloat4(std::string_view name, const glm::fmat4 &matrix) noexcept;

    std::unordered_map<std::string_view, GLint> uniforms;

private:
    std::string shaderName{"-- This is no Shader --"};
    GLuint programID{ 0 };
};
