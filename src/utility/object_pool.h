#ifndef XHBLIB_OBJECT_POOL_H_
#define XHBLIB_OBJECT_POOL_H_
/*
lock-free、多生产消费者、固定长度、环形数组、对象池
对象池在未满的时候不会析构对象。
*/
#include <memory>
#include <atomic>
#include <functional>

#ifdef XHBLIB_OBJ_POOL_UNIT_TEST
#include <vector>
#endif

#include "likely.h" // for likely(x)
#include "spinlock.h" // for _mm_pause

namespace xhb {

namespace obj_pool_impl {


} // internal ns

template <typename T, uint32_t POOL_SIZE = 128, typename Alloc = std::allocator<T>>
class obj_pool final :public Alloc {
    static_assert((POOL_SIZE & (POOL_SIZE - 1)) == 0 && POOL_SIZE <= (2 << 12), "pool_size must be a power of 2.");
    static constexpr uint32_t MASK = POOL_SIZE - 1;
    static constexpr uint32_t cache_line_size = 64;

private:
    struct item {
        T* _data = nullptr;
        char _pad[cache_line_size - sizeof(T*)];
    };
    item _objs[POOL_SIZE];

    std::atomic<uint32_t> _prod_head{0};
    std::atomic<uint32_t> _prod_tail{0};

    std::atomic<uint32_t> _cons_head{0};
    std::atomic<uint32_t> _cons_tail{0};

    using my_type = obj_pool<T,POOL_SIZE,Alloc>;
    using alloc_traits = std::allocator_traits<Alloc>;

public:
#ifdef XHBLIB_OBJ_POOL_UNIT_TEST
    std::atomic<uint32_t> _allocate_counter{0};
    std::atomic<uint32_t> _deallocate_counter{0};
    std::atomic<uint64_t> _allocate_all{0};
    std::atomic<uint64_t> _deallocate_all{0}; 
#endif 

public:

    obj_pool() = default;
    ~obj_pool() {
        // 析构函数应该在其他线程未调用接口后单独进行。
        auto cons_head = _cons_head.load(std::memory_order_relaxed);
        auto prod_tail = _prod_tail.load(std::memory_order_relaxed);

        for (; cons_head != prod_tail; cons_head++) {
            auto idx = cons_head & MASK;
            alloc_traits::destroy(*this, _objs[idx]._data);
            alloc_traits::deallocate(*this, _objs[idx]._data, 1);
        }
    }

    obj_pool(const my_type&) = delete;
    obj_pool(my_type&&) = delete;
    
    uint32_t free_items() const {
        uint32_t prod_head = 0, cons_tail = 0;

        cons_tail = _cons_tail.load(std::memory_order_acquire);
        prod_head = _prod_head.load(std::memory_order_relaxed); 
        
        return MASK + cons_tail - prod_head;
    }

    uint32_t size() const {
        uint32_t prod_tail = 0, cons_head = 0;

        prod_tail = _prod_tail.load(std::memory_order_acquire); 
        cons_head = _cons_head.load(std::memory_order_relaxed);
        
        return  prod_tail - cons_head;
    }

    void put_raw(T* ptr) {
        uint32_t prod_head, prod_next;
        
        do {
            prod_head = _prod_head.load(std::memory_order_acquire);
            uint32_t cons_tail = _cons_tail.load(std::memory_order_relaxed);
            uint32_t free_entries = MASK + cons_tail - prod_head;
            
            if(unlikely(free_entries == 0)) {
#ifdef XHBLIB_OBJ_POOL_UNIT_TEST
                _deallocate_counter.fetch_add(1, std::memory_order_relaxed);
                _deallocate_all.fetch_add(ptr->_i, std::memory_order_relaxed);
#endif
                alloc_traits::destroy(*this, ptr);
                alloc_traits::deallocate(*this, ptr, 1); 
                return;
            }

            prod_next = prod_head + 1;
            
        } while (false == _prod_head.compare_exchange_weak(prod_head, prod_next,
            std::memory_order_release, std::memory_order_relaxed));
        
        uint32_t idx = prod_head & MASK;
        _objs[idx]._data = ptr;

        while (unlikely(_prod_tail.load(std::memory_order_relaxed) != prod_head)) {
            YieldProcessor();
        }
        _prod_tail.store(prod_next, std::memory_order_release);
    }

    template <typename... Args>
    T* get_raw(Args&&... args) {
        uint32_t cons_head, cons_next;
        T* ret = nullptr;

        do {
            uint32_t prod_tail = _prod_tail.load(std::memory_order_acquire);
            cons_head = _cons_head.load(std::memory_order_relaxed);
            uint32_t entries = prod_tail - cons_head;
            
            if(unlikely(entries == 0)) {
                ret = alloc_traits::allocate(*this, 1);
                alloc_traits::construct(*this, ret, std::forward<Args>(args)...);

#ifdef XHBLIB_OBJ_POOL_UNIT_TEST
                auto pre = _allocate_counter.fetch_add(1, std::memory_order_relaxed);
                ret->_i = pre;
                _allocate_all.fetch_add(pre, std::memory_order_relaxed);
#endif 
                return ret;
            }
            cons_next = cons_head + 1;

        } while (false == _cons_head.compare_exchange_weak(cons_head, cons_next, 
            std::memory_order_release, std::memory_order_relaxed));
        
        uint32_t idx = cons_head & MASK;
        ret = _objs[idx]._data;

        while (unlikely(_cons_tail.load(std::memory_order_relaxed) != cons_head)) {
            YieldProcessor();
        }
        _cons_tail.store(cons_next, std::memory_order_relaxed);

        if (sizeof...(args) != 0) {
            alloc_traits::destroy(*this, ret);
            alloc_traits::construct(*this, ret, std::forward<Args>(args)...);
        }
        return ret;
    }

    template <typename... Args>
    auto get(Args&&... args) {
        T* raw_ptr = get_raw(std::forward<Args>(args)...);
        auto deleter = [this](T* ptr)mutable { this->put_raw(ptr); };
        std::unique_ptr<T,std::function<void(T*)>> ret{raw_ptr, deleter};
        
        return ret;
    }
#ifdef XHBLIB_OBJ_POOL_UNIT_TEST
    void dump(std::vector<T*>& vec) {
        auto cons_head = _cons_head.load(std::memory_order_relaxed);
        auto prod_tail = _prod_tail.load(std::memory_order_relaxed);

        for (; cons_head != prod_tail; cons_head++) {
            auto idx = cons_head & MASK;
            vec.push_back(_objs[idx]._data);
        }
    }
#endif
};

} // xhb ns


#endif // XHBLIB_OBJECT_POOL_H_