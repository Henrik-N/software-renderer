#include "_asset_store.h"
#include "_camera.h"
#include "_debug_rotate.h"
#include "_debug_texture.h"
#include "_ecs.h"
#include "_mesh_render.h"
#include "_renderer.h"
#include "_time.h"
#include "_window.h"


static void spawn_icosphere(Registry& reg) {
    const Entity sphere = reg.add();
    reg.add(sphere, Transform{
        .translation = {-2.f, -2.f, 5.f},
        .scale = {1.2f, 2.f, 1.2f},
    });
    reg.add(sphere, Debug_Rotate{});
    reg.add(sphere, reg.get<Asset_Store_System>().get_mesh_id("isphere"));
}


static void spawn_cube(Registry& reg, const Vec3 translation) {
    const Entity cube = reg.add();
    reg.add(cube, Transform{
        .translation = translation,
    });
    reg.add(cube, Debug_Rotate{});
    reg.add(cube, reg.get<Asset_Store_System>().get_mesh_id("cube"));
}


static void test_texture(Registry& reg, const std::string_view filename) {
    if (!reg.has<Debug_Display_Texture_System>()) {
        reg.add<Debug_Display_Texture_System>(reg);
    }

    const Entity entity = reg.add();
    const Texture_Id texture = reg.get<Asset_Store_System>().load_texture_asset(filename, filename);
    reg.add(entity, texture);
}


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
    reg->add<Asset_Store_System>();

    reg->add<Mesh_Render_System>(*reg);
    reg->add<Debug_Rotate_System>();


    // Load assets
    {
        auto& asset_store = reg->get<Asset_Store_System>();
        asset_store.load_mesh_asset("isphere", "icosphere.obj");
        asset_store.load_mesh_asset("cube", "cube.obj");
    }

    spawn_icosphere(*reg);
    spawn_cube(*reg, {2.f, 2.f, 5.f});
    spawn_cube(*reg, {4.f, -1.f, 8.f});

    // test_texture(*reg, "tiger.tga");


    while (true) {
        if (!window.poll_events()) return EXIT_SUCCESS;

        reg->refresh_systems_entity_sets();
        reg->get<Time_System>().update();
        reg->get<Debug_Rotate_System>().update(*reg);
        reg->get<Mesh_Render_System>().update(*reg);

        if (reg->has<Debug_Display_Texture_System>()) {
            reg->get<Debug_Display_Texture_System>().update();
        }

        if (!window.present()) return EXIT_FAILURE;
    }
}
