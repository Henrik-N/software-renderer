#include "_renderer.h"

#include "_ecs.h"
#include "_window.h"


Render_System::Render_System(Registry& registry)
    : window(registry.get<Window_System>()) {
}


void Render_System::draw_grid(const i32 x_step, const i32 y_step, const Color color) const {
    for (i32 y = 0; y < window.height; y += y_step) {
        for (i32 x = 0; x < window.width; x += x_step) {
            window.set_pixel(x, y, color);
        }
    }
}


void Render_System::draw_rect(const Vec2i coord, const Vec2i rect, const Color color) const {
    Vec2i start;
    start.x = std::max(0, std::min(coord.x, window.width));
    start.y = std::max(0, std::min(coord.y, window.height));

    Vec2i end;
    end.x = std::max(0, std::min(coord.x + rect.width, window.width));
    end.y = std::max(0, std::min(coord.y + rect.height, window.height));

    for (i32 y = start.y; y < end.y; ++y) {
        for (i32 x = start.x; x < end.x; ++x) {
            window.set_pixel(x, y, color);
        }
    }
}


void Render_System::draw_line(const Vec2i from, const Vec2i to, const Color color) const {
    const Vec2i delta = to - from;
    const i32 largest_side_length = std::max(std::abs(delta.x), std::abs(delta.y));
    if (largest_side_length == 0) {
        return;
    }

    const Vec2 step{
        static_cast<f32>(delta.x) / static_cast<f32>(largest_side_length),
        static_cast<f32>(delta.y) / static_cast<f32>(largest_side_length),
    };

    Vec2 current_coord{static_cast<f32>(from.x), static_cast<f32>(from.y)};

    // NOTE: It is important that loop condition is <= and not <, or we are missing the last pixel, creating artifacts
    // when drawing triangles
    for (i32 i = 0; i <= largest_side_length; ++i) {
        const Vec2i screen_coord{
            .x = static_cast<i32>(current_coord.x),
            .y = static_cast<i32>(current_coord.y),
        };

        window.set_pixel(screen_coord.x, screen_coord.y, color);

        current_coord += step;
    }
}


void Render_System::draw_triangle_wireframe(const Triangle& triangle, const Color color) const {
    draw_line(triangle[0], triangle[1], color);
    draw_line(triangle[1], triangle[2], color);
    draw_line(triangle[2], triangle[0], color);
}


void Render_System::draw_triangle_filled(const Triangle& triangle, const Color color) const {
    // Draw triangle with flat bottom from top to bottom
    auto draw_bottom_flat = [this, color](const Triangle& tri) -> void {
        const Vec2i corner_a = tri[0]; // top
        const Vec2i corner_b = tri[1]; // bottom 0
        const Vec2i corner_c = tri[2]; // bottom 1

        const Vec2 inverse_slopes{
            static_cast<f32>(corner_b.x - corner_a.x) / static_cast<f32>(corner_b.y - corner_a.y),
            static_cast<f32>(corner_c.x - corner_a.x) / static_cast<f32>(corner_c.y - corner_a.y),
        };

        Vec2 current_xs_f32s = Vec2::splat(static_cast<f32>(corner_a.x));

        for (i32 scanline_y = corner_a.y; scanline_y <= corner_b.y; ++scanline_y) {
            const Vec2i current_xs_i32 = static_cast<Vec2i>(current_xs_f32s);

            draw_line({current_xs_i32[0], scanline_y},
                      {current_xs_i32[1], scanline_y},
                      color
                      );

            current_xs_f32s += inverse_slopes;
        }
    };

    // Draw triangle with flat bottom from bottom to top
    auto draw_top_flat = [this, color](const Triangle& tri) -> void {
        const Vec2i corner_a = tri[0]; // top 0
        const Vec2i corner_b = tri[1]; // top 1
        const Vec2i corner_c = tri[2]; // bottom

        const Vec2 inverse_slopes{
            static_cast<f32>(corner_c.x - corner_a.x) / static_cast<f32>(corner_c.y - corner_a.y),
            static_cast<f32>(corner_c.x - corner_b.x) / static_cast<f32>(corner_c.y - corner_b.y),
        };

        Vec2 current_xs_f32s = Vec2::splat(static_cast<f32>(corner_c.x));

        for (i32 scanline_y = corner_c.y; scanline_y > corner_a.y; --scanline_y) {
            const Vec2i current_xs_i32 = static_cast<Vec2i>(current_xs_f32s);

            draw_line({current_xs_i32[0], scanline_y},
                      {current_xs_i32[1], scanline_y},
                      color
                      );

            current_xs_f32s -= inverse_slopes;
        }
    };


    Vec2i corner_a = triangle[0];
    Vec2i corner_b = triangle[1];
    Vec2i corner_c = triangle[2];

    // Sort by y. Positive y is down here. A->B->C, where A is the lowest Y-value.
    if (corner_a.y > corner_b.y) std::swap(corner_a, corner_b);
    if (corner_a.y > corner_c.y) std::swap(corner_a, corner_c);
    if (corner_b.y > corner_c.y) std::swap(corner_b, corner_c);


    // Avoid division by zero in draw_top_flat
    if (corner_b.y == corner_c.y) {
        draw_bottom_flat(Triangle{corner_a, corner_b, corner_c});
        return;
    }

    // Avoid division by zero in draw_bottom_flat
    if (corner_a.y == corner_b.y) {
        draw_top_flat(Triangle{corner_a, corner_b, corner_c});
        return;
    }


    // NOTE/TODO: Division by zero may be possible when triangle is directly
    // aligned with the camera view corner_a.y == corner_b.y == corner_c.y


    Vec2i triangle_midpoint;
    {
        triangle_midpoint.x =
            static_cast<i32>(
                static_cast<f32>((corner_c.x - corner_a.x) * (corner_b.y - corner_a.y))
                /
                static_cast<f32>(corner_c.y - corner_a.y)
            )
            + corner_a.x;

        triangle_midpoint.y = corner_b.y;
    }

    draw_bottom_flat(Triangle{corner_a, corner_b, triangle_midpoint});
    draw_top_flat(Triangle{corner_b, triangle_midpoint, corner_c});
}


Vec2 Render_System::project_point(const Vec3 point, const f32 fov_factor) {
    // At this point, everything will be presented 1:1 onto the screen
    // If the point's position was (1, 1, 1) and we present it as is, the final pixel location will be (1, 1)
    Vec2 pixel_coord{point.x, point.y};

    // With a fov factor of 100, the final pixel location of that same point will be (100, 100)
    pixel_coord *= fov_factor;

    // Larger z-value == smaller displacement from the origin
    const f32 z = point.z != 0 ? point.z : 0.01f;
    pixel_coord /= z;

    // Positive y up
    pixel_coord.y = -pixel_coord.y;

    return pixel_coord;
}
