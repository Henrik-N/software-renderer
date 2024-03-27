#include "_time.h"

#include <iostream>

// =====================================================================================================================
// == Time =============================================================================================================
// =====================================================================================================================

Time::Time()
    : delta_nanoseconds(),
      delta_seconds(0) {
}


void Time::init(Time& time) {
    time = Time{};
}


void Time::tick()
{
    if (const Instant now = Clock::now();
        now > cached_now) {
        cached_now = now;
    }

    const auto dt_duration = cached_now - last_time;
    last_time = cached_now;

    delta_nanoseconds = std::chrono::duration_cast<Duration_NanoSeconds>(dt_duration).count();
    delta_seconds = static_cast<f32>(delta_nanoseconds) / static_cast<float>(ns_per_s());
}
