#ifndef XHBLIB_SMP_H_
#define XHBLIB_SMP_H_

#include <boost/lockfree/spsc_queue.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/range/irange.hpp>

#include <cassert>

#include <type_traits>
#include <thread>
#include <mutex>
#include <vector>
#include <sstream>
#include <memory>
#include <functional>

#include "utility/optional.h"
#include "utility/memory_align.h"
#include "utility/spinlock.h"
#include "utility/print_safe.h"
#include "utility/resource.h"
#include "posix.h"
#include "future.h"
#include "future_util.h"


namespace xhb {

class smp {
    static std::vector<posix_thread> _threads;
    static optional<boost::barrier> _all_event_loops_done;
    static std::vector<reactor*> _reactors;
    static smp_message_queue** _qs;
    static std::thread::id _tmain;

    template <typename Func>
    using returns_future = is_future<std::result_of_t<Func()>>;
    template <typename Func>
    using returns_void = std::is_same<std::result_of_t<Func()>, void>;

public:
    static bool main_thread() { return std::this_thread::get_id() == _tmain; }
    static void arrive_at_event_loop_end();

    static void configure(resource_config& rc);
    static void cleanup();
    static void cleanup_cpu();
    static void join_all();

    /// Runs a function on a remote core.
    ///
    /// \param t designates the core to run the function on (may be a remote
    ///          core or the local core).
    /// \param func a callable to run on core \c t.  If \c func is a temporary object,
    ///          its lifetime will be extended by moving it.  If @func is a reference,
    ///          the caller must guarantee that it will survive the call.
    /// \return whatever \c func returns, as a future<> (if \c func does not return a future,
    ///         submit_to() will wrap it in a future<>).
    template <typename Func>
    static futurize_t<std::result_of_t<Func()>> submit_to(unsigned t, Func&& func) {
        using ret_type = std::result_of_t<Func()>;
        if (t == engine().cpu_id()) {
            try {
                if (!is_future<ret_type>::value) {
                    // Non-deferring function, so don't worry about func lifetime
                    return futurize<ret_type>::apply(std::forward<Func>(func));
                } else if (std::is_lvalue_reference<Func>::value) {
                    // func is an lvalue, so caller worries about its lifetime
                    return futurize<ret_type>::apply(func);
                } else {
                    // Deferring call on rvalue function, make sure to preserve it across call
                    auto w = std::make_unique<std::decay_t<Func>>(std::move(func));
                    auto ret = futurize<ret_type>::apply(*w);
                    return ret.finally([w = std::move(w)] {});
                }
            } catch (...) {
                // Consistently return a failed future rather than throwing, to simplify callers
                return futurize<std::result_of_t<Func()>>::make_exception_future(std::current_exception());
            }
        } else {
            return _qs[t][engine().cpu_id()].submit(std::forward<Func>(func));
        }
    }
    static bool poll_queues();
    static bool pure_poll_queues();
    static boost::integer_range<unsigned> all_cpus() {
        return boost::irange(0u, count);
    }
    // Invokes func on all shards.
    // The returned future resolves when all async invocations finish.
    // The func may return void or future<>.
    // Each async invocation will work with a separate copy of func.
    template<typename Func>
    static future<> invoke_on_all(Func&& func) {
        static_assert(std::is_same<future<>, typename futurize<std::result_of_t<Func()>>::type>::value, "bad Func signature");
        return parallel_for_each(all_cpus(), [&func] (unsigned id) {
            return smp::submit_to(id, Func(func));
        });
    }
private:
    static void start_all_queues();
    static void pin(unsigned cpu_id);
    static void allocate_reactor(unsigned id);
    static void create_thread(std::function<void ()> thread_loop);
public:
    static unsigned count;
};

} // xhb namespace

#endif // XHBLIB_SMP_H_