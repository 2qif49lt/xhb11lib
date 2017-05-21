#include <cstdint>
#include <cerrno>

#include <system_error>

#include "thread.h"

namespace xhb {

thread_local jmp_buf_link g_unthreaded_context;
thread_local jmp_buf_link* g_current_context;


// hmp_buf_link 

void jmp_buf_link::initial_switch_in(ucontext_t* initial_context, const void*, size_t)
{
    auto prev = std::exchange(g_current_context, this);
    _link = prev;
    if (setjmp(prev->_jmpbuf) == 0) {
        setcontext(initial_context);
    }
}

inline void jmp_buf_link::switch_in()
{
    auto prev = std::exchange(g_current_context, this);
    _link = prev;
    if (setjmp(prev->_jmpbuf) == 0) {
        longjmp(_jmpbuf, 1);
    }
}

inline void jmp_buf_link::switch_out()
{
    g_current_context = _link;
    if (setjmp(_jmpbuf) == 0) {
        longjmp(g_current_context->_jmpbuf, 1);
    }
}

inline void jmp_buf_link::initial_switch_in_completed()
{
}

inline void jmp_buf_link::final_switch_out()
{
    g_current_context = _link;
    longjmp(g_current_context->_jmpbuf, 1);
}


// thread_conext

thread_local thread_context::preempted_thread_list thread_context::_preempted_threads;
thread_local thread_context::all_thread_list thread_context::_all_threads;

thread_context::stack_holder thread_context::make_stack() {
    return stack_holder(new char[_stack_size]);
}
void thread_context::s_main(unsigned int lo, unsigned int hi) {
    uintptr_t q = lo | (uint64_t(hi) << 32);
    reinterpret_cast<thread_context*>(q)->main();
}

void thread_context::setup() {
    // use setcontext() for the initial jump, as it allows us
    // to set up a stack, but continue with longjmp() as it's
    // much faster.
    ucontext_t initial_context;
    auto q = uint64_t(reinterpret_cast<uintptr_t>(this));
    auto main = reinterpret_cast<void (*)()>(&thread_context::s_main);
    auto r = getcontext(&initial_context);
  //  throw_system_error_on(r == -1);
    if (r == -1) {
        throw std::system_error(errno, std::system_category());
    }
    initial_context.uc_stack.ss_sp = _stack.get();
    initial_context.uc_stack.ss_size = _stack_size;
    initial_context.uc_link = nullptr;
    makecontext(&initial_context, main, 2, int(q), int(q >> 32));
    _context._thread = this;
    _context.initial_switch_in(&initial_context, _stack.get(), _stack_size);
}

void thread_context::main() {
    _context.initial_switch_in_completed();
    if (_attr.scheduling_group) {
        _attr.scheduling_group->account_start();
    }
    try {
        _func();
        _done.set_value();
    } catch (...) {
        _done.set_exception(std::current_exception());
    }
    if (_attr.scheduling_group) {
        _attr.scheduling_group->account_stop();
    }

    _context.final_switch_out();
}

thread_context::thread_context(thread_attributes attr, std::function<void ()> func)
        : _attr(std::move(attr))
        , _func(std::move(func)) {
    setup();
    _all_threads.push_front(*this);
}

thread_context::~thread_context() {
    _all_threads.erase(_all_threads.iterator_to(*this));
}

void thread_context::switch_in() {
    if (_attr.scheduling_group) {
        _attr.scheduling_group->account_start();
        _context._yield_at = _attr.scheduling_group->_this_run_start + _attr.scheduling_group->_this_period_remain;
    } else {
        _context._yield_at = {};
    }
    _context.switch_in();
}

void thread_context::switch_out() {
    if (_attr.scheduling_group) {
        _attr.scheduling_group->account_stop();
    }
    _context.switch_out();
}

bool thread_context::should_yield() const {
    if (!_attr.scheduling_group) {
        return need_preempt();
    }
    return need_preempt() || bool(_attr.scheduling_group->next_scheduling_point());
}

void thread_context::reschedule() {
    _preempted_threads.erase(_preempted_threads.iterator_to(*this));
    _sched_promise->set_value();
}

void thread_context::yield() {
    if (!_attr.scheduling_group) {
        later().get();
    } else {
        auto when = _attr.scheduling_group->next_scheduling_point();
        if (when) {
            _preempted_threads.push_back(*this);
            _sched_promise.emplace();
            auto fut = _sched_promise->get_future();
            _sched_timer.arm(*when);
            fut.get();
            _sched_promise = nullopt;
        } else if (need_preempt()) {
            later().get();
        }
    }
}

// thread

template <typename F>
thread::thread(F func) : thread(thread_attributes(), std::move(func)) {
}

template <typename F>
thread::thread(thread_attributes attr, F func)
        : _context(std::make_unique<thread_context>(std::move(attr), func)) {
}

future<> thread::join() {
    _context->_joined = true;
    return _context->_done.get_future();
}

void thread::yield() {
    thread_impl::get()->yield();
}
bool thread::should_yield() {
    return thread_impl::get()->should_yield();
}
bool thread::try_run_one_yielded_thread() {
    if (thread_context::_preempted_threads.empty()) {
        return false;
    }
    auto&& t = thread_context::_preempted_threads.front();
    t._sched_timer.cancel();
    t._sched_promise->set_value();
    thread_context::_preempted_threads.pop_front();
    return true;
}

// thread_scheduling_group

thread_scheduling_group::thread_scheduling_group(std::chrono::nanoseconds period, float usage)
        : _period(period), _quota(std::chrono::duration_cast<std::chrono::nanoseconds>(usage * period)) {
}

void thread_scheduling_group::account_start() {
    auto now = thread_clock::now();
    if (now >= _this_period_ends) {
        _this_period_ends = now + _period;
        _this_period_remain = _quota;
    }
    _this_run_start = now;
}

void thread_scheduling_group::account_stop() {
    _this_period_remain -= thread_clock::now() - _this_run_start;
}

optional<thread_clock::time_point> thread_scheduling_group::next_scheduling_point() const {
    auto now = thread_clock::now();
    auto current_remain = _this_period_remain - (now - _this_run_start);
    if (current_remain > std::chrono::nanoseconds(0)) {
        return nullopt;
    }
    return _this_period_ends - current_remain;

}


namespace thread_impl {

void yield() {
    g_current_context->thread->yield();
}

void switch_in(thread_context* to) {
    to->switch_in();
}

void switch_out(thread_context* from) {
    from->switch_out();
}

void init() {
    g_unthreaded_context.link = nullptr;
    g_unthreaded_context.thread = nullptr;
    g_current_context = &g_unthreaded_context;
}

}

} // xhb namespace