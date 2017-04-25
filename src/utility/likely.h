#ifndef XHBLIB_UTILITY_LIKELY_H_
#define XHBLIB_UTILITY_LIKELY_H_


#if defined(__GNUC__) || defined(__GNUG__) || defined(__clang__)

#ifndef likely
#define likely(x)       ::__builtin_expect(!!(x), 1)
#endif 

#ifndef unlikely
#define unlikely(x)     ::__builtin_expect(!!(x), 0)
#endif

#else
// msvc
#define likely(x) (x)
#define unlikely(x) (x)
#define __builtin_expect(exp, a)  (exp)

#endif


#endif // XHBLIB_UTILITY_LIKELY_H_