#pragma once
#include "_common.h"

#include <SDL2/SDL.h>

struct Color;

struct Window {
    i32 width;
    i32 height;

    SDL_Window* sdl_window;
    SDL_Renderer* sdl_renderer;
    SDL_Texture* sdl_color_buffer_texture;

    std::vector<Color> color_buffer;

    static bool init(Window& window, i32 resolution_width, i32 resolution_height, bool real_fullscreen);
    explicit Window();
    ~Window();

    bool poll_events() const;

    void clear_color_buffer(Color in_color);
    void set_pixel(i32 x, i32 y, Color color); // (in color buffer)
    bool present(); // present color buffer to the screen

    Window(const Window&) = delete;
    Window(const Window&&) = delete;
private:
    bool render_present_color_buffer() const;
};
