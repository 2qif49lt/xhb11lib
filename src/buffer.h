#ifndef XHBLIB_BUFFER_H_
#define XHBLIB_BUFFER_H_

#include <exception>
#include <algorithm>
#include <cstdlib> // malloc/free

#include "cleaner.h"
#include "utility/memory_align.h"

namespace xhb {

// buffer 具有自动内存管理,共享,子缓存等特点的
// 禁用了一般复制拷贝,一般赋值
class buffer final{
    char* _buf = nullptr;
    size_t _size = 0;
    cleaner _cleaner;

public:
    // 空缓冲
    buffer() = default;
    ~buffer() = default;
    // 禁用一般复制
    buffer(const buffer&) = delete;

    // 创建一个自己申请空间的buffer
    buffer(size_t size) {
        _buf = static_cast<char*>(std::malloc(size));
        // malloc(0) 是 implementation defined. 有的返回null,有的返回有效指针.
        if (_buf == nullptr || size == 0) 
            throw std::bad_alloc();
        _size = size;
        _cleaner = make_cleaner(_buf);
    }
    // 使用指定空间大小cleaner
    buffer(const char* data, size_t size, cleaner c) : _buf(const_cast<char*>(data)), _size(size), _cleaner(std::move(c)){
    }

    // 转移缓冲
    buffer(buffer&& other) : _buf(other._buf), _size(other._size), _cleaner(std::move(other._cleaner)){
        other._buf = nullptr;
        other._size = 0;
    }

    // 拷贝指定的数据
    buffer(const char* data, size_t size) : buffer(size) {
        std::copy_n(data, size, _buf);
    }

    // 禁用一般赋值
    buffer& operator=(const buffer&) = delete;

    buffer& operator=(buffer&& other) {
        if (this != &other) {
            this->~buffer();
            new(this)buffer(std::move(other));
        }
        return *this;
    }

    // 分配一个按内存地址按align对齐,size大小的buffer
    static buffer aligned(size_t align, size_t size) {
        void* ptr = nullptr;
        int ret = posix_memalign(&ptr, align, size);
        if (ret != 0)
            throw std::bad_alloc();
        return buffer(static_cast<char*>(ptr), size, make_cleaner(ptr));
    }

    char* get() { return _buf; }
    const char* get() const { return _buf; }

    size_t size() const { return _size; }

    char* begin() { return _buf; }
    const char* begin() const {return _buf; }
    
    char* end() { return _buf + _size; }
    const char* end() const { return _buf + _size; }

    // 返回一个新的缓存，内部指向同块数据的前size部分，共享的buffer空间在所有的buffer都析构后时才会回收
    buffer front(size_t s) & {
        if (s  > size()) {
            throw std::out_of_range("size is oor ");
        }
        return buffer(_buf,_size,_cleaner.share());
    }

    // 临时buffer对象
    // 方便 buffer(1024).front(512).todo 或 func().front(int)类操作
    //  member function ref-qualifiers
    buffer front(size_t s) && {
        if (s  > size()) {
            throw std::out_of_range("size is oor ");
        }
        _size = s;
        return std::move(*this);
    }

    // 返回一个新的缓存，内部指向同块数据全部。
    buffer share() {
        return buffer(_buf,_size,_cleaner.share());
    }

    // share(0,size) 等价于 b.front(size)
    buffer share(size_t pos, size_t size) {
        buffer ret = share();
        ret._buf += pos;
        ret._size = size;
        return ret;
    }

    // 抹掉缓存中前size个字节
    void trim_front(size_t size) {
        _buf += size;
        _size -= size;
    }

    void trim(size_t size) {
        _size -= size;
    }
    
    char& operator[](size_t pos) {
        return _buf[pos];
    }

    
    const char& operator[](size_t pos) const {
        return _buf[pos];
    }

    // 是否为空
    bool empty() const {
        return size() == 0;
    }

    explicit operator bool() const {
        return size() != 0;
    }

    bool operator==(const buffer& other) const {
        return size() == other.size() && std::equal(begin(),end(),other.begin());
    }
    bool operator!=(const buffer& other) const {
        return !(*this == other);
    }

    // 分离cleaner.
    // 分离后,该对象析构后不会清理内存,调用者负责处理返回得到的cleaner.
    cleaner detach_cleaner() {
        return std::move(_cleaner);
    }



};


} // xhb namespace 

#endif // XHBLIB_BUFFER_H_
