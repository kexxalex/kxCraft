#pragma once


#include "BMP.h"
#include <memory>

class Texture {
public:
    Texture(unsigned int ID, const char *data, int width, int height, unsigned bytesPerPixel, bool interpolation,
            bool mipmaps, int anisotropy);

    Texture(unsigned int ID, const std::unique_ptr<BMP> &bitmap, bool interpolation, bool mipmaps, int anisotropy);

    void Bind() const;

private:
    const unsigned int m_ID;
};
