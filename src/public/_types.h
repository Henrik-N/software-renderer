#pragma once
#include "_common.h"
#include "_color.h"
#include "_math.h"


struct Camera_System;
struct Render_System;
struct Window_System;


using Triangle = std::array<Vec2i, 3>;


struct Light {
    Vec3 direction;
};


struct Mesh {
    using Face_Vertex_Indices = std::array<u16, 3>;

    std::vector<Vec3> vertices;
    std::vector<Face_Vertex_Indices> faces;

    static bool load_from_obj(Mesh& mesh, std::string_view filename);

    void log();
};


struct Texture {
    std::vector<Color> pixels;
    i32 width;
    i32 height;

    static bool load_from_tga(Texture& texture, std::string_view filename);
};


struct Transform {
    Mat4 world_matrix;
    Vec3 translation;
    Vec3 rotation;
    Vec3 scale;

    void update_world_matrix() {
        world_matrix = Mat4::translation(translation) *
                       Mat4::rot_z(rotation.z) *
                       Mat4::rot_y(rotation.y) *
                       Mat4::rot_x(rotation.x) *
                       Mat4::scale(scale);
    }
};
