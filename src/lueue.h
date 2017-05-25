#ifndef XHBLIB_LINK_QUEUE_H_
#define XHBLIB_LINK_QUEUE_H_

// lueue 区别std:deque在于:deque内部用数组进行索引节点并支持随机访问插入等操作，而lueue用链表进行管理节点，只支持队列首尾的操作。
// 主要的优点是可以配置节点元素大小、剔除了deque的部分功能保证所有操作都是O(1)、内存分配平滑。
// 相比rueue，lueue优点体现在分配内存时波动平滑,内存不会只增不减

#include <memory>
#include <algorithm>
#include <stdexcept>
#include <cmath> // ceil
#include <cstdlib> // malloc

namespace xhb {

template<typename T,
    size_t BLOCK_ITEM_SIZE = 128,
    class Alloc = std::allocator<T>>
class lueue final :public Alloc{
private:
    struct block{
        T items[BLOCK_ITEM_SIZE]; // 尽量加大cpu cache line的命中概率
        struct block* next = nullptr;
        unsigned int beg = 0;
        unsigned int end = 0;
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

private:
    using alloc_traits = std::allocator_traits<allocator_type>;
    
public:
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
    lueue& operator=(my_type&& other)noexcept{
        if(&other != this){
            this->~lueue();
            new(this)lueue(std::move(other));
        }
        return *this;
    }

    inline void push_back(const T& val){
        may_allocate();
        alloc_traits::construct(*this,&_tail->items[_tail->end],val);
        ++_tail->end;
    }
    inline void push_back(T&& val){
        may_allocate();
        alloc_traits::construct(*this,&_tail->items[_tail->end],std::move(val));
        ++_tail->end;
    }
    template<typename... A>
    inline void emplace_back(A&&... args){
        may_allocate();
        alloc_traits::construct(*this,&_tail->items[_tail->end],std::forward<A>(args)...);
        ++_tail->end;
    }
    
    inline void pop_front() noexcept{
        alloc_traits::destroy(*this,&front());
        if(++_head->beg == _head->end)
            delete_front_block();
    }

    reference back()noexcept{
        if(empty()) throw(std::runtime_error("lueue is empty"));
        return _tail->items[_tail->end - 1];
    }
    const_reference back()const noexcept{
        if(empty()) throw(std::runtime_error("lueue is empty"));
        return _tail->items[_tail->end - 1];
    }
    
    reference front()noexcept{
        if(empty()) throw(std::runtime_error("lueue is empty"));
        return _head->items[_head->beg];
    }
    const_reference front()const noexcept{
        if(empty()) throw(std::runtime_error("lueue is empty"));
        return _head->items[_head->beg];
    }

    inline bool empty()const noexcept{
        return _head == nullptr;
    }

    inline size_type size()const noexcept{
        if(empty()) return 0;
        if(_head == _tail) return _head->end - _head->beg;
        return _head->end - _head->beg 
                + _tail->end - _tail->beg
                + (_nblock - 2) * BLOCK_ITEM_SIZE;
    }

    // 预先申请可以容纳n个元素，常用初始化队列
    // n为总共的元素个数
    void reserve(size_t n){
        if(n <= (_nblock + _nfblock) * BLOCK_ITEM_SIZE)
            return;
       
        size_t total_block =  (size_t)std::ceil((double)n/BLOCK_ITEM_SIZE);
        size_t need_block = total_block - _nblock;
       
        if(need_block <= _nfblock) return;
       
        need_block -= _nfblock;

        while(need_block--){
            block* p = static_cast<block*>(std::malloc(sizeof(block)));
            p->beg = 0;
            p->end = 0;

            p->next = _free;
            _free = p;
            
            ++_nfblock;
        }
    }

    size_t capacity()const{
        return (_nfblock + _nblock) * BLOCK_ITEM_SIZE;
    }
    void clear(){
        while(!empty()){
            pop_front();
        }
    }

private:
    void new_back_block(){
        block* old = _tail;
        if(_free){
            _tail = _free;
            _free = _free->next;
            --_nfblock;
        }else{
            _tail = static_cast<block*>(std::malloc(sizeof(block)));
        }
        _tail->next = nullptr;
        _tail->beg = 0;
        _tail->end = 0;

        if(old != nullptr)
            old->next = _tail;
        if(_head == nullptr)
            _head = _tail;
        ++_nblock;
    }
    
    void delete_front_block(){
         static const int FREE_BLOCK_NUM = 1;

        block* next = _head->next;

        if(_nfblock < FREE_BLOCK_NUM){
            _head->next = _free;
            _free = _head;
            ++_nfblock;
        }else{
            std::free(_head);
        }

        if(_head == _tail) _tail = nullptr;

        _head = next;
        --_nblock;
    }
    void may_allocate(){
        if(_tail == nullptr || (_tail->end - _tail->beg) >= BLOCK_ITEM_SIZE)
            new_back_block();
    }
};

} // xhb namespace
#endif // XHBLIB_LINK_QUEUE_H_
