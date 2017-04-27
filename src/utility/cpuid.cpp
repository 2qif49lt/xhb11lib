#include "cpuid.h"

#ifdef __APPLE__
#include <cpuid.h>

#define _xhb_CPUID(INFO, LEAF, SUBLEAF) __cpuid_count(LEAF, SUBLEAF, INFO[0], INFO[1], INFO[2], INFO[3])

int xhb_cpuid() {
    int id = 0;

    unsigned int CPUInfo[4];
    _xhb_CPUID(CPUInfo, 1, 0);
    /* CPUInfo[1] is EBX, bits 24-31 are APIC ID */ 
    if ( (CPUInfo[3] & (1 << 9)) == 0) { 
        id =  -1;  /* no APIC on chip */ 
    } 
    else { 
        id =  (unsigned)CPUInfo[1] >> 24; 
    } 
    if (id < 0) id = 0;
    return id;
}
#endif


#ifdef _WIN32

#if (WINVER < _WIN32_WINNT_WIN8)
#include <Windows.h>
#else
#include <Processthreadsapi.h>
#endif

int xhb_cpuid() {
    return (int)GetCurrentProcessorNumber();
}

#endif
