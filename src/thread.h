#ifndef XHBLIB_THREAD_H_
#define XHBLIB_THREAD_H_

#include <memory>
#include <chrono>
#include <type_traits>
#include <functional>
#include <cassert>
#include "future.h"
#include "reactor.h"
#include "thread_impl.h"

namespace xhb {

class thread;

// 线程上下文
class thread_context {
    struct stack_deleter {
        void operator()(char* ptr) const {
            delete [] ptr;
        }
    };
    using stack_holder = std::unique_ptr<char[], stack_deleter>;

    static constexpr size_t _stack_size = 128 * 1024;
    stack_holder _stack{make_stack()};
    std::function<void()> _func;
    jmp_buf_link _context;
    promise<> _done;
    bool _joined = false;

private:
    static stack_holder make_stack();
    static void s_main(unsigned int lo, unsigned int hi);
    void setup();
    void main();
public:
    thread_context(std::function<void ()> func);
    ~thread_context();
    void switch_in();
    void switch_out();
    void yield();
    friend class thread;
    friend void thread_impl::switch_in(thread_context*);
    friend void thread_impl::switch_out(thread_context*);
};


class thread {
    std::unique_ptr<thread_context> _context;
public:
    thread() = default;
    
    template <typename F>
    thread(F func): _context(std::make_unique<thread_context>(func)) {
    }

    thread(thread&& other) noexcept = default;
    thread& operator=(thread&& other) noexcept = default;
    
    ~thread() { assert(!_context || _context->_joined); }

    future<> join();

    static void yield();

    static bool running_in_thread() {
        return thread_impl::get() != nullptr;
    }
private:
    friend class reactor;
    static bool try_run_one_yielded_thread();
};

/// Executes a callable in a seastar thread.
///
/// Runs a block of code in a threaded context,
/// which allows it to block (using \ref future::get()).  The
/// result of the callable is returned as a future.
///
/// \param attr a \ref thread_attributes instance
/// \param func a callable to be executed in a thread
/// \param args a parameter pack to be forwarded to \c func.
/// \return whatever \c func returns, as a future.
///
/// Example:
/// \code
///    future<int> compute_sum(int a, int b) {
///        thread_attributes attr = {};
///        attr.scheduling_group = some_scheduling_group_ptr;
///        return seastar::async(attr, [a, b] {
///            // some blocking code:
///            sleep(1s).get();
///            return a + b;
///        });
///    }
/// \endcode

template <typename Func, typename... Args>
inline
futurize_t<std::result_of_t<std::decay_t<Func>(std::decay_t<Args>...)>>
async(Func&& func, Args&&... args) {
    using return_type = std::result_of_t<std::decay_t<Func>(std::decay_t<Args>...)>;
    struct work {
        Func func;
        std::tuple<Args...> args;
        promise<return_type> pr;
        thread th;
    };
    return do_with(work{std::forward<Func>(func), std::forward_as_tuple(std::forward<Args>(args)...)}, [] (work& w) mutable {
        auto ret = w.pr.get_future();
        w.th = thread([&w] {
            futurize<return_type>::apply(std::move(w.func), std::move(w.args)).forward_to(std::move(w.pr));
        });
        return w.th.join().then([ret = std::move(ret)] () mutable {
            return std::move(ret);
        });
    });
}

future<> later();
} // xhb namespace
#endif // XHBLIB_THREAD_H_