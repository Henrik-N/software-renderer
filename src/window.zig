const std = @import("std");

const c = @cImport({
    @cInclude("SDL2/SDL.h");
});

const Color = @import("color.zig").Color;

fn logSdlError() void {
    std.log.err("{s}", .{c.SDL_GetError()});
}

pub const Window = struct {
    sdl_window: *c.SDL_Window,
    width: u32,
    height: u32,

    sdl_renderer: *c.SDL_Renderer,
    sdl_color_buffer_texture: *c.SDL_Texture, // color buffer texture

    ally: std.mem.Allocator,
    color_buffer: []Color,

    pub fn deinit(self: Window) void {
        self.ally.free(self.color_buffer);
        c.SDL_DestroyTexture(self.sdl_color_buffer_texture);
        c.SDL_DestroyRenderer(self.sdl_renderer);
        c.SDL_DestroyWindow(self.sdl_window);
        c.SDL_Quit();
    }

    pub fn init(allocator: std.mem.Allocator, resolution_width: ?u32, resolution_height: ?u32, fullscreen: bool) !Window {
        if (c.SDL_Init(c.SDL_INIT_EVERYTHING) != 0) {
            c.SDL_Log("Unable to initialize SDL: %s", c.SDL_GetError());
            return error.SDLInitFailed;
        }
        errdefer c.SDL_Quit();

        var self: Window = undefined;

        {
            const display_index = 0;
            var display_mode: c.SDL_DisplayMode = undefined;
            if (c.SDL_GetCurrentDisplayMode(display_index, &display_mode) != 0) {
                logSdlError();
                return error.SDL_GetCurrentDisplayMode;
            }

            // fake fullscreen if width == null and height == null
            self.width = if (resolution_width) |w| w else @intCast(display_mode.w);
            self.height = if (resolution_height) |h| h else @intCast(display_mode.h);
        }

        self.sdl_window = c.SDL_CreateWindow(
            null, // window title
            @as(c_int, c.SDL_WINDOWPOS_CENTERED),
            @as(c_int, c.SDL_WINDOWPOS_CENTERED),
            @as(c_int, @intCast(self.width)),
            @as(c_int, @intCast(self.height)),
            @as(c_int, c.SDL_WINDOW_BORDERLESS),
        ) orelse {
            logSdlError();
            return error.SDL_CreateWindow;
        };
        errdefer c.SDL_DestroyWindow(self.sdl_window);

        if (fullscreen) { // "real" fullscreen
            _ = c.SDL_SetWindowFullscreen(self.sdl_window, c.SDL_WINDOW_FULLSCREEN);
        }

        const rendering_driver_index = -1; // first one found supporting the given flags
        const renderer_flags = 0;
        self.sdl_renderer = c.SDL_CreateRenderer(
            self.sdl_window,
            rendering_driver_index,
            renderer_flags,
        ) orelse {
            logSdlError();
            return error.SDL_CreateRenderer;
        };
        errdefer c.SDL_DestroyRenderer(self.sdl_renderer);

        var renderer_info: c.SDL_RendererInfo = undefined;
        if (c.SDL_GetRendererInfo(self.sdl_renderer, &renderer_info) != 0) {
            logSdlError();
            return error.SDL_GetRendererInfo;
        }

        const pixel_format: c.SDL_PixelFormatEnum = c.SDL_PIXELFORMAT_ARGB8888;

        for (0..renderer_info.num_texture_formats) |index| {
            if (pixel_format == renderer_info.texture_formats[index]) {
                break;
            }
        } else return error.PixelFormatUnsupported;

        self.sdl_color_buffer_texture = c.SDL_CreateTexture(
            self.sdl_renderer,
            pixel_format,
            c.SDL_TEXTUREACCESS_STREAMING, // will be updating this texture every frame
            @as(c_int, @intCast(self.width)),
            @as(c_int, @intCast(self.height)),
        ) orelse {
            logSdlError();
            return error.SDL_CreateTexture;
        };
        errdefer c.SDL_DestroyTexture(self.sdl_color_buffer_texture);

        self.ally = allocator;

        self.color_buffer = try self.ally.alloc(Color, self.width * self.height);
        errdefer self.ally.free(self.color_buffer);
        @memset(self.color_buffer, Color.black);

        return self;
    }

    pub fn pollEvents(_: Window) bool {
        var event: c.SDL_Event = undefined;
        while (c.SDL_PollEvent(&event) == 1) {
            //
            switch (event.type) {
                c.SDL_QUIT => {
                    return false;
                },
                c.SDL_KEYDOWN => {
                    const keyboard_event: c.SDL_KeyboardEvent = event.key;
                    if (keyboard_event.keysym.sym == c.SDLK_ESCAPE) {
                        return false;
                    }
                },
                else => {},
            }
        }
        return true;
    }

    pub fn setPixel(self: Window, x: usize, y: usize, color: Color) void {
        self.color_buffer[(self.width * y) + x] = color; // in debug builds zig will panic when out of bounds.
    }

    fn renderColorBuffer(self: Window) !void {
        // TODO: UpdateTexture is slow and meant to be used with static textures
        var result: c_int = c.SDL_UpdateTexture(
            self.sdl_color_buffer_texture,
            null, // full rect
            self.color_buffer.ptr,
            @as(c_int, @intCast(self.width * @sizeOf(Color))),
        );
        if (result != 0) {
            logSdlError();
            return error.SDL_UpdateTexture;
        }

        result = c.SDL_RenderCopy(self.sdl_renderer, self.sdl_color_buffer_texture, null, null);
        if (result != 0) {
            logSdlError();
            return error.SDL_RenderCopy;
        }
    }

    fn clearColorBuffer(self: Window, color: Color) void {
        @memset(self.color_buffer, color);
    }

    pub fn present(self: Window) !void {
        try self.renderColorBuffer();
        self.clearColorBuffer(Color.black);
        c.SDL_RenderPresent(self.sdl_renderer);
    }
};

