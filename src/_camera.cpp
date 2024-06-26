#include "_camera.h"
#include "_window.h"

Camera_System::Camera_System(Registry& registry)
    : window(registry.get<Window_System>()),
      position(Vec3::zeroed()),
      fov(math::to_radians(60.f)),
      aspect_ratio(0.f),
      z_near(0.1f),
      z_far(10.f) {
}


void Camera_System::set_fov(const f32 fov_deg) {
    fov = math::to_radians(fov_deg);
}


Mat4 Camera_System::get_projection_matrix() const {
    const f32 aspect_ratio = static_cast<f32>(window.height) / static_cast<f32>(window.width);
    return Mat4::perspective_projection(fov, aspect_ratio, z_near, z_far);
}


Vec3 Camera_System::get_position() const {
    return position;
}
