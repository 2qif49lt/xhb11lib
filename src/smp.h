#ifndef XHBLIB_SMP_H_
#define XHBLIB_SMP_H_

// #include <boost/lockfree/spsc_queue.hpp>
// #include <boost/thread/barrier.hpp>
// #include <boost/range/irange.hpp>

#include <cassert>

#include <type_traits>
#include <thread>
#include <mutex>
#include <vector>
#include <sstream>
#include <memory>
#include <functional>

#include "utility/memory_align.h"
#include "utility/spinlock.h"
#include "utility/print_safe.h"
#include "resource.h"
#include "posix.h"


namespace xhb {

class reactor;

class smp {
    static std::vector<posix_thread> _threads;
    static std::vector<reactor*> _reactors;
    static std::thread::id _tmain;

public:
    static bool main_thread() { return std::this_thread::get_id() == _tmain; }

    static int run_one(std::function<void ()>&& func);
    static void configure(resource_config& rc);
    static void cleanup();
    static void join_all();

private:
    static void pin(unsigned cpu_id);
    static void allocate_reactor(unsigned id);
    static void create_thread(std::function<void ()> thread_loop);
public:
    static unsigned count;
};

} // xhb namespace

#endif // XHBLIB_SMP_H_