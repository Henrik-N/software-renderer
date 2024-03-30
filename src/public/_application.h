#pragma once

#include "_common.h"

#include "_math.h"
#include "_mesh.h"
#include "_time.h"
#include "_window.h"

constexpr bool enable_culling = true;

using Triangle = std::array<i32_2, 3>;

struct Application {
    Window window;
    Time time;

    Vector3 camera_position;

    f32 fov_factor;
    f32 t;

    Mesh mesh;

    std::vector<Triangle> triangles_to_draw;

    static bool init(Application& app);
    i32 run();

private:
    bool setup();
    void update();
    void render();
};


namespace rendering {
    Vector2 project_point(const Vector3& point, f32 fov_factor);
    void draw_grid(Window& window, i32 x_step, i32 y_step, Color color);
    void draw_rect(Window& window, i32_2 coord, i32_2 rect, Color color);
    void draw_line(Window& window, i32_2 from, i32_2 to, Color color);
    void draw_triangle_wireframe(Window& window, const Triangle& triangle, Color color);
    void draw_triangle_filled(Window& window, const Triangle& triangle, Color color);
}
