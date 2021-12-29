#pragma once


#include "Shader.hpp"
#include <vector>
#include <memory>


class ShaderManager {
public:
    ~ShaderManager();

    void initialize();
    void reloadAll();

    std::shared_ptr<Shader> getShader(const char *name) noexcept;
    std::shared_ptr<Shader> &getDefault() noexcept { return m_default; }

private:
    std::shared_ptr<Shader> m_default{nullptr};
    std::vector<std::shared_ptr<Shader>> shaders = {};
    std::unordered_map<const char *, unsigned int> shaderHash;
};
