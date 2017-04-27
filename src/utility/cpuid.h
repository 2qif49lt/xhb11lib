#ifndef XHBLIB_UTILITY_CPUID_H_
#define XHBLIB_UTILITY_CPUID_H_

#if defined(__linux__)

#include <sched.h>

#define xhb_cpuid() sched_getcpu()

#elif defined(__APPLE__)

int xhb_cpuid(); 

#elif defined(_WIN32)
// msvc
int xhb_cpuid();
#else
// others 
#error "not support"
#endif // others

#endif // XHBLIB_UTILITY_CPUID_H_
