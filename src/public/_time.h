#pragma once
#include "_common.h"
#include "_ecs.h"

#include <chrono>


struct Time_System final : System {
    explicit Time_System();
    void update();

    f32 get_delta_seconds() const;

private:
    using Clock = std::chrono::high_resolution_clock;
    using Instant = std::chrono::time_point<std::chrono::system_clock>;
    using Duration_NanoSeconds = std::chrono::duration<u64, std::nano>;

    Instant cached_now;
    Instant last_time;
    u64 delta_nanoseconds;
    f32 delta_seconds;

    consteval static i32 ns_per_us() { return 1000; } // ns/µs, nanoseconds per microsecond
    consteval static i32 ns_per_ms() { return 1000 * ns_per_us(); } // ns/ms, nanoseconds per millisecond
    consteval static i32 ns_per_s() { return 1000 * ns_per_ms(); } // ns/s, nanoseconds per second
};
