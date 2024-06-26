#pragma once
#include "_common.h"
#include "_ecs.h"
#include "_types.h"

struct Debug_Display_Texture_System final : System {
    explicit Debug_Display_Texture_System(Registry& in_reg);

    void update();

private:
    Registry& reg;
    Window_System& window;
    Asset_Store_System& asset_store;
};
