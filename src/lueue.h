#ifndef XHBLIB_LINK_QUEUE_H_
#define XHBLIB_LINK_QUEUE_H_

// lueue 区别std:deque在于:deque内部用数组进行索引节点并支持随机访问插入等操作，而lueue用链表进行管理节点，只支持队列首尾的操作。
// 主要的优点是可以配置节点元素大小、剔除了deque的部分功能保证所有操作都是O(1)、内存分配平滑。
// 相比rueue，lueue优点体现在分配内存时波动平滑。

#include <memory>
#include <algorithm>
#include <stdexcept>

template<typename T,
    size_t BLOCK_ITEM_SIZE = 128,
    class Alloc = std::allocator<T>>
class lueue final {
private:

    struct block{
        T[BLOCK_ITEM_SIZE];
        struct block* next = nullptr;
        unsigned int beg = 0;
        unsgined int end = 0;
    };

    block* _head = nullptr;
    block* _tail = nullptr;

    // block的数目，当调用size()时可以在o(1)里计算首尾block个数和block倍数个元素
    size_t _nblock = 0;

    // 保存已经释放了的block，目的是为了提升性能。尤其是当队列处理快速被消费时，避免传统频繁申请释放内存。
    block* _free = nullptr;
    size_t _nfblock = 0;
public:

    using my_type = lueue<T,BLOCK_ITEM_SIZE,Alloc>;
    using allocator_type = Alloc;
    
    using value_type = T;
    using size_type = size_t;
    using reference = T&;
    using pointer = T*;
    using const_reference = const T&;
    using const_point = const T*;

    lueue() = default;
    lueue(my_type&& other)noexcept : _head(other._head)
            ,_tail(other._tail)
            ,_nblock(other._nblock)
            ,_free(other._free)
            ,_nfblock(other._nfblock){
        other._head = nullptr;
        other._tail = nullptr;
        _nblock = 0;
        _free = nullptr;
        _nfblock = 0;
    }
    lueue(const my_type&) = delete;
    ~lueue(){
        clear();
    }

    lueue& operator=(const my_type&) = delete;
    lueue& operator=(my_type&& other)noexcept;

    inline void push_back(const T& val);
    inline void push_back(T&& val);
    template<typename... A>
    inline void emplace_back(A&&... args);
    
    inline void pop_front() noexcept; 

    reference back()noexcept;
    const_reference back()const noexcept;
    
    reference front()noexcept;
    const_reference front()const noexcept;

    inline bool empty()const noexcept;

    inline size_type size()const noexcept;

    // 扩容
    void reserve(size_t n);

    void clear(){
        while()
    }

private:
    void new_back_block();
    void delete_front_block();
    inline size_t mask(size_t idx)const noexcept;
};
#endif // XHBLIB_LINK_QUEUE_H_