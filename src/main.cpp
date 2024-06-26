#include "_camera.h"
#include "_debug_rotate.h"
#include "_ecs.h"
#include "_mesh_render.h"
#include "_renderer.h"
#include "_time.h"
#include "_window.h"

struct Yeet{};
struct Moo{};

i32 main() {
    const std::unique_ptr<Registry> reg = std::make_unique<Registry>();

    reg->add<Window_System>();
    Window_System& window = reg->get<Window_System>();
    if (!window.init(800, 600, false)) {
        return false;
    }

    reg->add<Time_System>();
    reg->add<Render_System>(*reg);
    reg->add<Camera_System>(*reg);
    reg->add<Mesh_Render_System>(*reg);
    reg->add<Debug_Rotate_System>();


    const Entity mesh_entity = reg->add();
    reg->add(mesh_entity, Transform{
        .translation = {0.f, 0.f, 5.f},
        .scale = {1.2f, 2.f, 1.2f},
    });
    reg->add(mesh_entity, Debug_Rotate{});

    {
        Mesh mesh;
        if (!Mesh::load_from_obj(mesh, "icosphere.obj")) {
            return false;
        }
        reg->add(mesh_entity, std::move(mesh));
    }

    while (true) {
        if (!window.poll_events()) return EXIT_SUCCESS;

        reg->refresh_systems_entity_sets();
        reg->get<Time_System>().update();
        reg->get<Debug_Rotate_System>().update(*reg);
        reg->get<Mesh_Render_System>().update(*reg);

        if (!window.present()) return EXIT_FAILURE;
    }
}
