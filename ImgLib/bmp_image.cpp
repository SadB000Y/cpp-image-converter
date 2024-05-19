#include "bmp_image.h"
#include "pack_defines.h"

#include <iostream>
#include <array>
#include <fstream>
#include <string_view>

using namespace std;

namespace img_lib {

    PACKED_STRUCT_BEGIN BitmapFileHeader
    {
        char id[2];
        uint32_t filesize;
        uint32_t unused;
        uint32_t offset;
    }
    PACKED_STRUCT_END

        PACKED_STRUCT_BEGIN BitmapInfoHeader
    {
        uint32_t size_dibheader;
        int32_t width;
        int32_t height;
        unsigned short number_of_plates;
        unsigned short bits_per_pixel;
        uint32_t pixel_compression;
        uint32_t bits_in_data;
        int32_t resolution_horizontal;
        int32_t resolution_vertical;
        int32_t used_color;
        int32_t important_color;
    }
    PACKED_STRUCT_END

        // функция вычисления отступа по ширине
        static int GetBMPStride(int w) {
        return 4 * ((w * 3 + 3) / 4);
    }

    // напишите эту функцию
    bool SaveBMP(const Path& file, const Image& image)
    {
        ofstream out(file, ios::binary);

        BitmapFileHeader file_h;
        BitmapInfoHeader info;
        int stride = GetBMPStride(image.GetWidth());

        file_h.id[0] = 'B'; file_h.id[1] = 'M';
        file_h.filesize = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader) +
            GetBMPStride(image.GetWidth()) * image.GetHeight();
        file_h.unused = 0;
        file_h.offset = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader);
        

        info.size_dibheader = sizeof(BitmapInfoHeader);
        info.width = image.GetWidth();
        info.height = image.GetHeight();
        info.number_of_plates = 1;
        info.bits_per_pixel = 24;
        info.pixel_compression = 0;
        info.bits_in_data = stride * image.GetHeight();
        info.resolution_horizontal = 11811;
        info.resolution_vertical = 11811;
        info.used_color = 0;
        info.important_color = 0x1000000;

        out.write(reinterpret_cast<char*>(&file_h), sizeof(file_h));
        out.write(reinterpret_cast<char*>(&info), sizeof(info));

        const int w = image.GetWidth();
        const int h = image.GetHeight();
        std::vector<char> buff(stride);

        for (int y = h - 1; y >= 0; --y) {
            const Color* line = image.GetLine(y);
            for (int x = 0; x < w; ++x) {
                buff[x * 3 + 0] = static_cast<char>(line[x].b);
                buff[x * 3 + 1] = static_cast<char>(line[x].g);
                buff[x * 3 + 2] = static_cast<char>(line[x].r);
            }
            out.write(buff.data(), stride);
        }

        return out.good();
    }

    Image LoadBMP(const Path& file)
    {
        ifstream ifs(file, ios::binary);
        BitmapFileHeader file_h;
        BitmapInfoHeader info;

        ifs.read(reinterpret_cast<char*>(&file_h), sizeof(file_h));
        ifs.read(reinterpret_cast<char*>(&info), sizeof(info));


        int stride = GetBMPStride(info.width);

        Image result(info.width, info.height, Color::Black());
        std::vector<char> buff(stride);

        for (int y = info.height - 1; y >= 0; --y) {
            Color* line = result.GetLine(y);
            ifs.read(buff.data(), stride);

            for (int x = 0; x < info.width; ++x) {
                line[x].b = static_cast<byte>(buff[x * 3 + 0]);
                line[x].g = static_cast<byte>(buff[x * 3 + 1]);
                line[x].r = static_cast<byte>(buff[x * 3 + 2]);
            }
        }

        return result;
    }
}  // namespace img_lib