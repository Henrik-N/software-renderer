#include "_tga.h"
#include "_common.h"

#include <sstream>

namespace tga
{


std::string_view to_string(const Image_Type image_type) {
    switch (image_type) {
        using enum Image_Type;
        ENUM_CASE_RETURN_AS_STRING(No_Image_Data);
        ENUM_CASE_RETURN_AS_STRING(Uncompressed_Color_Map_Indexed);
        ENUM_CASE_RETURN_AS_STRING(Uncompressed_Rgb);
        ENUM_CASE_RETURN_AS_STRING(Uncompressed_Bitmap);
        ENUM_CASE_RETURN_AS_STRING(Runlength_Encoded_Color_Map_Indexed);
        ENUM_CASE_RETURN_AS_STRING(Runlength_Encoded_Rgb);
        ENUM_CASE_RETURN_AS_STRING(Runlenght_Encoded_Bitmap);
        default: return "Unknown";
    }
}


std::string_view to_string(const Color_Map_Type color_map_type) {
    switch (color_map_type) {
        using enum Color_Map_Type;
        ENUM_CASE_RETURN_AS_STRING(None);
        ENUM_CASE_RETURN_AS_STRING(Present);
        default: return "Unknown";
    }
}


Header_View::Header_View(std::span<u8> data):
        image_id_length(                                                data[0]),
        color_map_type(                                                 data[sizeof(u8)] ),
        image_type(                                                     data[sizeof(u8) * 2]),
        color_map({
            .first_entry_index =               *reinterpret_cast<u16*>(&data[sizeof(u8) * 3]),
            .num_entries =                     *reinterpret_cast<u16*>(&data[sizeof(u8) * 3 + sizeof(u16)]),
            .entry_bit_size =                                           data[sizeof(u8) * 3 + sizeof(u16) * 2],
            // NOTE: Invisble u8 padding due to alignment if you would have tried to read this entire struct size as a chunk of data instead of using references. 
            //       The image file header in the binary is completely packed and this padding doesn't exist there.
         }),
         image({
            .x_origin =                        *reinterpret_cast<u16*>(&data[sizeof(u8) * 4 + sizeof(u16) * 2]),
            .y_origin =                        *reinterpret_cast<u16*>(&data[sizeof(u8) * 4 + sizeof(u16) * 3]),
            .pixel_width =                     *reinterpret_cast<u16*>(&data[sizeof(u8) * 4 + sizeof(u16) * 4]),
            .pixel_height =                    *reinterpret_cast<u16*>(&data[sizeof(u8) * 4 + sizeof(u16) * 5]),
            .bits_per_pixel =                                           data[sizeof(u8) * 4 + sizeof(u16) * 6],
            .descriptor =                                               data[sizeof(u8) * 5 + sizeof(u16) * 6],
         }) {
}


std::string to_string(const Header_View& header) {
    std::stringstream str;
    str << 
        "\n\timage_id_length: "         << static_cast<u32>(header.image_id_length) <<
        "\n\tcolor_map_type: "          << to_string(header.get_color_map_type()) <<
        "\n\timage_type: "              << to_string(header.get_image_type()) <<
        //
        "\n\tcolor_map_spec: "          << 
        "\n\t\tfirst_entry_index: "     << header.color_map.first_entry_index << 
        "\n\t\tnum_entries: "           << header.color_map.num_entries << 
        "\n\t\tentry_bit_size: "        << static_cast<u32>(header.color_map.entry_bit_size) <<
        //
        "\n\timage_spec: "              << 
        "\n\t\tx_origin: "              << header.image.x_origin << 
        "\n\t\ty_origin: "              << header.image.y_origin << 
        "\n\t\tpixel_width: "           << header.image.pixel_width << 
        "\n\t\tpixel_height: "          << header.image.pixel_height << 
        "\n\t\tbits_per_pixel: "        << static_cast<u32>(header.image.bits_per_pixel) <<
        "\n\t\timage_descriptor: "      << static_cast<u32>(header.image.descriptor) <<
        "\n\t\t\tis_positive_x_right: " << (header.is_positive_x_right() ? "yes" : "no") <<
        "\n\t\t\tis_positive_y_down: "  << (header.is_positive_y_down() ? "yes" : "no");
    return str.str();
}


}
