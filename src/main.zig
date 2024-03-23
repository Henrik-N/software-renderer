const std = @import("std");
const builtin = @import("builtin");

const Window = @import("window.zig").Window;
const Color = @import("color.zig").Color;
const Mesh = @import("mesh.zig").Mesh;
const Time = @import("time.zig").Time;

const math = @import("math.zig");
const Vec2 = math.Vec2;
const Vec3 = math.Vec3;
const Vec2i = math.Vec2i;

const X = 0;
const Y = 1;
const Z = 2;

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

const ProjectedFace = [3]Vec2;

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
    should_cull_face: []bool,

    fn deinit(app: *Application) void {
        app.cube_mesh.deinit();
        app.ally.free(app.should_cull_face);
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

        self.camera_position = Vec3{ 0, 0, 0 };

        self.t = 0.0;

        return self;
    }

    fn run(app: *Application) !void {
        try app.setup();

        while (true) {
            if (!app.window.pollEvents()) {
                break;
            }
            app.time.tick();
            app.update();
            app.render();
            try app.window.present();
        }
    }

    fn setup(app: *Application) !void {
        app.cube_mesh = try Mesh.loadFromObj(app.ally, "assets/cube.obj");
        errdefer app.cube_mesh.deinit();

        const num_faces = app.cube_mesh.faces.items.len;

        app.projected_faces = try app.ally.alloc(ProjectedFace, num_faces);
        errdefer app.ally.free(app.projected_faces);

        app.should_cull_face = try app.ally.alloc(bool, num_faces);
        errdefer app.ally.free(app.should_cull_face);
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

            // transform
            //
            for (&face_corners) |*corner| {
                corner.* = math.rotX(corner.*, std.math.tau * app.t);
                corner.* = math.rotY(corner.*, std.math.tau * app.t);
                corner.* = math.rotZ(corner.*, std.math.tau * app.t);
                corner[Z] += 5;
            }

            // cull
            // NOTE: This is a temporary and inaccurate culling solution
            //
            const face_normal_not_normalized = math.vec3Cross(face_corners[1] - face_corners[0], face_corners[2] - face_corners[0]);
            const ray_to_face_corner = face_corners[0] - app.camera_position;

            const should_cull = math.vec3Dot(face_normal_not_normalized, ray_to_face_corner) > 0;
            app.should_cull_face[face_index] = should_cull;
            if (should_cull) {
                continue;
            }

            // project
            //
            const projected_face: *ProjectedFace = &app.projected_faces[face_index];

            for (face_corners, 0..) |corner, corner_index| {
                const projected_corner: *Vec2 = &projected_face[corner_index];
                projected_corner.* = projectPoint(corner, app.fov_factor);
                projected_corner[X] += @floatFromInt(app.window.width / 2);
                projected_corner[Y] += @floatFromInt(app.window.height / 2);
            }
        }
    }

    fn render(app: Application) void {
        drawGrid(app.window, 10, 10, Color.grey);

        for (app.projected_faces, 0..) |projected_face, face_index| {
            const should_cull = app.should_cull_face[face_index];
            if (should_cull) {
                continue;
            }

            const corner_a: Vec2i = math.vec2iFromVec2(projected_face[0]);
            const corner_b: Vec2i = math.vec2iFromVec2(projected_face[1]);
            const corner_c: Vec2i = math.vec2iFromVec2(projected_face[2]);

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
