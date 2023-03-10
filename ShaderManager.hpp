#pragma once


#include "Shader.hpp"
#include <vector>


class ShaderManager {
public:
    static ShaderManager &create() {
        static ShaderManager manager;
        return manager;
    }

    ShaderManager(const ShaderManager &) = default;

    void initialize() {
        bool isOK;
        m_default = Shader("./res/shader/default", isOK);
    }

    ~ShaderManager();

    void reloadAll();

    [[nodiscard]] Shader &getShader(std::string_view name) noexcept;
    [[nodiscard]] Shader &getDefault() noexcept { return m_default; }

private:
    ShaderManager() = default;

    Shader m_default;
    std::vector<Shader> shaders;
    std::unordered_map<std::string_view, unsigned int> shaderNameIndex;
};

extern ShaderManager SHADER_MANAGER;
