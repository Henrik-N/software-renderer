#pragma once

#include <memory>

#include "_common.h"

#include "_color.h"
#include "_math.h"
#include "_window.h"


using Triangle = std::array<Vec2i, 3>;

namespace rendering {
    Vec2 project_point(Vec3 point, f32 fov_factor);
    void draw_grid(Window_System& window, i32 x_step, i32 y_step, Color color);
    void draw_rect(Window_System& window, Vec2i coord, Vec2i rect, Color color);
    void draw_line(Window_System& window, Vec2i from, Vec2i to, Color color);
    void draw_triangle_wireframe(Window_System& window, const Triangle& triangle, Color color);
    void draw_triangle_filled(Window_System& window, const Triangle& triangle, Color color);
}
