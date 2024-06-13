#pragma once


#include <span>


namespace tga { 


enum class Image_Type : u8 {
    No_Image_Data = 0,
    Uncompressed_Color_Map_Indexed = 1,
    Uncompressed_Rgb = 2,
    Uncompressed_Bitmap = 3,
    Runlength_Encoded_Color_Map_Indexed = 9,
    Runlength_Encoded_Rgb = 10,
    Runlenght_Encoded_Bitmap = 11
};
std::string_view to_string(Image_Type image_type);


enum class Color_Map_Type : u8 {
    None = 0,
    Present = 1,
};
std::string_view to_string(Color_Map_Type image_type);


struct Header_View {
    u8& image_id_length;
    u8& color_map_type;
    u8& image_type;


    struct Color_Map_Spec {
        u16& first_entry_index;
        u16& num_entries;
        u8& entry_bit_size;
    } color_map;


    struct Image_Spec {
        u16& x_origin;
        u16& y_origin;
        u16& pixel_width;
        u16& pixel_height;
        u8& bits_per_pixel; // aka "pixel depth"
        u8& descriptor; // bits 3-0: alpha channel depth, bits 5-4: pixel ordering
    } image;


    explicit Header_View(std::span<u8> data);


    consteval static usize packed_byte_size_header()                       { return 18; } // Total byte size of the entire header in the buffer
    usize                  packed_byte_size_image_id() const               { return image_id_length; }
    usize                  packed_byte_size_color_map() const              { return color_map.num_entries * (color_map.entry_bit_size / 8); }
    usize                  packed_byte_size_image() const                  { return image.pixel_width * image.pixel_height * (image.bits_per_pixel / 8); }

    consteval static usize offset_image_id()                               { return packed_byte_size_header(); }
    usize                  offset_color_map() const                        { return offset_image_id() + packed_byte_size_image_id(); }
    usize                  offset_image() const                            { return offset_color_map() + packed_byte_size_color_map(); }

    Color_Map_Type         get_color_map_type() const                      { return static_cast<Color_Map_Type>(color_map_type); }
    Image_Type             get_image_type() const                          { return static_cast<Image_Type>(image_type); }

    bool                   is_positive_x_right() const                     { return !(image.descriptor & 0x10); };
    bool                   is_positive_y_down() const                      { return image.descriptor & 0x20; };

    bool                   has_alpha() const                               { return image.bits_per_pixel == 32 || image.bits_per_pixel == 16; }
    usize                  bytes_per_pixel() const                         { return image.bits_per_pixel / 8; }
    usize                  num_pixels() const                              { return packed_byte_size_image() / bytes_per_pixel(); }

    friend std::string      to_string(const Header_View& header);
};


} // namespace tga
