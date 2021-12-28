#pragma once


#include <string>

#pragma pack(push, 1)
struct st_BMP_HEADER {
    unsigned char bfType[2]{0, 0};
    unsigned int bfSize{0};
    unsigned int bfReserved{0};
    unsigned int bfOffBytes{0};
};

struct st_BMP_INFO_HEADER {
    unsigned int biSize{0};
    int biWidth{0};
    int biHeight{0};
    unsigned short biPlanes{0};
    unsigned short biBitCount{0};
    unsigned int biCompression{0};
    unsigned int biSizeImage{0};
    int biXPixelsPerMeter{0};
    int biYPixelsPerMeter{0};
    unsigned int biClrUsed{0};
    unsigned int biClrImportant{0};
};
#pragma pack(pop)


class BMP {
public:
    BMP(const char *name, bool &isOK);

    ~BMP();

    [[nodiscard]] const char *getData() const noexcept;

    [[nodiscard]] unsigned int getChannelCount() const noexcept { return info.biBitCount / 8; }

    [[nodiscard]] unsigned int getWidth() const noexcept { return info.biWidth; }

    [[nodiscard]] unsigned int getHeight() const noexcept { return info.biHeight; }

    [[nodiscard]] bool isBitmap() const noexcept { return data != nullptr; }

private:
    const char *name{nullptr};
    struct st_BMP_HEADER header{};
    struct st_BMP_INFO_HEADER info{};
    char *data{nullptr};
};
