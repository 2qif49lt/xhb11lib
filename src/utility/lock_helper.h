#ifndef XHBLIB_UTILITY_LOCK_HELPER_H_
#define XHBLIB_UTILITY_LOCK_HELPER_H_

#include <mutex>

#define WITH_LOCK(m)  for (std::lock_guard<decltype(m)> _with_lock_a_##m(m), *_with_lock_b_##m = 0; _with_lock_b_##m == 0; _with_lock_b_##m++)
#endif // XHBLIB_UTILITY_LOCK_HELPER_H_