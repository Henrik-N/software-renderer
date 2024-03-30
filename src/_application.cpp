#include "_application.h"
#include "_color.h"

#include <random>


// =====================================================================================================================
// == Application ======================================================================================================
// =====================================================================================================================

bool Application::init(Application& app) {
    if (!Window::init(app.window, 800, 600, false)) {
        return false;
    }

    Time::init(app.time);

    app.fov_factor = 600.f;

    app.camera_position = Vector3{0, 0, 0};

    app.t = 0.f;

    app.mesh = {};

    return true;
}


i32 Application::run() {
    if (!setup()) {
        return EXIT_FAILURE;
    }

    while (true) {
        if (!window.poll_events()) return EXIT_SUCCESS;
        time.tick();
        update();
        render();
        if (!window.present()) return EXIT_FAILURE;
    }
}


bool Application::setup() {
    if (!Mesh::load_from_obj(mesh, "cube.obj")) {
        return false;
    }

    const usize num_faces = mesh.faces.size();
    triangles_to_draw.reserve(num_faces);
    triangle_depth_values.reserve(num_faces);
    triangle_draw_order.reserve(num_faces);


    triangle_draw_colors.resize(num_faces);
    {
        std::random_device random;
        std::uniform_int_distribution<u8> dist{0, 255};
        for (Color& col : triangle_draw_colors) {
            col = Color::rgba(dist(random), dist(random), dist(random), 255);
        }
    }

    return true;
}


void Application::update() {
    t += time.delta_seconds * 0.1f;
    if (t >= 1.f) {
        t = 0.f;
    }

    triangles_to_draw.clear();
    triangle_depth_values.clear();

    const f32 rot_angle = static_cast<f32>(math::tau) * t;

    for (usize face_index = 0; face_index < mesh.faces.size(); ++face_index) {
        Vector3 face_corners[3]{
            mesh.vertices[mesh.faces[face_index][0]],
            mesh.vertices[mesh.faces[face_index][1]],
            mesh.vertices[mesh.faces[face_index][2]],
        };

        // transform
        //
        for (Vector3& corner : face_corners) {
            corner = math::rot_x(corner, rot_angle);
            corner = math::rot_y(corner, rot_angle);
            corner = math::rot_z(corner, rot_angle);
            corner.z += 5.f;
        }

        // temporary culling solution
        if constexpr (enable_culling) {
            const Vector3 face_normal_not_normalized = math::cross(face_corners[1] - face_corners[0],
                                                                   face_corners[2] - face_corners[0]);
            const Vector3 ray_to_face_corner = face_corners[0] - camera_position;

            if (const bool should_cull_face = math::dot(face_normal_not_normalized, ray_to_face_corner) >= 0;
                should_cull_face) {
                continue;
            }
        }


        f32& triangle_depth = triangle_depth_values.emplace_back();
        triangle_depth = (face_corners[0].z + face_corners[1].z + face_corners[2].z); // /3.f,;

        Triangle& triangle = triangles_to_draw.emplace_back();
        for (usize corner_index = 0; corner_index < 3; ++corner_index) {
            Vector2 projected_corner = rendering::project_point(face_corners[corner_index], fov_factor);
            projected_corner.x += static_cast<f32>(window.width) / 2.f;
            projected_corner.y += static_cast<f32>(window.height) / 2.f;

            triangle[corner_index] = static_cast<i32_2>(projected_corner);
        }
    }

    triangle_draw_order.resize(triangles_to_draw.size());
    std::iota(triangle_draw_order.begin(), triangle_draw_order.end(), 0);
    std::sort(triangle_draw_order.begin(), triangle_draw_order.end(), [&](const usize a, const usize b) -> bool {
        return triangle_depth_values[a] > triangle_depth_values[b]; // larger z == draw first
    });
}


void Application::render() {
    using namespace rendering;

    draw_grid(window, 10, 10, Color::grey());

    for (const usize draw_index : triangle_draw_order) {
        const Triangle& triangle = triangles_to_draw[draw_index];
        const Color draw_color = triangle_draw_colors[draw_index];
        draw_triangle_filled(window, triangle, draw_color);
        draw_triangle_wireframe(window, triangle, Color::white());
    }
}


// =====================================================================================================================
// == namespace rendering ==============================================================================================
// =====================================================================================================================

Vector2 rendering::project_point(const Vector3& point, const f32 fov_factor) {
    // At this point, everything will be presented 1:1 onto the screen
    // If the point's position was (1, 1, 1) and we present it as is, the final pixel location will be (1, 1)
    Vector2 pixel_coord{point.x, point.y};

    // With a fov factor of 100, the final pixel location of that same point will be (100, 100)
    pixel_coord *= fov_factor;

    // Larger z-value == smaller displacement from the origin
    const f32 z = point.z != 0 ? point.z : 0.01f;
    pixel_coord /= z;

    // Positive y up
    pixel_coord.y = -pixel_coord.y;

    return pixel_coord;
}


void rendering::draw_grid(Window& window, const i32 x_step, const i32 y_step, const Color color) {
    for (i32 y = 0; y < window.height; y += y_step) {
        for (i32 x = 0; x < window.width; x += x_step) {
            window.set_pixel(x, y, color);
        }
    }
}


void rendering::draw_rect(Window& window, const i32_2 coord, const i32_2 rect, const Color color) {
    i32_2 start;
    start.x = std::max(0, std::min(coord.x, window.width));
    start.y = std::max(0, std::min(coord.y, window.height));

    i32_2 end;
    end.x = std::max(0, std::min(coord.x + rect.width, window.width));
    end.y = std::max(0, std::min(coord.y + rect.height, window.height));

    for (i32 y = start.y; y < end.y; ++y) {
        for (i32 x = start.x; x < end.x; ++x) {
            window.set_pixel(x, y, color);
        }
    }
}


void rendering::draw_line(Window& window, const i32_2 from, const i32_2 to, const Color color) {
    const i32_2 delta = to - from;
    const i32 largest_side_length = std::max(std::abs(delta.x), std::abs(delta.y));
    if (largest_side_length == 0) {
        return;
    }

    const Vector2 step{
        static_cast<f32>(delta.x) / static_cast<f32>(largest_side_length),
        static_cast<f32>(delta.y) / static_cast<f32>(largest_side_length),
    };

    Vector2 current_coord{static_cast<f32>(from.x), static_cast<f32>(from.y)};

    // NOTE: It is important that loop conidition is <= and not <, or we are missing the last pixel, creating artifacts
    // when drawing triangles
    for (i32 i = 0; i <= largest_side_length; ++i) {
        const i32_2 screen_coord{
            .x = static_cast<i32>(current_coord.x),
            .y = static_cast<i32>(current_coord.y),
        };

        window.set_pixel(screen_coord.x, screen_coord.y, color);

        current_coord += step;
    }
}


void rendering::draw_triangle_wireframe(Window& window, const Triangle& triangle, const Color color) {
    draw_line(window, triangle[0], triangle[1], color);
    draw_line(window, triangle[1], triangle[2], color);
    draw_line(window, triangle[2], triangle[0], color);
}


void rendering::draw_triangle_filled(Window& window, const Triangle& triangle, const Color color) {
    // draw triangle with flat bottom from top to bottom
    auto draw_bottom_flat = [&window, color](const Triangle& tri) -> void {
        const i32_2 corner_a = tri[0]; // top
        const i32_2 corner_b = tri[1]; // bottom 0
        const i32_2 corner_c = tri[2]; // bottom 1

        const f32_2 inverse_slopes{
            static_cast<f32>(corner_b.x - corner_a.x) / static_cast<f32>(corner_b.y - corner_a.y),
            static_cast<f32>(corner_c.x - corner_a.x) / static_cast<f32>(corner_c.y - corner_a.y),
        };

        f32_2 current_xs_f32s = f32_2::splat(static_cast<f32>(corner_a.x));

        for (i32 scanline_y = corner_a.y; scanline_y <= corner_b.y; ++scanline_y) {
            const i32_2 current_xs_i32 = static_cast<i32_2>(current_xs_f32s);

            draw_line(window,
                      {current_xs_i32[0], scanline_y},
                      {current_xs_i32[1], scanline_y},
                      color
            );

            current_xs_f32s += inverse_slopes;
        }
    };

    // draw triangle with flat bottom from bottom to top
    auto draw_top_flat = [&window, color](const Triangle& tri) -> void {
        // draw fr
        const i32_2 corner_a = tri[0]; // top 0
        const i32_2 corner_b = tri[1]; // top 1
        const i32_2 corner_c = tri[2]; // bottom

        const f32_2 inverse_slopes{
            static_cast<f32>(corner_c.x - corner_a.x) / static_cast<f32>(corner_c.y - corner_a.y),
            static_cast<f32>(corner_c.x - corner_b.x) / static_cast<f32>(corner_c.y - corner_b.y),
        };

        f32_2 current_xs_f32s = f32_2::splat(static_cast<f32>(corner_c.x));

        for (i32 scanline_y = corner_c.y; scanline_y > corner_a.y; --scanline_y) {
            const i32_2 current_xs_i32 = static_cast<i32_2>(current_xs_f32s);

            draw_line(window,
                      {current_xs_i32[0], scanline_y},
                      {current_xs_i32[1], scanline_y},
                      color
            );

            current_xs_f32s -= inverse_slopes;
        }
    };


    i32_2 corner_a = triangle[0];
    i32_2 corner_b = triangle[1];
    i32_2 corner_c = triangle[2];

    // Sort by y. Positive y is down here. A->B->C, where A is the lowest Y-value.
    if (corner_a.y > corner_b.y) std::swap(corner_a, corner_b);
    if (corner_a.y > corner_c.y) std::swap(corner_a, corner_c);
    if (corner_b.y > corner_c.y) std::swap(corner_b, corner_c);


    // avoids division by zero in draw_top_flat
    if (corner_b.y == corner_c.y) {
        draw_bottom_flat(Triangle{corner_a, corner_b, corner_c});
        return;
    }

    // avoids division by zero in draw_bottom_flat
    if (corner_a.y == corner_b.y) {
        draw_top_flat(Triangle{corner_a, corner_b, corner_c});
        return;
    }


    // NOTE: Division by zero is possible in a case where
    // corner_a.y == corner_b.y == corner_c.y may also be possible in a case where the triangle is directly
    // aligned with the camera view.


    i32_2 triangle_midpoint;
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
