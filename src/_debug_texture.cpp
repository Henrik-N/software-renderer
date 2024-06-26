#include "_debug_texture.h"
#include "_asset_store.h"
#include "_types.h"
#include "_window.h"

Debug_Display_Texture_System::Debug_Display_Texture_System(Registry& in_reg)
    : reg(in_reg),
      window(in_reg.get<Window_System>()),
      asset_store(in_reg.get<Asset_Store_System>()) {
    require_component<Texture_Id>();
}


void Debug_Display_Texture_System::update() {
    for (const Entity entity : get_entities()) {
        const Texture_Id texture_id = reg.get<Texture_Id>(entity);
        const Texture_View texture = asset_store.access_texture_data(texture_id);

        for (i32 y = 0; y < texture.height; ++y) {
            for (i32 x = 0; x < texture.width; ++x) {
                window.set_pixel(x, y, texture.get_pixel(x, y));
            }
        }
    }
}
