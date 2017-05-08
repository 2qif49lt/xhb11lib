#ifndef XHBLIB_UTILITY_BIT_OPT_H_
#define XHBLIB_UTILITY_BIT_OPT_H_

// 常用位操作

/*
    assert(xhb::ctz((unsigned int)12) == 2);
	assert(xhb::clz((unsigned int)12) == 28);
	assert(xhb::ctz((unsigned short)12) == 2);
	assert(xhb::clz((unsigned short)12) == 12);
	assert(xhb::ctz((unsigned long)12) == 2);
	assert(xhb::clz((unsigned long)12) == 60);
	assert(xhb::ctz((unsigned long long)12) == 2);
	assert(xhb::clz((unsigned long long)12) == 60);
*/
namespace xhb {

#include <limits>

#ifdef _MSC_VER
#include <intrin.h>

#else
    #ifdef __clang__
        #if __clang_major__ < 5
        #error "support clang 5.0+"
        #endif
    #else
        // gcc
    #endif

#endif

// 计算前导0的个数
inline unsigned int clz(unsigned int val) {
    constexpr unsigned int bits =  std::numeric_limits<unsigned int>::digits;

    #ifdef _MSC_VER
    unsigned long leading_zeros = 0;
    return _BitScanReverse(&leading_zero, val) ? (unsigned int)(bits -1 - leading_zeros) : bits;
    #else
    return val ? (unsigned int)__builtin_clz(val) : bits;
    #endif
}
inline unsigned int clz(unsigned short val) {
    constexpr unsigned int bits =  std::numeric_limits<unsigned short>::digits;
    constexpr unsigned int cast_zeros = std::numeric_limits<unsigned int>::digits - bits;

    #ifdef _MSC_VER
    unsigned long leading_zeros = 0;
    return _BitScanReverse(&leading_zero, val) ? (unsigned int)(bits -1 - leading_zeros) : bits;
    #else
    return val ? (unsigned int)__builtin_clz((unsigned int)val) - cast_zeros : bits;
    #endif
}
inline unsigned int clz(unsigned long val) {
    constexpr unsigned int bits =  std::numeric_limits<unsigned long>::digits;

    #ifdef _MSC_VER
    unsigned long leading_zeros = 0;
    return _BitScanReverse(&leading_zero, val) ? (unsigned int)(bits -1 - leading_zeros) : bits;
    #else
    return val ? (unsigned int)__builtin_clzl(val) : bits;
    #endif
}
inline unsigned int clz(unsigned long long val) {
    constexpr unsigned int bits =  std::numeric_limits<unsigned long long>::digits;

    #ifdef _MSC_VER
    unsigned long leading_zeros = 0;
    return _BitScanReverse64(&leading_zero, val) ? (unsigned int)(bits -1 - leading_zeros) : bits;
    #else
    return val ? (unsigned int)__builtin_clzll(val) : bits;
    #endif
}


// 计算后缀0个数。
inline unsigned int ctz(unsigned int val) {
    constexpr unsigned int bits =  std::numeric_limits<unsigned int>::digits;

    #ifdef _MSC_VER
    unsigned long trailing_zeros = 0;
    return _BitScanForward(&trailing_zeros, val) ? (unsigned int)(trailing_zeros) : bits;
    #else
    return val ? (unsigned int)__builtin_ctz(val) : bits;
    #endif
}
inline unsigned int ctz(unsigned short val) {
    constexpr unsigned int bits =  std::numeric_limits<unsigned short>::digits;

    #ifdef _MSC_VER
    unsigned long trailing_zeros = 0;
    return _BitScanForward(&trailing_zeros, val) ? (unsigned int)(trailing_zeros) : bits;
    #else
    return val ? (unsigned int)__builtin_ctz((unsigned int)val) : bits;
    #endif  
}
inline unsigned int ctz(unsigned long val) {
    constexpr unsigned int bits =  std::numeric_limits<unsigned long>::digits;

    #ifdef _MSC_VER
    unsigned long trailing_zeros = 0;
    return _BitScanForward(&trailing_zeros, val) ? (unsigned int)(trailing_zeros) : bits;
    #else
    return val ? (unsigned int)__builtin_ctzl(val) : bits;
    #endif
}
inline unsigned int ctz(unsigned long long val) {
    constexpr unsigned int bits =  std::numeric_limits<unsigned int>::digits;

    #ifdef _MSC_VER
    unsigned long trailing_zeros = 0;
    return _BitScanForward64(&trailing_zeros, val) ? (unsigned int)(trailing_zeros) : bits;
    #else
    return val ? (unsigned int)__builtin_ctzll(val) : bits;
    #endif
}

} // xhb namepsace


#endif // XHBLIB_UTILITY_BIT_OPT_H_