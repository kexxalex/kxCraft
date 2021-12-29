#include "TextureManager.hpp"
#include <GL/glew.h>
#include <iostream>


TextureManager::~TextureManager() {
    clear();
}

void TextureManager::initialize(unsigned int texture_count) {
    clear();
    m_free_ids = std::vector<unsigned int>(texture_count);

    glGenTextures(texture_count, &m_free_ids[0]);
}

unsigned int TextureManager::getID(const std::shared_ptr<Texture> &texture) {
    if (nullptr == texture || m_id_texture.find(texture) == m_id_texture.end())
        return 0;

    return m_id_texture[texture];
}

std::shared_ptr<Texture> TextureManager::getTexture(unsigned int id) const {
    if (m_texture_id.find(id) == m_texture_id.end())
        return nullptr;

    return m_texture_id.at(id);
}

void TextureManager::clear() {
    for (const auto &pair: m_texture_id) {
        m_free_ids.emplace_back(pair.first);
    }

    m_texture_id.clear();
    m_id_texture.clear();

    if (!m_free_ids.empty()) {
        glDeleteTextures(static_cast<int>(m_free_ids.size()), &m_free_ids[0]);
        m_free_ids.clear();
        m_free_ids.shrink_to_fit();
    }
}

std::shared_ptr<Texture> TextureManager::loadTexture(const char *name, bool interpolation, bool mipmaps, int anisotropy) {
    unsigned int ID = m_free_ids.back();

    bool isOK = true;
    std::unique_ptr<BMP> bmp = std::make_unique<BMP>(name, isOK);

    std::cout << "[  INFO  ][Texture] New: " << name << " (" << bmp->getWidth() << ", " << bmp->getHeight()
              << ") - status: " << ID * isOK << std::endl;
    if (!isOK) {
        return nullptr;
    }

    std::shared_ptr<Texture> texture = std::make_shared<Texture>(ID, bmp, interpolation, mipmaps, anisotropy);

    m_id_texture[texture] = ID;
    m_texture_id[ID] = texture;

    m_free_ids.pop_back();
    return texture;
}
