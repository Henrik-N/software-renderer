#include "_mesh_render.h"

#include "_camera.h"
#include "_renderer.h"
#include "_window.h"


constexpr bool enable_culling = true;


Mesh_Render_System::Mesh_Render_System(const Registry& reg)
    : window(reg.get<Window_System>()),
      renderer(reg.get<Render_System>()),
      camera(reg.get<Camera_System>()) {

    require_component<Transform>();
    require_component<Mesh>();
}


void Mesh_Render_System::update(Registry& reg) {
    triangles_to_draw.clear();
    triangle_depth_values.clear();
    triangle_draw_colors.clear();

    const f32 half_window_width = static_cast<f32>(window.width) / 2.f;
    const f32 half_window_height = static_cast<f32>(window.height) / 2.f;

    const Mat4 projection_matrix = camera.get_projection_matrix();


    for (const Entity entity : get_entities()) {
        const Mesh& mesh = reg.get<Mesh>(entity);
        Transform& transform = reg.get<Transform>(entity);
        transform.update_world_matrix();

        const Mat4 world_matrix = transform.world_matrix;


        for (usize face_index = 0; face_index < mesh.faces.size(); ++face_index) {
            Vec3 face_corners[3]{
                mesh.vertices[mesh.faces[face_index][0]],
                mesh.vertices[mesh.faces[face_index][1]],
                mesh.vertices[mesh.faces[face_index][2]],
            };

            // Transformation
            for (Vec3& corner : face_corners) {
                corner = Vec3::from_vec4(world_matrix *
                                         Vec4::from_vec3(corner, 1.f)
                                         );
            }


            const Vec3 face_normal_not_normalized = math::cross(face_corners[1] - face_corners[0],
                                                                face_corners[2] - face_corners[0]
                                                                );

            // Temporary culling solution
            if constexpr (enable_culling) {
                const Vec3 ray_to_face_corner = face_corners[0] - camera.get_position();

                if (const bool should_cull_face = math::dot(face_normal_not_normalized, ray_to_face_corner) >= 0;
                    should_cull_face) {
                    continue;
                }
            }


            // Flat shading
            f32 light_intensity = -math::dot(light.direction, math::normalized(face_normal_not_normalized));
            if (light_intensity < 0.f)  {
                light_intensity = 0.0f;
            }
            triangle_draw_colors.emplace_back() = Color::white().with_intensity(light_intensity);


            // Temporary depth buffer
            triangle_depth_values.emplace_back() = face_corners[0].z + face_corners[1].z + face_corners[2].z; // / 3.f;


            // Projection
            //
            Triangle& triangle = triangles_to_draw.emplace_back();

            for (usize corner_index = 0; corner_index < 3; ++corner_index) {

                // Convert coordinates to image space / normalized device coordinates
                Vec4 projected_corner = projection_matrix * Vec4::from_vec3(face_corners[corner_index], 1.f);
                projected_corner = math::perspective_divide(projected_corner);


                // Flip y (positive y up)
                projected_corner.y = -projected_corner.y;

                // Translate origin to the middle of the viewport, then scale

                // scale
                projected_corner.x *= half_window_width;
                projected_corner.y *= half_window_height;

                // translate
                projected_corner.x += half_window_width;
                projected_corner.y += half_window_height;


                // Cast to screen coordinates

                triangle[corner_index] = Vec2i{
                    static_cast<i32>(projected_corner.x),
                    static_cast<i32>(projected_corner.y),
                };
            }
        }
    }

    // Calculate draw order based on depth values
    triangle_draw_order.resize(triangles_to_draw.size());
    std::iota(triangle_draw_order.begin(), triangle_draw_order.end(), 0);
    std::sort(triangle_draw_order.begin(), triangle_draw_order.end(), [&](const usize a, const usize b) -> bool {
        return triangle_depth_values[a] > triangle_depth_values[b]; // larger z == draw first
    });


    renderer.draw_grid(10, 10, Color::grey());

    for (const usize draw_index : triangle_draw_order) {
        const Triangle& triangle = triangles_to_draw[draw_index];
        const Color draw_color = triangle_draw_colors[draw_index];
        renderer.draw_triangle_filled(triangle, draw_color);
    }
}
