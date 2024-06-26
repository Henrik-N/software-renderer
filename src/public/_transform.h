#pragma once
#include "_common.h"
#include "_math.h"

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
