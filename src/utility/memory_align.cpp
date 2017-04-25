#include "memory_align.h"

#ifdef _WIN32
#include <errno.h>  
#include <windows.h>


int posix_memalign(void **ptr, size_t align, size_t size)
{
    if (align == 0 || (align & (align - 1) != 0)))
        return EINVAL;
 
    int saved_errno = errno;
    void *p = _aligned_malloc(size, align);
    if (p == NULL)
    {
        errno = saved_errno;
        return ENOMEM;
    }
 
    *ptr = p;
    return 0;
}


#endif // _WIN32