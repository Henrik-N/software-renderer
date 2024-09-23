#pragma once
#include <span>

#include "_common.h"
#include "_color.h"
#include "_math.h"


struct Asset_Store_System;
struct Camera_System;
struct Render_System;
struct Window_System;


using Triangle = std::array<Vec2i, 3>;
using Face_Vertex_Indices = std::array<u16, 3>;
using Face_UV_Indices = std::array<u16, 3>;


struct Light {
    Vec3 direction;
};


struct Mesh_Id {
    usize id = -1;
};


struct Mesh_View {
    std::span<Vec3> vertices;
    std::span<Face_Vertex_Indices> faces;
};


struct Texture_Id {
    usize id = -1;
};


struct Texture_View {
    std::span<Color> pixels;
    i32 width;
    i32 height;

    Color get_pixel(const i32 x, const i32 y) const { return pixels[(width * y) + x]; }
};



struct Transform {
    Mat4 world_matrix;
    Vec3 translation;
    Vec3 rotation;
    Vec3 scale = Vec3{1.f, 1.f, 1.f};

    void update_world_matrix() {
        world_matrix = Mat4::translation(translation) *
                       Mat4::rot_z(rotation.z) *
                       Mat4::rot_y(rotation.y) *
                       Mat4::rot_x(rotation.x) *
                       Mat4::scale(scale);
    }
};
