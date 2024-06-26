#pragma once
#include "_common.h"
#include "_application.h"
#include "_light.h"

struct Render_System;
struct Camera_System;
struct Window_System;

struct Mesh_Render_System final : System {
    explicit Mesh_Render_System(const Registry& reg);

    void update(Registry& reg);

private:
    const Window_System& window;
    const Render_System& renderer;
    const Camera_System& camera;

    std::vector<Triangle> triangles_to_draw;
    std::vector<f32> triangle_depth_values;
    std::vector<usize> triangle_draw_order;

    std::vector<Color> triangle_draw_colors;

    const Light light {
        .direction = Vec3{0.25f, -0.5f, 0.25f},
    };
};
