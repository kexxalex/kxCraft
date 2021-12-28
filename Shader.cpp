#include "Shader.hpp"
#include <fstream>
#include <sstream>
#include <iostream>


bool loadShaderProgram(const std::string &&filename, GLint shaderType, GLuint &shaderID) {
    std::string sourceCode;
    std::ifstream shaderFile(filename);

    if (shaderFile) {
        std::stringstream sourceStringStream;
        sourceStringStream << shaderFile.rdbuf();

        shaderFile.close();

        sourceCode = sourceStringStream.str();
    } else {
        std::cout << "[WARNING ][Shader ] Cannot open shader: " << filename << std::endl;
        return false;
    }

    int success;
    char infoLog[1024];

    const char *source_cptr = sourceCode.c_str();

    shaderID = glCreateShader(shaderType);
    glShaderSource(shaderID, 1, &source_cptr, NULL);
    glCompileShader(shaderID);

    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shaderID, 1024, NULL, infoLog);
        std::cout << "[ ERROR  ][Shader ]\n" << infoLog << std::endl;
        return false;
    }

    return true;
}


Shader::Shader(const char *shaderpath, bool &isOK)
        : shaderName(shaderpath) {
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
    if (!loadShaderProgram(shaderName + ".vs", GL_VERTEX_SHADER, vertexID))
        return false;
    if (!loadShaderProgram(shaderName + ".fs", GL_FRAGMENT_SHADER, fragmentID))
        return false;
    bool hasGeometry = loadShaderProgram(shaderName + ".gs", GL_GEOMETRY_SHADER, geometryID);

    programID = glCreateProgram();
    glAttachShader(programID, vertexID);
    glAttachShader(programID, fragmentID);
    if (hasGeometry)
        glAttachShader(programID, geometryID);
    glLinkProgram(programID);

    glDeleteShader(vertexID);
    glDeleteShader(fragmentID);
    if (hasGeometry)
        glDeleteShader(geometryID);

    int success;
    char infoLog[1024];
    glGetProgramiv(programID, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(programID, 1024, NULL, infoLog);
        std::cout << "[ ERROR  ][Shader ] Link:\n" << infoLog << std::endl;
        return false;
    }

    return true;
}

void Shader::setBool(const char *name, bool value) noexcept {
    if (uniforms.find(name) == uniforms.end())
        uniforms[name] = glGetUniformLocation(programID, name);

    glUniform1i(uniforms[name], (int) value);
}

void Shader::setInt(const char *name, int value) noexcept {
    if (uniforms.find(name) == uniforms.end())
        uniforms[name] = glGetUniformLocation(programID, name);

    glUniform1i(uniforms[name], value);
}

void Shader::setFloat(const char *name, float value) noexcept {
    if (uniforms.find(name) == uniforms.end())
        uniforms[name] = glGetUniformLocation(programID, name);

    glUniform1f(uniforms[name], value);
}

void Shader::setDouble(const char *name, double value) noexcept {
    if (uniforms.find(name) == uniforms.end())
        uniforms[name] = glGetUniformLocation(programID, name);

    glUniform1d(uniforms[name], value);
}

void Shader::setFloat3(const char *name, const glm::fvec3 &value) noexcept {
    if (uniforms.find(name) == uniforms.end())
        uniforms[name] = glGetUniformLocation(programID, name);

    glUniform3fv(uniforms[name], 1, &value.x);
}

void Shader::setDouble3(const char *name, const glm::dvec3 &value) noexcept {
    if (uniforms.find(name) == uniforms.end())
        uniforms[name] = glGetUniformLocation(programID, name);

    glUniform3dv(uniforms[name], 1, &value.x);
}

void Shader::setMatrixFloat4(const char *name, const glm::fmat4 &matrix) noexcept {
    if (uniforms.find(name) == uniforms.end())
        uniforms[name] = glGetUniformLocation(programID, name);

    glUniformMatrix4fv(uniforms[name], 1, GL_FALSE, &matrix[0].x);
}

Shader::~Shader() {
    std::cout << "[  INFO  ][Shader ] Delete Shader: " << shaderName << '(' << programID << ')' << std::endl;

    Release();
    glDeleteProgram(programID);
    uniforms.clear();
}
