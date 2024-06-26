#pragma once
#include "_common.h"
#include "_application.h"
#include "_color.h"
#include "_ecs.h"
#include "_math.h"


struct Window_System;

struct Render_System final : System {
    explicit Render_System(Registry& registry);

    void draw_grid(i32 x_step, i32 y_step, Color color) const;
    void draw_rect(Vec2i coord, Vec2i rect, Color color) const;
    void draw_line(Vec2i from, Vec2i to, Color color) const;
    void draw_triangle_wireframe(const Triangle& triangle, Color color) const;
    void draw_triangle_filled(const Triangle& triangle, Color color) const;

private:
    Window_System& window;
};
