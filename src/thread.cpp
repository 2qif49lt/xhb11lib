#include <cstdint>
#include <cerrno>

#include <system_error>

#include <unistd.h>

#include "thread.h"
namespace xhb {

thread_local jmp_buf_link g_unthreaded_context;
thread_local jmp_buf_link* g_current_context;

thread_local jmp_buf_link* g_previous_context;

void jmp_buf_link::initial_switch_in(ucontext_t* initial_context)
{
    auto prev = std::exchange(g_current_context, this);
    _link = prev;
    g_previous_context = prev;
    swapcontext(&prev->_context, initial_context);
}

inline void jmp_buf_link::switch_in()
{
    auto prev = std::exchange(g_current_context, this);
    _link = prev;
    g_previous_context = prev;
    swapcontext(&prev->_context, &_context);
}

inline void jmp_buf_link::switch_out()
{
    g_current_context = _link;
    g_previous_context = this;
    swapcontext(&_context, &g_current_context->_context);
}

inline void jmp_buf_link::initial_switch_in_completed()
{
}

inline void jmp_buf_link::final_switch_out()
{
    g_current_context = _link;
    g_previous_context = this;
    setcontext(&g_current_context->_context);
}


// thread_conext


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
    if (r == -1) {
        throw std::system_error(errno, std::system_category());
    }
    initial_context.uc_stack.ss_sp = _stack.get();
    initial_context.uc_stack.ss_size = _stack_size;
    initial_context.uc_link = nullptr;
    makecontext(&initial_context, main, 2, int(q), int(q >> 32));
    _context._thread = this;
    _context.initial_switch_in(&initial_context);
}

void thread_context::main() {
    _context.initial_switch_in_completed();
    try {
        _func();
        _done.set_value();
    } catch (...) {
        _done.set_exception(std::current_exception());
    }

    _context.final_switch_out();
}

thread_context::thread_context(std::function<void ()> func):_func(std::move(func)) {
    setup();
}

thread_context::~thread_context() {
}

void thread_context::switch_in() {
    _context.switch_in();
}

void thread_context::switch_out() {
    _context.switch_out();
}

void thread_context::yield() {
    later().get();
}

// thread

future<> thread::join() {
    _context->_joined = true;
    return _context->_done.get_future();
}
void thread::yield() {
    thread_impl::get()->yield();
}


namespace thread_impl {

thread_context* get() {
    return g_current_context->_thread;
}

void yield() {
    g_current_context->_thread->yield();
}

void switch_in(thread_context* to) {
    to->switch_in();
}

void switch_out(thread_context* from) {
    from->switch_out();
}

void init() {
    g_unthreaded_context._link = nullptr;
    g_unthreaded_context._thread = nullptr;
    g_current_context = &g_unthreaded_context;
}

}
future<> later() {
    promise<> p;
    auto f = p.get_future();
    engine_schedule(make_task([p = std::move(p)] () mutable {
        printf("wtf\n");
        ::sleep(3);
        p.set_value();
    }));
    return f;
}
} // xhb namespace