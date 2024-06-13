#pragma once
#include "_color.h"


struct Texture {
    std::vector<Color> pixels;
    i32 width;
    i32 height;

    static bool load_from_tga(Texture& texture, std::string_view filename);
};
