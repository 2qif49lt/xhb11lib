#ifndef XHBLIB_PREEMPT_H_
#define XHBLIB_PREEMPT_H_

#pragma once
#include <atomic>

extern thread_local bool g_need_preempt;

inline bool need_preempt() {
#ifndef DEBUG
    // prevent compiler from eliminating loads in a loop
    // 特别是在有使用setjmp的环境下
    std::atomic_signal_fence(std::memory_order_seq_cst);
    return g_need_preempt;
#else
    return true;
#endif
}

#endif // XHBLIB_PREEMPT_H_