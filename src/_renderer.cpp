#include "_renderer.h"
#include "_application.h"
#include "_ecs.h"
#include "_window.h"


Render_System::Render_System(Registry& registry)
    : window(registry.get<Window_System>()) {
}


void Render_System::draw_grid(const i32 x_step, const i32 y_step, const Color color) const {
    rendering::draw_grid(window, x_step, y_step, color);
}


void Render_System::draw_rect(const Vec2i coord, const Vec2i rect, const Color color) const {
    rendering::draw_rect(window, coord, rect, color);
}


void Render_System::draw_line(const Vec2i from, const Vec2i to, const Color color) const {
    rendering::draw_line(window, from, to, color);
}


void Render_System::draw_triangle_wireframe(const Triangle& triangle, const Color color) const {
    rendering::draw_triangle_wireframe(window, triangle, color);
}


void Render_System::draw_triangle_filled(const Triangle& triangle, const Color color) const {
    rendering::draw_triangle_filled(window, triangle, color);
}
