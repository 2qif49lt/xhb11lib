#ifndef XHBLIB_THREAD_H_
#define XHBLIB_THREAD_H_

#include <setjmp.h>

#include <memory>
#include <chrono>
#include <type_traits>
#include <functional>

#include <boost/intrusive/list.hpp>

#include "utility/optional.h"
#include "timer.h"
#include "future.h"
#include "reactor.h"
#include "thread_impl.h"

namesapce xhb {

namespace bi = boost::intrusive;

class thread;
class thread_attributes;
class thread_scheduling_group;

class thread_attributes {
public:
    thread_scheduling_group* _group = nullptr;
};

extern thread_local jmp_buf_link g_unthreaded_context;

// 线程上下文
class thread_context {
    struct stack_deleter {
        void operator()(char* ptr) const {
            delete [] ptr;
        }
    };
    using stack_holder = std::unique_ptr<char[], stack_deleter>;

    bi::list_member_hook<> _preempted_link;
    bi::list_member_hook<> _all_link;

    thread_attributes _attr;
    static constexpr size_t _stack_size = 128 * 1024;
    stack_holder _stack{make_stack()};
    std::function<void()> _func;
    jmp_buf_link _context;
    promise<> _done;
    bool _joined = false;
    timer<> _sched_timer{[this] { reschedule(); }};
    optional<promise<>> _sched_promise;

    using preempted_thread_list = bi::list<thread_context,
        bi::member_hook<thread_context, bi::list_member_hook<>,
        &thread_context::_preempted_link>,
        bi::constant_time_size<false>>;
    using all_thread_list = bi::list<thread_context,
        bi::member_hook<thread_context, bi::list_member_hook<>,
        &thread_context::_all_link>,
        bi::constant_time_size<false>>;
    
    static thread_local preempted_thread_list _preempted_threads;
    static thread_local all_thread_list _all_threads;

private:
    static stack_holder make_stack();
    static void s_main(unsigned int lo, unsigned int hi);
    void setup();
    void main();
public:
    thread_context(thread_attributes attr, std::function<void ()> func);
    ~thread_context();
    void switch_in();
    void switch_out();
    bool should_yield() const;
    void reschedule();
    void yield();
    friend class thread;
    friend void thread_impl::switch_in(thread_context*);
    friend void thread_impl::switch_out(thread_context*);
};


class thread {
    std::unique_ptr<thread_context> _context;
    static thread_local thread* _current;
public:
    thread() = default;
    
    template <typename F>
    thread(F func);

    template <typename f>
    thread(thread_attributes attr, f func);

    thread(thread&& other) noexcept = default;
    thread& operator=(thread&& other) noexcept = default;

    
    ~thread() { assert(!_context || _context->_joined); }

    future<> join();

    static void yield();
    static bool should_yield();

    static bool running_in_thread() {
        return thread_impl::get() != nullptr;
    }
private:
    friend class reactor;
    static bool try_run_one_yielded_thread();
};

// 线程调度
class thread_scheduling_group {
    std::chrono::nanoseconds _period;
    std::chrono::nanoseconds _quota;
    std::chrono::time_point<thread_clock> _this_period_ends = {};
    std::chrono::time_point<thread_clock> _this_run_start = {};
    std::chrono::nanoseconds _this_period_remain = {};
public:
    ///
    /// \param period a duration representing the period
    /// \param usage which fraction of the \c period to assign for the scheduling group. Expected between 0 and 1.
    thread_scheduling_group(std::chrono::nanoseconds period, float usage);
    /// \brief changes the current maximum usage per period
    ///
    /// \param new_usage The new fraction of the \c period (Expected between 0 and 1) during which to run
    void update_usage(float new_usage) {
        _quota = std::chrono::duration_cast<std::chrono::nanoseconds>(new_usage * _period);
    }
private:
    void account_start();
    void account_stop();
    optional<thread_clock::time_point> next_scheduling_point() const;
    friend class thread_context;
};

} // xhb namespace
#endif // XHBLIB_THREAD_H_