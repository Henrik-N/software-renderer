#include "_time.h"


// =====================================================================================================================
// == Time =============================================================================================================
// =====================================================================================================================

Time_System::Time_System()
    : delta_nanoseconds(),
      delta_seconds(0) {
}


void Time_System::update()
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

f32 Time_System::get_delta_seconds() const {
    return delta_seconds;
}
