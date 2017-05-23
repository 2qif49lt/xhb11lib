#include  "utility/optional.h"
#include "reactor.h"
#include "timer.h"
#include "lowres_clock.h"

namespace xhb {

template <typename Clock>
timer<Clock>::~timer() {
    if (_queued) {
        engine().del_timer(this);
    }
}
template <typename Clock>
void timer<Clock>::readd_periodic() {
    arm_helper(Clock::now() + _period.value(), {_period.value()});
    engine().queue_timer(this);
}

template<typename Clock>
void timer<Clock>::arm(time_point until, optional<duration> period) {
    arm_helper(until, period);
    engine().add_timer(this);
}
template <typename Clock>
void timer<Clock>::rearm(time_point until, std::experimental::optional<duration> period) {
    if (_armed) {
        cancel();
    }
    arm(until, period);
}
template<typename Clock>
bool timer<Clock>::cancel() {
    if (!_armed) {
        return false;
    }
    _armed = false;
    if (_queued) {
        engine().del_timer(this);
        _queued = false;
    }
    return true;
}

// lowres_clock

std::atomic<lowres_clock::rep> lowres_clock::_now;

} // xhb namesapce