#ifndef XHBLIB_THREAD_IMPL_H_
#define XHBLIB_THREAD_IMPL_H_

#include <setjmp.h>
#include <ucontext.h>

#include <chrono>

#include "utility/optional.h"
#include "preempt.h"

namespace xhb {
    
using thread_clock = std::chrono::steady_clock;

class thread_context;

struct jmp_buf_link {
    jmp_buf _jmpbuf;
    jmp_buf_link* _link;
    thread_context* _thread;
    optional<std::chrono::time_point<thread_clock> _yield_at = {};

    void initial_switch_in(ucontext_t* initial_context, const void* stack_bottom, size_t stack_size);
    void switch_in();
    void switch_out();
    void initial_switch_in_completed();
    void final_switch_out();
};

extern thread_local jmp_buf_link* g_current_context;


namespace thread_impl {

extern thread_context* get();
bool should_yield();
void yield();
void switch_in(thread_context* to);
void switch_out(thread_context* from);
void init();

} // thread_impl namespace 

} // xhb namespace
#endif // XHBLIB_THREAD_IMPL_H_