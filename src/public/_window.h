#pragma once

#include "_common.h"
#include "_ecs.h"

#include <SDL2/SDL.h>
#include <vector>

struct Color;


struct Window_System final : System {
    i32 width;
    i32 height;

    SDL_Window* sdl_window;
    SDL_Renderer* sdl_renderer;
    SDL_Texture* sdl_color_buffer_texture;

    std::vector<Color> color_buffer;

    explicit Window_System();
    ~Window_System() override;
    bool init(i32 resolution_width, i32 resolution_height, bool real_fullscreen);

    bool poll_events() const;

    void clear_color_buffer(Color in_color);
    void set_pixel(i32 x, i32 y, Color color); // (in color buffer)
    bool present(); // present color buffer to the screen

    Window_System(const Window_System&) = delete;
    Window_System(const Window_System&&) = delete;
private:
    bool render_present_color_buffer() const;
};
