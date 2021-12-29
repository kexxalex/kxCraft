#include "ShaderManager.hpp"
#include <iostream>


void ShaderManager::initialize() {
    m_default = getShader("./res/shader/default");
}

std::shared_ptr<Shader> ShaderManager::getShader(const char *name) noexcept {
    if (shaderHash.find(name) == shaderHash.end()) {
        bool isOK(false);
        unsigned int id(shaders.size());

        shaders.emplace_back(std::make_shared<Shader>(name, isOK));
        shaderHash[name] = id;

        std::cout << "[  INFO  ][Shader ]  |- Shader status: " << id << " -- " << (isOK ? "(OK)" : "(XX)") << std::endl;
    }
    return shaders[shaderHash[name]];
}

void ShaderManager::reloadAll() {
    for (auto &shader: shaders)
        shader->Reload();
}

ShaderManager::~ShaderManager() {
    shaders.clear();
    shaderHash.clear();
}
