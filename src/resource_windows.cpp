#include "resource.h"

#ifdef _WIN32

#if (WINVER < _WIN32_WINNT_WIN8)
#include <Windows.h>
#else
#include <Windows.h>
#include <Processthreadsapi.h>
#endif

int get_current_cpuid() {
    return (int)GetCurrentProcessorNumber();
}

#endif // win32