#ifndef XHBLIB_LOWRES_CLOCK_H_
#define XHBLIB_LOWRES_CLOCK_H_


namespace xhb {

#include <chrono>
#include <atomic>
#include "timer.h"

class lowres_clock {
public:
    typedef int64_t rep;
    // The lowres_clock's resolution is 10ms. However, to make it is easier to
    // do calcuations with std::chrono::milliseconds, we make the clock's
    // period to 1ms instead of 10ms.
    typedef std::ratio<1, 1000> period;
    typedef std::chrono::duration<rep, period> duration;
    typedef std::chrono::time_point<lowres_clock, duration> time_point;
    lowres_clock() {
        update();
        _timer.set_callback([this] { update(); });
        _timer.arm_periodic(_granularity);
    }
    static time_point now() {
        auto nr = _now.load(std::memory_order_relaxed);
        return time_point(duration(nr));
    }
private:
    static void update() {
        using namespace std::chrono;
        auto now = steady_clock_type::now();
        auto ticks = duration_cast<milliseconds>(now.time_since_epoch()).count();
        _now.store(ticks, std::memory_order_relaxed);
    }
    // _now is updated by cpu0 and read by other cpus. Make _now on its own
    // cache line to avoid false sharing.
    static alignas(64) std::atomic<rep> _now;
    // High resolution timer to drive this low resolution clock
    timer<> _timer;
    // High resolution timer expires every 10 milliseconds
    static constexpr std::chrono::milliseconds _granularity{10};
};


} // xhb namespace
#endif // XHBLIB_LOWRES_CLOCK_H_