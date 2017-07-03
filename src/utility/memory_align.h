#ifndef XHBLIB_UTILITY_MEMORY_ALIGN_H_
#define XHBLIB_UTILITY_MEMORY_ALIGN_H_

// 该头文件为实现posix_memalign的跨平台使用

#ifdef _WIN32
extern int posix_memalign(void **ptr, size_t align, size_t size);
#else
#include <stdlib.h> //  posix_memalign
#endif

#include <cassert>
#include <memory>
#include <new> // bad_alloc
#include <stdexcept>
#include <string>
namespace xhb {


struct free_deleter {
    void operator()(void* p) { ::free(p); }
};

template <typename CharType>
inline
std::unique_ptr<CharType[], free_deleter> allocate_aligned_buffer(size_t size, size_t align) {
    static_assert(sizeof(CharType) == 1, "must allocate byte type");
    void* ret;
    auto r = posix_memalign(&ret, align, size);
    if (r == ENOMEM) {
        throw std::bad_alloc();
    } else if (r == EINVAL) {
        char buff[256] = {};
        sprintf(buff,"Invalid alignment of %d; allocating %d bytes", align, size);
        throw std::runtime_error(std::string(buff));
    } else {
        assert(r == 0);
        return std::unique_ptr<CharType[], free_deleter>(reinterpret_cast<CharType *>(ret));
    }
}


} // xhb namespace

#endif // XHBLIB_UTILITY_MEMORY_ALIGN_H_