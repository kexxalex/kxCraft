#pragma once


#include <unordered_map>
#include <memory>
#include <vector>
#include "Texture.hpp"

class TextureManager {
public:
    static TextureManager &create() {
        static TextureManager manager;
        return manager;
    }

    TextureManager(const TextureManager &) = default;

    ~TextureManager();

    void initialize(unsigned int texture_count = 32);

    void clear();

    unsigned int getID(const std::shared_ptr<Texture> &texture);

    std::shared_ptr<Texture> getTexture(unsigned int id) const;

    std::shared_ptr<Texture> loadTexture(const char *name, bool interpolation = false, bool mipmaps = false, int anisotropy=16);

private:
    TextureManager() = default;

    std::shared_ptr<Texture> m_default{nullptr};
    std::unordered_map<unsigned int, std::shared_ptr<Texture>> m_texture_id;
    std::unordered_map<std::shared_ptr<Texture>, unsigned int> m_id_texture;

    std::vector<unsigned int> m_free_ids;
};
