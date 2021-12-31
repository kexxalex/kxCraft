#include "ShaderManager.hpp"
#include <iostream>


Shader& ShaderManager::getShader(std::string_view name) noexcept {
    if (shaderNameIndex.find(name) == shaderNameIndex.end()) {
        bool isOK(false);
        unsigned int id(shaders.size());

        shaders.emplace_back(name, isOK);
        shaderNameIndex[name] = id;

        std::cout << "[  INFO  ][Shader ]  |- Shader status: " << id << " -- " << (isOK ? "(OK)" : "(XX)") << std::endl;
    }
    return shaders[shaderNameIndex.at(name)];
}

void ShaderManager::reloadAll() {
    for (auto &shader: shaders)
        shader.Reload();
}

ShaderManager::~ShaderManager() {
    shaders.clear();
    shaderNameIndex.clear();
}
