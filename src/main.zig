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

fn projectPoint(point: Vec3, fov_factor: f32) Vec2 {
    // At this point, everything will be presented 1:1.
    // If the position was (1, 1, 1) and we present it as is, the final pixel location will be (1, 1)
    var coord = Vec2{ point[0], point[1] };

    // With a fov factor of 100, the final pixel location will be (100, 100)
    coord *= @as(Vec2, @splat(fov_factor));

    // Larger z-value => smaller displacement from the origin
    const z = if (point[2] != 0) point[2] else 0.1;
    coord /= @as(Vec2, @splat(z));

    // Positive Y up
    coord[1] = -coord[1];

    return coord;
}

const X = 0;
const Y = 1;
const Z = 2;

fn rotX(vec: Vec3, angle: f32) Vec3 {
    return Vec3{
        vec[X],
        vec[Y] * @cos(angle) + vec[Z] * @sin(angle),
        vec[Z] * @cos(angle) - vec[Y] * @sin(angle),
    };
}

fn rotY(vec: Vec3, angle: f32) Vec3 {
    return Vec3{
        vec[X] * @cos(angle) - vec[Z] * @sin(angle),
        vec[Y],
        vec[Z] * @cos(angle) + vec[X] * @sin(angle),
    };
}

fn rotZ(vec: Vec3, angle: f32) Vec3 {
    return Vec3{
        vec[X] * @cos(angle) - vec[Y] * @sin(angle),
        vec[Y] * @cos(angle) + vec[X] * @sin(angle),
        vec[Z],
    };
}

const Time = struct {
    const Instant = std.time.Instant;

    cached_now: Instant,
    last_time: Instant,
    delta_nanoseconds: u64,
    delta_seconds: f32,

    fn init() Time {
        const now = Instant.now() catch unreachable;
        return Time{
            .cached_now = now,
            .last_time = now,
            .delta_nanoseconds = 0,
            .delta_seconds = 0,
        };
    }

    fn tick(self: *Time) void {
        self.delta_nanoseconds = self.tickImpl();
        self.delta_seconds = @as(f32, @floatFromInt(self.delta_nanoseconds)) / std.time.ns_per_s;
    }

    fn tickImpl(self: *Time) u64 {
        const now = Instant.now() catch unreachable;
        if (now.order(self.cached_now) == .gt) {
            self.cached_now = now;
        }
        defer self.last_time = self.cached_now;
        return self.cached_now.since(self.last_time);
    }
};

const Face = @Vector(3, u16);

const cube_mesh = struct {
    // zig fmt: off
    const vertices = [_]Vec3{
        .{ -1, -1, -1 },
        .{  1, -1, -1 },
        .{  1,  1, -1 },
        .{ -1,  1, -1 },
        //
        .{ -1, -1,  1 },
        .{  1, -1,  1 },
        .{  1,  1,  1 },
        .{ -1,  1,  1 },
    };

    // counter-clockwise
    const faces = [_]Face{
        // front
        .{0, 1, 2},
        .{2, 3, 0},
        // right
        .{1, 5, 2},
        .{2, 5, 6},
        // top
        .{2, 6, 3},
        .{3, 6, 7},
        // left
        .{4, 0, 3},
        .{3, 7, 4},
        // bottom
        .{0, 4, 1},
        .{1, 4, 5},
        // back
        .{7, 6, 4},
        .{4, 6, 5}
    };

    // zig fmt: on
};

const ProjectedFace = [3]Vec2;

const Application = struct {
    const num_cube_points = 9 * 9 * 9;
    const num_axes_points = 10 * 3;
    const num_projected_points = num_cube_points + num_axes_points;

    ally: std.mem.Allocator,
    window: Window,
    time: Time,

    fov_factor: f32,
    animate: struct {
        enable: bool = false,
        min: f32,
        max: f32,
        speed: f32 = 100.0,
    },

    camera_position: Vec3,

    t: f32,

    projected_faces: [cube_mesh.faces.len]ProjectedFace,

    fn deinit(app: *Application) void {
        app.window.deinit();
    }

    fn init(ally: std.mem.Allocator) !Application {
        var self: Application = undefined;
        self.ally = ally;

        self.window = try Window.init(ally, null, null, false);
        errdefer self.window.deinit();

        self.time = Time.init();

        self.fov_factor = 600.0;
        self.animate = .{
            // .enable = true,
            .min = 0,
            .max = self.fov_factor,
        };

        self.camera_position = Vec3{ 0, 0, -5 };

        self.t = 0.0;

        return self;
    }

    fn run(app: *Application) !void {
        app.setup();

        while (true) {
            if (!app.pollEvents()) {
                break;
            }
            app.time.tick();
            app.update();
            app.render();
            try app.window.present();
        }
    }

    fn pollEvents(_: Application) bool {
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

    fn setup(_: *Application) void {}

    fn update(app: *Application) void {
        if (app.animate.enable) {
            if (app.fov_factor < app.animate.min or app.fov_factor >= app.animate.max) {
                app.animate.speed = -app.animate.speed;
            }
            app.fov_factor += app.animate.speed * app.time.delta_seconds;
        }

        app.t += app.time.delta_seconds * 0.1;

        for (cube_mesh.faces, 0..) |face, face_index| {
            var face_corners = [3]Vec3{
                cube_mesh.vertices[face[0]],
                cube_mesh.vertices[face[1]],
                cube_mesh.vertices[face[2]],
            };

            const projected_face: *ProjectedFace = &app.projected_faces[face_index];

            for (&face_corners, 0..) |*corner, corner_index| {
                corner.* = rotX(corner.*, std.math.tau * app.t);
                corner.* = rotY(corner.*, std.math.tau * app.t);
                corner.* = rotZ(corner.*, std.math.tau * app.t);

                corner[Z] -= app.camera_position[Z];

                const projected_corner: *Vec2 = &projected_face[corner_index];
                projected_corner.* = projectPoint(corner.*, app.fov_factor);
                projected_corner[X] += @floatFromInt(app.window.width / 2);
                projected_corner[Y] += @floatFromInt(app.window.height / 2);
            }
        }
    }

    fn render(app: Application) void {
        drawGrid(app.window, 10, 10, Color.grey);

        for (app.projected_faces) |projected_face| {
            for (projected_face) |projected_corner| {
                const pixel_loc_x: i32 = @intFromFloat(projected_corner[0]);
                const pixel_loc_y: i32 = @intFromFloat(projected_corner[1]);

                drawRect(
                    app.window,
                    pixel_loc_x - 2,
                    pixel_loc_y - 2,
                    4, // width
                    4, // height
                    Color.white,
                );
            }
        }
        //
    }
};

pub fn main() !void {
    var general_purpose_allocator = std.heap.GeneralPurposeAllocator(.{}){};
    defer {
        const did_leak = general_purpose_allocator.deinit() == .leak;
        if (did_leak) {
            std.log.err("Memory leak detected!", .{});
            std.debug.assert(false);
        }
    }

    const gpa = general_purpose_allocator.allocator(); // allocator interface

    var app = try Application.init(gpa);
    defer app.deinit();

    try app.run();
}
