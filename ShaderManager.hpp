#pragma once


#include "Shader.hpp"
#include <vector>


class ShaderManager {
public:
    void initialize() {
        bool isOK;
        m_default = Shader("./res/shader/default", isOK);
    }

    ShaderManager() = default;
    ShaderManager(const ShaderManager &shaderManager) = default;
    ~ShaderManager();

    void reloadAll();

    [[nodiscard]] Shader &getShader(std::string_view name) noexcept;
    [[nodiscard]] Shader &getDefault() noexcept { return m_default; }

private:
    Shader m_default;
    std::vector<Shader> shaders;
    std::unordered_map<std::string_view, unsigned int> shaderNameIndex;
};
