#ifndef XHBLIB_TIMER_H_
#define XHBLIB_TIMER_H_

#include <chrono>
#include <atomic>
#include <utiltiy>
#include <functional>

#include <boost/intrusive/list>

#include "utility/optional.h"
#include "time_liner.h"

namespace xhb {

template<typename Clock = std::chrono::steady_clock>
class timer {
public:
    // 特征
    using clock = Clock;
    using time_point = Clock::time_point;
    using duration = Clock::duration;
private:
    using callback_t = std::function<void()>;
    namespace bi = boost::intrusive;
    bi::list_member_hook<> _hook;

    callback_t _cb; 
    time_point _expiry;
    optional<time_point> _period;
    bool _armed = false;
    bool _queued = false;
    bool _expired = false;
    void readd_periodic();
    void arm_helper(time_point until, optional<duration> period) {
        assert(_armed == false);
        _period = period;
        _armed = true;
        _expired = false;
        _expiry = until;
        _queued = true;
    }
public:
    timer() = default;
    timer(timer&& t) noexcept : _cb(std::move(t._cb)), 
        _expiry(std::move(t._expiry)), 
        _period(std::move(t._period)),
        _armed(t._armed), _queued(t._queued), _expired(t._expired) {
            _hook.swap_nodes(t._hook);
            t._armed = false;
            t._queued = false;
            t._expired = false;
        }
    explicit timer(callback_t&& cb) : _cb(std::move(cb)) {}
    ~timer();
    void set_callback(callback_t&& cb) {
        _cb = std::move(cb);
    }
    void arm(time_point until, optional<duration> period = {});
    void rearm(time_point until, optional<duration> period = {});
    void arm(duration deferral) {
        arm(clock::now() + deferral);
    }
    void arm_periodic(duration deferral) {
        arm(clock::now() + deferral, optional<duration>{deferral});
    }
    bool armed() const { return _armed; }
    bool cancel();
    time_point get_timeout() const {
        return _expiry;
    }
    friend class xhb::time_liner<timer, &timer::_hook>;
};

} // xhb namespace 
#endif // XHBLIB_TIMER_H_