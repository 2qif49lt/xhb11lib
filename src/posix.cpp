#include <sys/mman.h>

#include "posix.h"
#include "utility/align.h"

namespace xhb {

void mmap_deleter::operator()(void* ptr) const {
    ::munmap(ptr, _size);
}

mmap_area mmap_anonymous(void* addr, size_t length, int prot, int flags) {
    auto ret = ::mmap(addr, length, prot, flags | MAP_ANONYMOUS, -1, 0);
    throw_system_error_on(ret == MAP_FAILED);
    return mmap_area(reinterpret_cast<char*>(ret), mmap_deleter{length});
}


void* posix_thread::start_routine(void* arg) noexcept {
    auto pfunc = reinterpret_cast<std::function<void ()>*>(arg);
    (*pfunc)();
    return nullptr;
}

posix_thread::posix_thread(std::function<void ()> func)
    : posix_thread(attr{}, std::move(func)) {
}

posix_thread::posix_thread(posix_thread&& x)
    : _func(std::move(x._func)), _pthread(x._pthread), _valid(x._valid)
    , _stack(std::move(x._stack)) {
    x._valid = false;
}

posix_thread::~posix_thread() {
    assert(!_valid);
}

posix_thread::posix_thread(attr a, std::function<void ()> func)
    : _func(std::make_unique<std::function<void ()>>(std::move(func))) {
    pthread_attr_t pa;
    auto r = pthread_attr_init(&pa);
    if (r) {
        throw std::system_error(r, std::system_category());
    }
    auto stack_size = a._stack_size.size;
    if (!stack_size) {
        stack_size = 2 << 20;
    }
    // allocate guard area as well
    _stack = mmap_anonymous(nullptr, stack_size + (4 << 20),
            PROT_NONE, MAP_PRIVATE | MAP_NORESERVE);
    // 栈基
    auto stack_start = align_up<2 << 20>(_stack.get() + 1);
    mmap_area real_stack = mmap_anonymous(stack_start, stack_size,
            PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_FIXED | MAP_STACK);// MAP_STACK:give out an address that is best suited for process/thread stacks
    real_stack.release(); // protected by @_stack


    ::madvise(stack_start, stack_size, MADV_HUGEPAGE);
    r = pthread_attr_setstack(&pa, stack_start, stack_size);
    if (r) {
        throw std::system_error(r, std::system_category());
    }
    r = pthread_create(&_pthread, &pa,
                &posix_thread::start_routine, _func.get());
    if (r) {
        throw std::system_error(r, std::system_category());
    }
}

void posix_thread::join() {
    assert(_valid);
    pthread_join(_pthread, NULL);
    _valid = false;
}


} // xhb namespace