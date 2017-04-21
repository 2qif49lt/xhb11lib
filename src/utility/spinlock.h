#ifndef XHBLIB_SPIN_LOCK_H_
#define XHBLIB_SPIN_LOCK_H_

#include <atomic>
#include <cassert>

#ifdef _MSC_VER
#include <Windows.h>
#else
#include <xmmintrin.h>
#define YieldProcessor()  _mm_pause()
#endif

// memory order
// http://preshing.com/20120913/acquire-and-release-semantics/
// The role of a release fence, as defined by the C++11 standard, 
// is to prevent previous memory operations from moving past subsequent stores.

class spinlock {
    std::atomic<bool> _busy = { false };
public:
    spinlock() = default;
    spinlock(const spinlock&) = delete;
    ~spinlock() { assert(!_busy.load(std::memory_order_relaxed)); }
    void lock() noexcept {
        while (_busy.exchange(true, std::memory_order_acquire)) {
            YieldProcessor();
        }
    }
    void unlock() noexcept {
        _busy.store(false, std::memory_order_release);
    }
};


#endif // XHBLIB_SPIN_LOCK_H_
