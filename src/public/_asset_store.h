#pragma once
#include "_common.h"
#include "_ecs.h"
#include "_math.h"
#include "_types.h"


struct Asset_Store_System final : System {
    explicit Asset_Store_System() = default;

    Mesh_View access_mesh_data(Mesh_Id id) const;
    Texture_View access_texture_data(Texture_Id id) const;

    Mesh_Id load_mesh_asset(std::string_view unique_mesh_name, std::string_view filename);
    Texture_Id load_texture_asset(std::string_view unique_texture_name, std::string_view filename);

    Mesh_Id get_mesh_id(std::string_view unique_mesh_name) const;
    Texture_Id get_texture_id(std::string_view unique_texture_name) const;

private:
    // Mesh
    std::vector<std::string> mesh_names;
    std::vector<Mesh_View> mesh_views;
    std::vector<Vec3> vertices;
    std::vector<Face_Vertex_Indices> faces;

    bool load_obj_mesh(std::string_view filename);

    // Texture
    std::vector<std::string> texture_names;
    std::vector<Texture_View> texture_views;
    std::vector<Color> texture_pixels;

    bool load_tga_texture(std::string_view filename, Vec2i& dimensions);
};
