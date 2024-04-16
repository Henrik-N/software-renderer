#include "_window.h"
#include "_color.h"

#include <cassert>
#include <iostream>

// =====================================================================================================================
// == Util =============================================================================================================
// =====================================================================================================================

static void log_sdl_error() {
    std::cerr << "SDL error " << SDL_GetError() << std::endl;
}


// =====================================================================================================================
// == Window ===========================================================================================================
// =====================================================================================================================

Window::Window(): width(0),
                  height(0),
                  sdl_window(nullptr),
                  sdl_renderer(nullptr),
                  sdl_color_buffer_texture(nullptr),
                  color_buffer({}) {
}


Window::~Window() {
    if (sdl_color_buffer_texture) SDL_DestroyTexture(sdl_color_buffer_texture);
    if (sdl_renderer) SDL_DestroyRenderer(sdl_renderer);
    if (sdl_window) SDL_DestroyWindow(sdl_window);
    SDL_Quit();
}


bool Window::init(Window& window, const i32 resolution_width, const i32 resolution_height, const bool real_fullscreen) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << __FUNCTION__ << ": SDL_init failed" << std::endl;
        return {};
    }

    auto failure = []() -> bool {
        log_sdl_error();
        return false;
    };


    // Determine resolution
    {
        constexpr i32 display_index = 0;
        SDL_DisplayMode display_mode;
        if (SDL_GetCurrentDisplayMode(display_index, &display_mode) != 0) {
            return failure();
        }

        window.width = resolution_width > 0 ? resolution_width : display_mode.w;
        window.height = resolution_height > 0 ? resolution_height : display_mode.h;
    }


    // Create window
    {
        window.sdl_window = SDL_CreateWindow(nullptr,
                                             SDL_WINDOWPOS_CENTERED,
                                             SDL_WINDOWPOS_CENTERED,
                                             window.width,
                                             window.height,
                                             SDL_WINDOW_BORDERLESS
                                             );
        if (!window.sdl_window) {
            return failure();
        }

        if (real_fullscreen) {
            SDL_SetWindowFullscreen(window.sdl_window, SDL_WINDOW_FULLSCREEN);
        }
    }


    // Create SDL renderer
    {
        constexpr i32 rendering_driver_index = -1; // first one supporting the given flags
        constexpr i32 renderer_flags = 0;
        window.sdl_renderer = SDL_CreateRenderer(window.sdl_window,
                                                 rendering_driver_index,
                                                 renderer_flags
                                                 );
        if (!window.sdl_renderer) {
            return failure();
        }
    }


    // Validate pixel format
    {
        constexpr SDL_PixelFormatEnum pixel_format = SDL_PIXELFORMAT_ARGB8888;

        SDL_RendererInfo renderer_info;
        if (SDL_GetRendererInfo(window.sdl_renderer, &renderer_info) != 0) {
            return failure();
        }

        bool is_valid_format = false;
        for (u32 k = 0; k < renderer_info.num_texture_formats; ++k) {
            if (pixel_format == renderer_info.texture_formats[k]) {
                is_valid_format = true;
                break;
            }
        }
        if (!is_valid_format) {
            std::cerr << __FUNCTION__ << ": pixel format unsupported." << std::endl;
            return failure();
        }

        window.sdl_color_buffer_texture = SDL_CreateTexture(window.sdl_renderer,
                                                            pixel_format,
                                                            SDL_TEXTUREACCESS_STREAMING, // we'll be updating the texture every frame
                                                            window.width,
                                                            window.height
                                                            );
    }

    window.color_buffer = std::vector(window.width * window.height, Color::black());
    window.color_buffer.shrink_to_fit();

    return true;
}


bool Window::poll_events() const {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT: {
                return false;
            }
            case SDL_KEYDOWN: {
                const SDL_KeyboardEvent keyboard_event = event.key;
                if (keyboard_event.keysym.sym == SDLK_ESCAPE) {
                    return false;
                }
            }
            default: ;
        }
    }

    return true;
}


void Window::set_pixel(const i32 x, const i32 y, const Color color) {
    if (x < 0 || x >= width || y < 0 || y >= height) {
        return;
    }

    const i32 index = (width * y) + x;
    if (index < 0 || index >= color_buffer.size()) {
        return;
    }

    color_buffer[index] = color;
}


bool Window::present() {
    if (!render_present_color_buffer()) {
        return false;
    }
    clear_color_buffer(Color::black());

    return true;
}


void Window::clear_color_buffer(const Color in_color) {
    std::fill(color_buffer.begin(), color_buffer.end(), in_color);
}


bool Window::render_present_color_buffer() const {
    static_assert(sizeof(Color) == 4);
    assert(color_buffer.size() == width * height);

    auto failure = []() -> bool {
        log_sdl_error();
        return false;
    };


    void* pixels;
    i32 pitch;
    i32 result = SDL_LockTexture(sdl_color_buffer_texture, 
                                 nullptr,  // full rect
                                 &pixels, 
                                 &pitch
                                 );
    if (result != 0) {
        return failure();
    }
    assert(pitch == width * sizeof(Color));

    memcpy(pixels, color_buffer.data(), sizeof(Color) * color_buffer.size());
    SDL_UnlockTexture(sdl_color_buffer_texture);

    result = SDL_RenderCopy(sdl_renderer, 
                            sdl_color_buffer_texture, 
                            nullptr, 
                            nullptr
                            );
    if (result != 0) {
        return failure();
    }

    SDL_RenderPresent(sdl_renderer);

    return true;
}
