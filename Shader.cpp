#include "Shader.hpp"
#include <fstream>
#include <sstream>
#include <iostream>


bool loadShaderProgram(const std::string&& filename, GLint shaderType, GLuint &shaderID) {
    std::string sourceCode;
    std::ifstream shaderFile(filename);

    if (shaderFile) {
        std::stringstream sourceStringStream;
        sourceStringStream << shaderFile.rdbuf();

        shaderFile.close();

        sourceCode = sourceStringStream.str();
    } else {
        std::cout << "[WARNING ][Shader ] File does not exist: " << filename << std::endl;
        return false;
    }

    int success;
    char infoLog[1024];

    const char *source_cptr = sourceCode.c_str();

    shaderID = glCreateShader(shaderType);
    glShaderSource(shaderID, 1, &source_cptr, nullptr);
    glCompileShader(shaderID);

    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shaderID, 1024, nullptr, infoLog);
        std::cout << "[ ERROR  ][Shader ] Error in: " << filename << std::endl << infoLog << std::endl;
        return false;
    }

    return true;
}


Shader::Shader(std::string_view shaderPath, bool &isOK)
    : shaderName(shaderPath)
{
    isOK = Load();
}

bool Shader::Reload() noexcept {
    if (programID != 0) {
        glUseProgram(0);
        glDeleteProgram(programID);
    }

    uniforms.clear();
    return Load();
}

bool Shader::Load() noexcept {
    std::cout << "[  INFO  ][Shader ] Create Shader: " << shaderName << std::endl;

    GLuint vertexID(0), fragmentID(0), geometryID(0);
    bool hasVertex   = loadShaderProgram(shaderName + ".vs", GL_VERTEX_SHADER, vertexID);
    bool hasFragment = loadShaderProgram(shaderName + ".fs", GL_FRAGMENT_SHADER, fragmentID);
    bool hasGeometry = loadShaderProgram(shaderName + ".gs", GL_GEOMETRY_SHADER, geometryID);

    // no shader source found
    if (!hasVertex && !hasFragment && !hasGeometry)
        return false;

    // Create and Link the Shader Program, attach and delete available Shaders
    programID = glCreateProgram();

    if (hasVertex)
        glAttachShader(programID, vertexID);
    if (hasFragment)
        glAttachShader(programID, fragmentID);
    if (hasGeometry)
        glAttachShader(programID, geometryID);

    glLinkProgram(programID);

    if (hasVertex)
        glDeleteShader(vertexID);
    if (hasFragment)
        glDeleteShader(fragmentID);
    if (hasGeometry)
        glDeleteShader(geometryID);

    int success;
    char infoLog[1024];
    glGetProgramiv(programID, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(programID, 1024, nullptr, infoLog);
        std::cout << "[ ERROR  ][Shader ] Link:" << std::endl << infoLog << std::endl;
        return false;
    }

    return true;
}

void Shader::setBool(std::string_view name, bool value) noexcept {
    if (uniforms.find(name) == uniforms.end())
        uniforms[name] = glGetUniformLocation(programID, name.data());

    glUniform1i(uniforms[name], (int) value);
}

void Shader::setInt(std::string_view name, int value) noexcept {
    if (uniforms.find(name) == uniforms.end())
        uniforms[name] = glGetUniformLocation(programID, name.data());

    glUniform1i(uniforms[name], value);
}

void Shader::setFloat(std::string_view name, float value) noexcept {
    if (uniforms.find(name) == uniforms.end())
        uniforms[name] = glGetUniformLocation(programID, name.data());

    glUniform1f(uniforms[name], value);
}

void Shader::setDouble(std::string_view name, double value) noexcept {
    if (uniforms.find(name) == uniforms.end())
        uniforms[name] = glGetUniformLocation(programID, name.data());

    glUniform1d(uniforms[name], value);
}

void Shader::setFloat3(std::string_view name, const glm::fvec3 &value) noexcept {
    if (uniforms.find(name) == uniforms.end())
        uniforms[name] = glGetUniformLocation(programID, name.data());

    glUniform3fv(uniforms[name], 1, &value.x);
}

void Shader::setDouble3(std::string_view name, const glm::dvec3 &value) noexcept {
    if (uniforms.find(name) == uniforms.end())
        uniforms[name] = glGetUniformLocation(programID, name.data());

    glUniform3dv(uniforms[name], 1, &value.x);
}

void Shader::setMatrixFloat4(std::string_view name, const glm::fmat4 &matrix) noexcept {
    if (uniforms.find(name) == uniforms.end())
        uniforms[name] = glGetUniformLocation(programID, name.data());

    glUniformMatrix4fv(uniforms[name], 1, GL_FALSE, &matrix[0].x);
}

Shader::~Shader() {
    std::cout << "[  INFO  ][Shader ] Delete Shader: " << shaderName << '(' << programID << ')' << std::endl;

    glUseProgram(0);
    glDeleteProgram(programID);
    uniforms.clear();
}
