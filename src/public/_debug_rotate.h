#pragma once
#include "_common.h"
#include "_ecs.h"


struct Debug_Rotate {};


struct Debug_Rotate_System final : System {
    explicit Debug_Rotate_System();

    void update(Registry& reg);

private:
    f32 t;
};
