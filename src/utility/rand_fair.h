#ifndef XHB_LIB_RAND_FAIR_H_
#define XHB_LIB_RAND_FAIR_H_

#include <cstdlib>
#include <iostream>
namespace xhb {

// https://stackoverflow.com/questions/10984974/why-do-people-say-there-is-modulo-bias-when-using-a-random-number-generator

// rand_fair_test.cpp
// 大部分2次循环，和std::rand相差一倍性能左右，遍历INT32_MAX次耗时分别为:27.5s和16.1s
inline int rand(int max) {
    int ret = 0;
    do {
        ret = std::rand();
    } while (ret >= (RAND_MAX - RAND_MAX % max));
    
    ret %= max;

    return  ret;
}

} // xhb namespace
#endif // XHB_LIB_RAND_FAIR_H_