#include <GL/glew.h>
#include "Texture.hpp"
#include <glm/glm.hpp>

Texture::Texture(unsigned int ID, const char *data,
                 int width, int height, unsigned bytesPerPixel,
                 bool interpolation, bool mipmaps, int anisotropy)

        : m_ID(ID) {
    GLenum internalFormat;
    switch (bytesPerPixel) {
        case 1:
            internalFormat = GL_R8;
            break;
        case 2:
            internalFormat = GL_RG8;
            break;
        case 3:
            internalFormat = GL_RGB8;
            break;
        case 4:
            internalFormat = GL_RGBA8;
            break;
        default:
            internalFormat = GL_RGB8;
    }


    glTextureParameteri(ID, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(ID, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTextureParameteri(ID, GL_TEXTURE_MAG_FILTER, interpolation ? GL_LINEAR : GL_NEAREST);

    if (mipmaps) {
        int levels = static_cast<int>(glm::log2(static_cast<float>((width > height) ? width : height)));
        glTextureStorage2D(ID, levels, internalFormat, width, height);
        glTextureParameteri(ID, GL_TEXTURE_MAX_ANISOTROPY, anisotropy);
        glTextureParameteri(ID, GL_TEXTURE_MIN_FILTER,
                            interpolation ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_LINEAR);

        glTextureSubImage2D(ID, 0,
                            0, 0, width, height,
                            bytesPerPixel == 3 ? GL_BGR : GL_BGRA, GL_UNSIGNED_BYTE, data);
        glGenerateTextureMipmap(ID);
    } else {
        glTextureStorage2D(ID, 1, internalFormat, width, height);
        glTextureParameteri(ID, GL_TEXTURE_MIN_FILTER, interpolation ? GL_LINEAR : GL_NEAREST);
        glTextureSubImage2D(ID, 0,
                            0, 0, width, height,
                            bytesPerPixel == 3 ? GL_BGR : GL_BGRA, GL_UNSIGNED_BYTE, data);
    }

}

Texture::Texture(unsigned int ID, const std::unique_ptr<BMP> &bitmap, bool interpolation, bool mipmaps, int anisotropy)
        : Texture(ID, bitmap->getData(),
                  bitmap->getWidth(), bitmap->getHeight(), bitmap->getChannelCount(),
                  interpolation, mipmaps, anisotropy) {}

void Texture::BindTo(int unit) const {
    glBindTextureUnit(unit, m_ID);
}

