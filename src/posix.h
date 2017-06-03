#ifndef XHBLIB_POSIX_H_
#define XHBLIB_POSIX_H_

#include <pthread.h>
#include <sched.h>

#include <assert.h>

#include <utility>
#include <memory>
#include <functional>
#include <system_error>


namespace xhb {

inline void throw_system_error_on(bool condition, const char* what_arg = "");

template <typename T>
inline void throw_kernel_error(T r);

struct mmap_deleter {
    size_t _size;
    void operator()(void* ptr) const;
};

using mmap_area = std::unique_ptr<char[], mmap_deleter>;

mmap_area mmap_anonymous(void* addr, size_t length, int prot, int flags);

class posix_thread {
public:
    class attr;
private:
    // must allocate, since this class is moveable
    std::unique_ptr<std::function<void ()>> _func;
    pthread_t _pthread;
    bool _valid = true;
    mmap_area _stack;
private:
    static void* start_routine(void* arg) noexcept;
public:
    posix_thread(std::function<void ()> func);
    posix_thread(attr a, std::function<void ()> func);
    posix_thread(posix_thread&& x);
    ~posix_thread();
    void join();
public:
    class attr {
    public:
        struct stack_size { size_t size = 0; };
        attr() = default;
        template <typename... A>
        attr(A... a) {
            set(std::forward<A>(a)...);
        }
        void set() {}
        template <typename A, typename... Rest>
        void set(A a, Rest... rest) {
            set(std::forward<A>(a));
            set(std::forward<Rest>(rest)...);
        }
        void set(stack_size ss) { _stack_size = ss; }
    private:
        stack_size _stack_size;
        friend class posix_thread;
    };
};

inline
void throw_system_error_on(bool condition, const char* what_arg) {
    if (condition) {
        throw std::system_error(errno, std::system_category(), what_arg);
    }
}

template <typename T>
inline
void throw_kernel_error(T r) {
    static_assert(std::is_signed<T>::value, "kernel error variables must be signed");
    if (r < 0) {
        throw std::system_error(-r, std::system_category());
    }
}

template <typename T>
inline
void throw_pthread_error(T r) {
    if (r != 0) {
        throw std::system_error(r, std::system_category());
    }
}

inline
void pin_this_thread(unsigned cpu_id) {
    cpu_set_t cs;
    CPU_ZERO(&cs);
    CPU_SET(cpu_id, &cs);
    auto r = pthread_setaffinity_np(pthread_self(), sizeof(cs), &cs);
    assert(r == 0);
}

} // xhb namespace

#endif // XHBLIB_POSIX_H_