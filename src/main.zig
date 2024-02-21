const std = @import("std");
const builtin = @import("builtin");

const c = @cImport({
    @cInclude("SDL2/SDL.h");
});

fn logSdlError() void {
    std.log.err("{s}", .{c.SDL_GetError()});
}

const Color = packed union {
    hex: u32,
    channels: packed struct {
        a: u8,
        r: u8,
        g: u8,
        b: u8,
    },

    inline fn hex(hex_: u32) Color {
        return .{ .hex = hex_ };
    }

    inline fn rgba(r: u8, g: u8, b: u8, a: u8) Color {
        return .{ .channels = .{ .a = a, .r = r, .g = g, .b = b } };
    }

    const black = Color.hex(0xFF_00_00_00);
    const white = Color.hex(0xFF_FF_FF_FF);
    const grey = Color.hex(0xFF_33_33_33);
    const red = Color.hex(0xFF_FF_00_00);
    const green = Color.hex(0xFF_00_FF_00);
    const blue = Color.hex(0xFF_00_00_FF);
    const yellow = Color.hex(0xFF_FF_FF_00);
};

test "color union" {
    var colors: [3]u32 = undefined;
    colors[0] = @bitCast(Color.hex(0xFF0000FF));
    colors[1] = @bitCast(Color.rgba(0, 0, 255, 255));
    colors[2] = 0xFF0000FF;

    try std.testing.expect(std.mem.eql(u32, &.{colors[0]}, &.{colors[1]}));
    try std.testing.expect(std.mem.eql(u32, &.{colors[1]}, &.{colors[2]}));
}

const Window = struct {
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

fn drawGrid(window: Window, x_step: u32, y_step: u32, color: Color) void {
    var x: u32 = 0;
    var y: u32 = 0;

    while (y < window.height) {
        while (x < window.width) {
            window.setPixel(x, y, color);
            x += x_step;
        }
        x = 0;
        y += y_step;
    }
}

fn drawRect(window: Window, x: isize, y: isize, width: usize, height: usize, color: Color) void {
    const start_x: usize = @min(@max(0, x), window.width);
    const start_y: usize = @min(@max(0, y), window.height);

    const end_x: usize = @min(start_x + width, window.width);
    const end_y: usize = @min(start_y + height, window.height);

    for (start_y..end_y) |_y| {
        for (start_x..end_x) |_x| {
            window.setPixel(_x, _y, color);
        }
    }
}

const Vec2 = @Vector(2, f32);
const Vec3 = @Vector(3, f32);

fn projectOrtho(point: Vec3, fov_factor: f32) Vec2 {
    // At this point, everything will be presented 1:1.
    // If the position was (1, 1, 1) and we present it as is, the final pixel location will be (1, 1)
    const coord = Vec2{ point[0], point[1] };

    // With a fov factor of 100, the final pixel location will be (100, 100)
    return coord * @as(Vec2, @splat(fov_factor));
}

pub fn main() !void {
    var general_purpose_allocator = std.heap.GeneralPurposeAllocator(.{}){};
    defer {
        const did_leak = general_purpose_allocator.deinit() == .leak;
        if (builtin.mode == .Debug or builtin.mode == .ReleaseSafe) {
            std.debug.assert(did_leak == false);
        } else if (did_leak) {
            std.log.err("Memory leak detected!", .{});
        }
    }

    const gpa = general_purpose_allocator.allocator(); // allocator interface

    var window = try Window.init(gpa, null, null, false);
    defer window.deinit();

    const num_cube_points = 9 * 9 * 9;
    var cube_points: [num_cube_points]Vec3 = undefined;
    {
        var num_initialized: u32 = 0;
        var x: f32 = -1;
        var y: f32 = -1;
        var z: f32 = -1;

        while (z <= 1) : (z += 0.25) {
            while (y <= 1) : (y += 0.25) {
                while (x <= 1) : (x += 0.25) {
                    cube_points[num_initialized] = Vec3{ x, y, z };
                    num_initialized += 1;
                }
                x = -1;
            }
            y = -1;
        }
        std.debug.assert(num_initialized == num_cube_points);
    }

    const animate = false;
    const min_fov_factor: f32 = 0.0;
    const max_fov_factor: f32 = 120.0;
    var fov_factor_speed: f32 = 1.0;
    var fov_factor: f32 = max_fov_factor;

    game_loop: while (true) {
        // poll events
        {
            var event: c.SDL_Event = undefined;
            while (c.SDL_PollEvent(&event) == 1) {
                //
                switch (event.type) {
                    c.SDL_QUIT => {
                        break :game_loop;
                    },
                    c.SDL_KEYDOWN => {
                        const keyboard_event: c.SDL_KeyboardEvent = event.key;
                        if (keyboard_event.keysym.sym == c.SDLK_ESCAPE) {
                            break :game_loop;
                        }
                    },
                    else => {},
                }
                //
            }
        }

        // update
        //
        var projected_cube_points: [num_cube_points]Vec2 = undefined;
        {
            for (0..cube_points.len) |index| {
                const cube_point: Vec3 = cube_points[index];
                const projected_point: Vec2 = projectOrtho(cube_point, fov_factor);
                projected_cube_points[index] = projected_point;
            }
        }

        // render
        {
            drawGrid(window, 10, 10, Color.grey);
            // drawGrid(window, 100, 100, Color.yellow);
            // drawRect(window, -2000, -1000, 4000, 5000, Color.grey);


            for (projected_cube_points) |projected_point| {
                var pixel_loc_x: i32 = @intFromFloat(projected_point[0]);
                var pixel_loc_y: i32 = @intFromFloat(projected_point[1]);

                pixel_loc_x += @as(i32, @intCast(window.width / 2));
                pixel_loc_y += @as(i32, @intCast(window.height / 2));

                drawRect(
                    window,
                    pixel_loc_x - 2,
                    pixel_loc_y - 2,
                    4, // width
                    4, // height
                    Color.yellow,
                );
            }

            try window.present();

            if (animate) {
                if (fov_factor < min_fov_factor or fov_factor >= max_fov_factor) {
                    fov_factor_speed = -fov_factor_speed;
                }
                fov_factor += fov_factor_speed;
            }
        }
    }
}