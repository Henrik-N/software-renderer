#include "_types.h"
#include "_io.h"
#include "_tga.h"

#include <cassert>
#include <filesystem>


// =====================================================================================================================
// == Texture ==========================================================================================================
// =====================================================================================================================

bool Texture::load_from_tga(Texture& texture, const std::string_view filename) {
    std::vector<u8> tga_data;
    if (!io::read_binary(io::find_full_asset_path(filename), tga_data)) {
        return false;
    }

    const tga::Header_View header{tga_data};
    assert(header.get_color_map_type() != tga::Color_Map_Type::Present);
    assert(header.image.bits_per_pixel == 24 || header.image.bits_per_pixel == 32);

    const usize bytes_per_pixel = header.image.bits_per_pixel / 8;
    const usize num_pixels = header.num_pixels();

    texture.pixels.resize(num_pixels);

    // TODO:
    assert(header.is_positive_x_right());
    assert(header.is_positive_y_down());

    const std::span<u8> src = {&tga_data[header.offset_image()], header.packed_byte_size_image()};
    const std::span<u8> dst = {reinterpret_cast<u8*>(&texture.pixels[0]), header.packed_byte_size_image()};

    for (usize px_idx = 0; px_idx < num_pixels; ++px_idx) {
        for (usize offset = 0; offset < bytes_per_pixel; ++offset) {
            dst[px_idx * sizeof(Color) + offset] = src[px_idx * bytes_per_pixel + offset];
        }
    }

    texture.width = header.image.pixel_width;
    texture.height = header.image.pixel_height;

    return true;
}
