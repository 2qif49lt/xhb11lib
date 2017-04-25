#ifndef XHBLIB_UTILITY_MEMORY_ALIGN_H_
#define XHBLIB_UTILITY_MEMORY_ALIGN_H_

// 该头文件为实现posix_memalign的跨平台使用

#ifdef _WIN32
extern int posix_memalign(void **ptr, size_t align, size_t size);
#else
#include <stdlib.h> //  posix_memalign
#endif

#endif // XHBLIB_UTILITY_MEMORY_ALIGN_H_