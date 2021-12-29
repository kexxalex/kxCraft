#include "BMP.hpp"
#include <fstream>

static constexpr unsigned char BMP_CHECK[]{'B', 'M'};


BMP::BMP(const char *n, bool &isOK)
    : name(n)
{
    isOK = false;

    std::ifstream bmp_file(name, std::ios::binary);
    if (bmp_file) {
        bmp_file.read(reinterpret_cast<char *>(&header), 14);

        if (header.bfType[0] != BMP_CHECK[0] || header.bfType[1] != BMP_CHECK[1]) {
            bmp_file.close();
            return;
        }

        if (header.bfOffBytes == 0)
            header.bfOffBytes = 54;
        bmp_file.read(reinterpret_cast<char *>(&info), 40);

        data = new char[info.biSizeImage]; // allocate memory for image data

        unsigned int seekg = info.biSize + 14 + bmp_file.beg; // index of image data
        bmp_file.seekg(seekg);
        bmp_file.read(data, info.biSizeImage);
        isOK = true;
    }
    bmp_file.close();
}

BMP::~BMP() { delete[] data; }

const char *BMP::getData() const noexcept {
    if (!isBitmap()) {
        return nullptr;
    }

    return data;
}
