#include "_application.h"
#include "_color.h"

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
    projected_faces.resize(num_faces);
    if constexpr (enable_culling) {
        should_cull_face.resize(num_faces);
    }

    return true;
}


void Application::update() {
    t += time.delta_seconds * 0.1f;
    if (t >= 1.f) {
        t = 0.f;
    }

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

            should_cull_face[face_index] = math::dot(face_normal_not_normalized, ray_to_face_corner) > 0;
            if (should_cull_face[face_index]) {
                continue;
            }
        }

        // project
        //
        Projected_Face& projected_face = projected_faces[face_index];

        usize corner_index = 0;
        for (const Vector3& corner : face_corners) {
            Vector2& projected_corner = projected_face[corner_index++];

            projected_corner = rendering::project_point(corner, fov_factor);

            projected_corner.x += static_cast<f32>(window.width) / 2.f;
            projected_corner.y += static_cast<f32>(window.height) / 2.f;
        }

        projected_faces[face_index] = projected_face;
    }
}


void Application::render() {
    using namespace rendering;

    draw_grid(window, 10, 10, Color::grey());

    for (i32 face_index = 0; face_index < projected_faces.size(); ++face_index) {
        if constexpr (enable_culling) {
            if (should_cull_face[face_index]) {
                continue;
            }
        }

        const Projected_Face& projected_face = projected_faces[face_index];

        std::array<i32_2, 3> triangle;
        for (i32 corner_index = 0; corner_index < triangle.size(); ++corner_index) {
            triangle[corner_index].x = static_cast<i32>(projected_face[corner_index].x);
            triangle[corner_index].y = static_cast<i32>(projected_face[corner_index].y);
        }

        constexpr Color color = Color::white();
        draw_triangle_wireframe(window, triangle, color);
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

    for (i32 i = 0; i < largest_side_length; ++i) {
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
