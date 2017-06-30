#ifndef XHBLIB_THREAD_IMPL_H_
#define XHBLIB_THREAD_IMPL_H_

#include <setjmp.h>
#include <ucontext.h>



namespace xhb {
    
class thread_context;

struct jmp_buf_link {
    ucontext_t _context;
    jmp_buf_link* _link;
    thread_context* _thread;

    void initial_switch_in(ucontext_t* initial_context);
    void switch_in();
    void switch_out();
    void initial_switch_in_completed();
    void final_switch_out();
};


namespace thread_impl {

extern thread_context* get();
void yield();
void switch_in(thread_context* to);
void switch_out(thread_context* from);
void init();

} // thread_impl namespace 

} // xhb namespace
#endif // XHBLIB_THREAD_IMPL_H_