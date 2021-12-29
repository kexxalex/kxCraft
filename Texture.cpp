#include <GL/glew.h>
#include "Texture.hpp"

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

    glBindTexture(GL_TEXTURE_2D, ID);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0,
                 bytesPerPixel == 3 ? GL_BGR : GL_BGRA, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);


    glTextureParameteri(ID, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(ID, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTextureParameteri(ID, GL_TEXTURE_MAG_FILTER, interpolation ? GL_LINEAR : GL_NEAREST);

    if (mipmaps) {
        // glTextureParameteri(ID, GL_TEXTURE_MAX_ANISOTROPY, anisotropy);
        glTextureParameteri(ID, GL_TEXTURE_MAX_ANISOTROPY, anisotropy);
        glTextureParameteri(ID, GL_TEXTURE_MIN_FILTER,
                        interpolation ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_LINEAR);
    } else {
        glTexStorage2D(GL_TEXTURE_2D, 1, internalFormat, width, height);
        glTextureParameteri(ID, GL_TEXTURE_MIN_FILTER, interpolation ? GL_LINEAR : GL_NEAREST);
    }


    if (mipmaps)
        glGenerateTextureMipmap(ID);
}

Texture::Texture(unsigned int ID, const std::unique_ptr<BMP> &bitmap, bool interpolation, bool mipmaps, int anisotropy)
        : Texture(ID, bitmap->getData(),
                  bitmap->getWidth(), bitmap->getHeight(), bitmap->getChannelCount(),
                  interpolation, mipmaps, anisotropy) {}

void Texture::BindTo(int unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, m_ID);
}

