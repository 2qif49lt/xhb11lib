#if defined(__linux__)


cpu_set_t cupset(unsigned int cpuid) {
    cup_set_t cpu;
    CPU_ZERO(&cpu);
    CPU_SET(cpuid,&cpu);
    return cpu;
}

#ifdef XHBLIB_USE_HWLOC
#include <hwloc.h>


#endif // XHBLIB_USE_HWLOC

#endif // linux