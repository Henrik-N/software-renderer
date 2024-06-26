#include "_debug_rotate.h"

#include "_transform.h"
#include "_time.h"


Debug_Rotate_System::Debug_Rotate_System()
    : t(0.f) {
    require_component<Transform>();
    require_component<DebugRotate>();
}


void Debug_Rotate_System::update(Registry& reg) {
    const f32 dt = reg.get<Time_System>().get_delta_seconds();

    t += dt * 0.1f;
    if (t >= 1.f) {
        t = 0.f;
    }

    const f32 rot_angle = static_cast<f32>(math::tau) * t;

    for (const Entity entity: get_entities()) {
        Transform& transform = reg.get<Transform>(entity);
        transform.rotation.x = rot_angle;
        transform.rotation.y = rot_angle;
        transform.rotation.z = rot_angle;
    }
}
