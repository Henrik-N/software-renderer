#pragma once

#include "_common.h"

#include "_color.h"
#include "_light.h"
#include "_math.h"
#include "_mesh.h"
#include "_time.h"
#include "_window.h"


constexpr bool enable_culling = true;


using Triangle = std::array<Vec2i, 3>;


struct Application {
    Window window;
    Time time;

    Vec3 camera_position;
    Light light;

    f32 fov_factor;
    f32 t;

    Mesh mesh;

    std::vector<Triangle> triangles_to_draw;
    std::vector<f32> triangle_depth_values;
    std::vector<usize> triangle_draw_order;

    std::vector<Color> triangle_draw_colors;

    static bool init(Application& app);
    i32 run();

private:
    bool setup();
    void update();
    void render();
};


namespace rendering {
    Vec2 project_point(Vec3 point, f32 fov_factor);
    void draw_grid(Window& window, i32 x_step, i32 y_step, Color color);
    void draw_rect(Window& window, Vec2i coord, Vec2i rect, Color color);
    void draw_line(Window& window, Vec2i from, Vec2i to, Color color);
    void draw_triangle_wireframe(Window& window, const Triangle& triangle, Color color);
    void draw_triangle_filled(Window& window, const Triangle& triangle, Color color);
}
