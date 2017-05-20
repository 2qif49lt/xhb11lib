#ifndef XHBLIB_TIME_LINE_H_
#define XHBLIB_TIME_LINE_H_

// time_liner 管理计时器

#include <chrono>
#include <bitset>
#include <array>
#include <limit>
#include <algorithm>
#include <exception>
#include <boost/intrusive/list>
#include "utility/bitopt.h"

namespace xhb {

namespace bi = boost::intrusive;

// T 是一个时间锦囊
// 需要有 time_point T::get_timeout() 成员函数表示过期时间点。
template<typename T /*Timer*/, bi::list_member_hook<> T::*hook>
class time_liner {
public:
    using timer_list_t = bi::list<T, bi::member_hook<T, bi::list_member_hook<>, hook>>;
    using time_point = typename T::time_point;
    using duration = typename T::duration;

private:
    using timestamp_t = typename T::duration::rep;
    static constexpr timestamp_t max_ts = std::numeric_limits<timestamp_t>::max();
    static constexpr unsigned int n_list = std::numeric_limits<timestamp_t>::digits + 1;

    std::array<timer_list_t, n_list> _lists;
    timestamp_t _last; // last timestamp expire
    timestamp_t _next; // next timestamp to be expire
    std::bitset<n_list> _bits; // 对应array 
private:
    static timestamp_t get_timestamp(time_point tp) {
        return tp.time_since_epoch().count();
    }
    static timestamp_t get_timestamp(T& timer) {
        return get_timestamp(timer.get_timeout());
    }
    unsigned int get_index(timestamp_t ts) const {
        if (ts <= _last) {
            return n_list - 1;
        }
        // 按异或后的前导0个数排序.
        // _last 始终为所有时间中最小的，在_last变化后插入的值大于_last且小于min(最右索引)将被索引到min(最右索引)右边。
        // 不同的_last的值对应ts的索引始终是不变的。
        return clz(ts ^ _last);
    }
    unsigned int get_index(T& timer) const {
        return get_index(get_timestamp(timer));
    }

    // 获取_lists不为空的最右索引值
    unsigned int get_last_non_empty_index() const {
        return std::numeric_limits<timestamp_t>::digits - 1 - clz(_bits.to_ulong());
    }

public:
    time_liner() : _last(0), _next(max_ts), _bits(0) {}
    ~time_liner() {
        for (auto& l : _lists) {
            while (!l.empty()) {
                auto& timer = *list.begin();
                l.pop_front();
                timer.cancel();
            }
        }
    }
    
    // 返回true时，如果需要程序执行保证是按时间顺序进行则需要重新安排expire(),在expire()调用之前调用get_next_expire()
    bool insert(T& timer) {
        auto ts = get_timestamp(timer);
        auto idx = get_index(timer);

        _lists[idx].push_back(timer);
        _bits.set(idx);

        if (ts < _next) {
            _next = ts;
            return true;
        }
        return false;
    }

    time_point get_next_expire() const{
        return time_point(duration(std::max(_last, _next)));
    }

    void remove(T& timer) {
        auto idx = get_index(timer);
        auto& l = _lists[idx];

        l.erase(list.iterator_to(timer));
        if (l.empty()) {
            _bits.reset(idx);
        }
    }

    timer_list_t expire(time_point now) {
        timer_list_t ret;
        auto ts = get_timestamp(now);

        if (ts < _last) {
            throw runtime_error("expire receive a past time point");
        }

        auto idx = get_index(ts);
        // 将next及右侧小于next的所有全部收入ret.
        for (unsigned int pos = idx + 1; pos < n_list; ++pos) {
            if (_bits[pos]) {
                ret.splice(ret.end(), _lists[pos]);
                _bits[pos] = false;
            }
        }
        _last = ts;
        _next = max_ts;

        // 将当前索引里的
        auto& l = _lists[idx];
        while (!l.empty()) {
            auto& timer = l.front();
            l.pop_front();
            if (timer.get_timeout() <= now) {
                ret.push_back(timer);
            } else {
                // 与now(也就是更新后的_last)同一个索引里的值再次进行索引后至少会右移一位。所以不会导致死循环
                insert(timer); 
            }
        }

        _bits[idx] = !l.empty();

        if (_next == max_ts && _bits.any()) {
            for (auto& timer : _lists[get_last_non_empty_index()]) {
                _next = std::min(_next, get_timestamp(timer));
            }
        }
        return ret;
    }

    void clear() {
        for (unsigned int pos = 0; pos < n_list; ++pos) {
            if (_bits[pos]) {
                _lists[pos].clear();
                _bits[pos] = false;
            }
        }
    }
    
    size_t size() const {
        size_t ret;
        for (unsigned int pos = 0; pos < n_list; ++pos) {
            if (_bits[pos]) {
                ret += _lists[pos].size();
            }
        }
        return ret;
    }
    
    bool empty() const {
        return _bits.none();
    }

    time_point now() {
        return T::clock::now();
    }
};

} // xhb namespace

#endif // XHBLIB_TIME_LINE_H_