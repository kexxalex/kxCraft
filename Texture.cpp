#include <GL/glew.h>
#include "Texture.hpp"

Texture::Texture(unsigned int ID, const char *data,
                 int width, int height, unsigned bytesPerPixel,
                 bool interpolation, bool mipmaps, int anisotropy)

        : m_ID(ID) {
    Bind();

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


    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, interpolation ? GL_LINEAR : GL_NEAREST);

    if (mipmaps) {
        //int levels = static_cast<int>(log2(max(width, height)));
        //glTextureStorage2D(GL_TEXTURE_2D, levels, internalFormat, width, height);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, anisotropy);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                        interpolation ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_LINEAR);
    } else {
        //glTexStorage2D(GL_TEXTURE_2D, 1, internalFormat, width, height);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, interpolation ? GL_LINEAR : GL_NEAREST);
    }

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0,
                 bytesPerPixel == 3 ? GL_BGR : GL_BGRA, GL_UNSIGNED_BYTE, data);

    if (mipmaps)
        glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::Texture(unsigned int ID, const std::unique_ptr<BMP> &bitmap, bool interpolation, bool mipmaps, int anisotropy)
        : Texture(ID, bitmap->getData(),
                  bitmap->getWidth(), bitmap->getHeight(), bitmap->getChannelCount(),
                  interpolation, mipmaps, anisotropy) {}

void Texture::Bind() const {
    glBindTexture(GL_TEXTURE_2D, m_ID);
}

