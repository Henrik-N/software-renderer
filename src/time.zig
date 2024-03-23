const std = @import("std");
const Instant = std.time.Instant;

pub const Time = struct {
    cached_now: Instant,
    last_time: Instant,
    delta_nanoseconds: u64,
    delta_seconds: f32,

    pub fn init() Time {
        const now = Instant.now() catch unreachable;
        return Time{
            .cached_now = now,
            .last_time = now,
            .delta_nanoseconds = 0,
            .delta_seconds = 0,
        };
    }

    pub fn tick(self: *Time) void {
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
