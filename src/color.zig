pub const Color = packed union {
    hex: u32,
    channels: packed struct {
        a: u8,
        r: u8,
        g: u8,
        b: u8,
    },

    pub inline fn hex(hex_: u32) Color {
        return .{ .hex = hex_ };
    }

    pub inline fn rgba(r: u8, g: u8, b: u8, a: u8) Color {
        return .{ .channels = .{ .a = a, .r = r, .g = g, .b = b } };
    }

    pub const black = Color.hex(0xFF_00_00_00);
    pub const white = Color.hex(0xFF_FF_FF_FF);
    pub const grey = Color.hex(0xFF_33_33_33);
    pub const red = Color.hex(0xFF_FF_00_00);
    pub const green = Color.hex(0xFF_00_FF_00);
    pub const blue = Color.hex(0xFF_00_00_FF);
    pub const yellow = Color.hex(0xFF_FF_FF_00);
};

test "color union" {
    const std = @import("std");

    var colors: [3]u32 = undefined;
    colors[0] = @bitCast(Color.hex(0xFF0000FF));
    colors[1] = @bitCast(Color.rgba(0, 0, 255, 255));
    colors[2] = 0xFF0000FF;

    try std.testing.expect(std.mem.eql(u32, &.{colors[0]}, &.{colors[1]}));
    try std.testing.expect(std.mem.eql(u32, &.{colors[1]}, &.{colors[2]}));
}
