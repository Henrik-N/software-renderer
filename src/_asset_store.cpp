#include "_asset_store.h"

#include "_io.h"
#include "_tga.h"

#include <fstream>
#include <filesystem>


namespace fs = std::filesystem;


Mesh_View Asset_Store_System::access_mesh_data(const Mesh_Id id) const {
    return mesh_views[id.id];
}


Texture_View Asset_Store_System::access_texture_data(const Texture_Id id) const {
    return texture_views[id.id];
}


Mesh_Id Asset_Store_System::load_mesh_asset(const std::string_view unique_mesh_name, const std::string_view filename) {
#if _DEBUG
    for (const std::string_view mesh_name : mesh_names) {
        assert(mesh_name != unique_mesh_name);
    }
#endif

    const usize vertices_start = vertices.size();
    const usize faces_start = faces.size();

    const bool load_success = load_obj_mesh(filename);
    assert(load_success);


    mesh_names.emplace_back() = unique_mesh_name;

    mesh_views.emplace_back() = Mesh_View{
        .vertices = std::span{&vertices[vertices_start], vertices.size() - vertices_start},
        .faces = std::span{&faces[faces_start], faces.size() - faces_start},
    };

    const Mesh_Id mesh_id{
        .id = mesh_views.size() - 1,
    };

    return mesh_id;
}


Texture_Id Asset_Store_System::load_texture_asset(const std::string_view unique_texture_name,
                                                  const std::string_view filename) {
#if _DEBUG
    for (const std::string_view texture_name : texture_names) {
        assert(texture_name != unique_texture_name);
    }
#endif

    const usize pixel_start = texture_pixels.size();

    Vec2i dimensions;
    const bool load_success = load_tga_texture(filename, dimensions);
    assert(load_success);


    texture_names.emplace_back() = unique_texture_name;

    texture_views.emplace_back() = Texture_View{
        .pixels = std::span{&texture_pixels[pixel_start], texture_pixels.size() - pixel_start},
        .width = dimensions.width,
        .height = dimensions.height
    };

    const Texture_Id texture_id{
        .id = texture_views.size() - 1,
    };

    return texture_id;
}


Mesh_Id Asset_Store_System::get_mesh_id(const std::string_view unique_mesh_name) const {
    for (usize idx = 0; idx < mesh_names.size(); ++idx) {
        if (mesh_names[idx] == unique_mesh_name) {
            return Mesh_Id{ .id = idx };
        }
    }
    assert(false);
    return Mesh_Id{};
}


Texture_Id Asset_Store_System::get_texture_id(const std::string_view unique_texture_name) const {
    for (usize idx = 0; idx < texture_names.size(); ++idx) {
        if (texture_names[idx] == unique_texture_name) {
            return Texture_Id{ .id = idx };
        }
    }
    assert(false);
    return Texture_Id{};
}


bool Asset_Store_System::load_obj_mesh(const std::string_view filename) {
    if (filename.empty()) {
        ERR("filename nullptr");
        return false;
    }

    const fs::path asset_path = io::find_full_asset_path(filename);

    std::fstream file{};
    {
        file.open(asset_path, std::ios::in);
        if (file.fail()) {
            ERR("failed to open file " << asset_path);
            return false;
        }
    }


    std::string line;
    std::string buffer;
    line.reserve(1024);
    buffer.reserve(30);


    while (!file.eof()) {
        std::getline(file, line);
        std::istringstream line_stream{line};
        line_stream >> buffer;

        if (buffer == "v") {
            Vec3 vertex{};
            line_stream >> vertex.x >> vertex.y >> vertex.z;

            vertices.emplace_back(vertex);
            continue;
        }

        if (buffer == "f") {
            auto next_number_before_slash = [&]() -> i32 {
                line_stream >> buffer;
                std::istringstream temp_stream{buffer};
                std::getline(temp_stream, buffer, '/');
                // TODO: Maybe use something that doesn't throw here in case the obj-file has an invalid format.
                return std::stoi(buffer);
            };

            Face_Vertex_Indices face_vertex_indices;
            face_vertex_indices[0] = next_number_before_slash() - 1; // obj files start vertices at index 1
            face_vertex_indices[1] = next_number_before_slash() - 1;
            face_vertex_indices[2] = next_number_before_slash() - 1;

            faces.emplace_back(face_vertex_indices);
            continue;
        }
    }

    file.close();

    return true;
}


bool Asset_Store_System::load_tga_texture(const std::string_view filename, Vec2i& dimensions) {
    std::vector<u8> tga_data;
    if (!io::read_binary(io::find_full_asset_path(filename), tga_data)) {
        return false;
    }

    const tga::Header_View header{tga_data};
    assert(header.get_color_map_type() != tga::Color_Map_Type::Present);
    assert(header.image.bits_per_pixel == 24 || header.image.bits_per_pixel == 32);

    const usize bytes_per_pixel = header.image.bits_per_pixel / 8;
    const usize num_pixels = header.num_pixels();

    // TODO:
    assert(header.is_positive_x_right());
    assert(header.is_positive_y_down());


    const usize texture_pixels_start = texture_pixels.size();
    texture_pixels.resize(texture_pixels.size() + num_pixels, Color::black());


    const std::span<u8> src = {&tga_data[header.offset_image()], header.packed_byte_size_image()};
    const std::span<u8> dst = {reinterpret_cast<u8*>(&texture_pixels[texture_pixels_start]), sizeof(Color) * header.num_pixels()};

    for (usize px_idx = 0; px_idx < num_pixels; ++px_idx) {
        for (usize offset = 0; offset < bytes_per_pixel; ++offset) {
            dst[(px_idx * sizeof(Color)) + offset] = src[(px_idx * bytes_per_pixel) + offset];
        }
    }

    dimensions.width = header.image.pixel_width;
    dimensions.height = header.image.pixel_height;

    return true;
}

