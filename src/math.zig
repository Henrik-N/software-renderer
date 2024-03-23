const std = @import("std");

pub const Vec2 = @Vector(2, f32);
pub const Vec3 = @Vector(3, f32);
pub const Vec2i = @Vector(2, i32);

const X = 0;
const Y = 1;
const Z = 2;

pub fn vec3Normalized(vec: Vec3) Vec3 {
    const magnitude = vec3Magnitude(vec);
    if (magnitude <= 0) {
        return Vec3{ 0, 0, 0 };
    }
    return vec / @as(Vec3, @splat(magnitude));
}

pub fn vec3Dot(a: Vec3, b: Vec3) f32 {
    return a[X] * b[X] + a[Y] * b[Y] + a[Z] * b[Z];
}

pub fn vec3Cross(a: Vec3, b: Vec3) Vec3 {
    return Vec3{
        a[Y] * b[Z] - a[Z] * b[Y],
        a[Z] * b[X] - a[X] * b[Z],
        a[X] * b[Y] - a[Y] * b[X],
    };
}

pub fn vec3SqMagnitude(vec: Vec3) f32 {
    return vec[X] * vec[X] + vec[Y] * vec[Y] + vec[Z] * vec[Z];
}

pub fn vec3Magnitude(vec: Vec3) f32 {
    return std.math.sqrt(vec3SqMagnitude(vec));
}

pub fn vec2iFromVec2(vec2: Vec2) Vec2i {
    return Vec2i{ @intFromFloat(vec2[X]), @intFromFloat(vec2[Y]) };
}

pub fn rotX(vec: Vec3, angle: f32) Vec3 {
    return Vec3{
        vec[X],
        vec[Y] * @cos(angle) + vec[Z] * @sin(angle),
        vec[Z] * @cos(angle) - vec[Y] * @sin(angle),
    };
}

pub fn rotY(vec: Vec3, angle: f32) Vec3 {
    return Vec3{
        vec[X] * @cos(angle) - vec[Z] * @sin(angle),
        vec[Y],
        vec[Z] * @cos(angle) + vec[X] * @sin(angle),
    };
}

pub fn rotZ(vec: Vec3, angle: f32) Vec3 {
    return Vec3{
        vec[X] * @cos(angle) - vec[Y] * @sin(angle),
        vec[Y] * @cos(angle) + vec[X] * @sin(angle),
        vec[Z],
    };
}
