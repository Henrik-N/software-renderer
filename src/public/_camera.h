#pragma once
#include "_common.h"
#include "_ecs.h"
#include "_math.h"


struct Window_System;


struct Camera_System final : System {
    explicit Camera_System(Registry& registry);

    void set_fov(f32 fov_deg);
    Mat4 get_projection_matrix() const;
    Vec3 get_position() const;

private:
    Window_System& window;

    Vec3 position;
    f32 fov;
    f32 aspect_ratio;
    f32 z_near;
    f32 z_far;
};
