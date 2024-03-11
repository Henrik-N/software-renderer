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

fn drawLine(window: Window, from: Vec2i, to: Vec2i, color: Color) void {
    const delta: Vec2i = to - from;
    const largest_side_len = @max(@abs(delta[X]), @abs(delta[Y]));

    const step = Vec2{
        @as(f32, @floatFromInt(delta[X])) / @as(f32, @floatFromInt(largest_side_len)),
        @as(f32, @floatFromInt(delta[Y])) / @as(f32, @floatFromInt(largest_side_len)),
    };

    var current_coord = Vec2{ @floatFromInt(from[X]), @floatFromInt(from[Y]) };

    var i: isize = 0;
    while (i < largest_side_len) : (i += 1) {
        const coord: Vec2 = @round(current_coord);
        if (coord[X] < 0.0 or coord[Y] < 0.0) {
            continue;
        }

        const coord_x_usize: usize = @intFromFloat(coord[X]);
        const coord_y_usize: usize = @intFromFloat(coord[Y]);
        if (coord_x_usize >= window.width or coord_y_usize >= window.height) {
            continue;
        }

        window.setPixel(coord_x_usize, coord_y_usize, color);

        current_coord += step;
    }
}

const Vec2 = @Vector(2, f32);
const Vec3 = @Vector(3, f32);
const Vec2i = @Vector(2, i32);

fn vec2iFromVec2(vec2: Vec2) Vec2i {
    return Vec2i{ @intFromFloat(vec2[X]), @intFromFloat(vec2[Y]) };
}

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

const Mesh = struct {
    vertices: std.ArrayList(Vec3),
    faces: std.ArrayList(Face),

    fn deinit(mesh: Mesh) void {
        mesh.faces.deinit();
        mesh.vertices.deinit();
    }

    // Assumes the mesh uses triangle faces
    fn loadFromObj(ally: std.mem.Allocator, filename: []const u8) !Mesh {
        var mesh = Mesh{
            .vertices = std.ArrayList(Vec3).init(ally),
            .faces = std.ArrayList(Face).init(ally),
        };
        errdefer mesh.deinit();

        var file = try std.fs.cwd().openFile(filename, .{}); // TODO: Don't use cwd
        defer file.close();

        var buffered_reader = std.io.bufferedReader(file.reader());
        const reader = buffered_reader.reader();

        var line = std.ArrayList(u8).init(ally);
        defer line.deinit();

        while (reader.streamUntilDelimiter(line.writer(), '\n', null)) {
            defer line.clearRetainingCapacity();

            var it: std.mem.SplitIterator(u8, .sequence) = std.mem.split(u8, line.items, " ");
            var word: []const u8 = it.next() orelse continue;

            const is_vertex_entry = std.mem.eql(u8, word, "v");
            if (is_vertex_entry) {
                var vertex: Vec3 = undefined;
                for (0..3) |k| {
                    word = it.next() orelse return error.ObjVertexEntry;
                    vertex[k] = try std.fmt.parseFloat(f32, word);
                }
                try mesh.vertices.append(vertex);
                continue;
            }

            const is_face_entry = std.mem.eql(u8, word, "f");
            if (is_face_entry) {
                var face: Face = undefined;
                for (0..3) |k| {
                    word = it.next() orelse return error.ObjFaceCorner;
                    var face_entries_it = std.mem.split(u8, word, "/");
                    const vertex_index: []const u8 = face_entries_it.next() orelse return error.ObjFaceEntry;
                    const base = 10;
                    face[k] = try std.fmt.parseInt(u16, vertex_index, base);
                    face[k] -= 1; // NOTE obj files start vertex indices at 1
                }
                try mesh.faces.append(face);
                continue;
            }
        } else |err| switch (err) {
            error.EndOfStream => {},
            else => return err,
        }

        return mesh;
    }
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

    cube_mesh: Mesh,
    projected_faces: []ProjectedFace,

    fn deinit(app: *Application) void {
        app.cube_mesh.deinit();
        app.ally.free(app.projected_faces);
        app.window.deinit();
    }

    fn init(ally: std.mem.Allocator) !Application {
        var self: Application = undefined;
        self.ally = ally;
        self.projected_faces = &.{};

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
        try app.setup();

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

    fn setup(app: *Application) !void {
        app.cube_mesh = try Mesh.loadFromObj(app.ally, "assets/cube.obj");
        errdefer app.cube_mesh.deinit();

        app.projected_faces = try app.ally.alloc(ProjectedFace, app.cube_mesh.faces.items.len);
        errdefer app.ally.free(app.projected_faces);
    }

    fn update(app: *Application) void {
        if (app.animate.enable) {
            if (app.fov_factor < app.animate.min or app.fov_factor >= app.animate.max) {
                app.animate.speed = -app.animate.speed;
            }
            app.fov_factor += app.animate.speed * app.time.delta_seconds;
        }

        app.t += app.time.delta_seconds * 0.1;

        for (app.cube_mesh.faces.items, 0..) |face, face_index| {
            var face_corners = [3]Vec3{
                app.cube_mesh.vertices.items[face[0]],
                app.cube_mesh.vertices.items[face[1]],
                app.cube_mesh.vertices.items[face[2]],
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
            const corner_a: Vec2i = vec2iFromVec2(projected_face[0]);
            const corner_b: Vec2i = vec2iFromVec2(projected_face[1]);
            const corner_c: Vec2i = vec2iFromVec2(projected_face[2]);

            drawLine(app.window, corner_a, corner_b, Color.white);
            drawLine(app.window, corner_b, corner_c, Color.white);
            drawLine(app.window, corner_c, corner_a, Color.white);
        }
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
