#ifndef RING_QUEUE_H_
#define RING_QUEUE_H_

#include <memory> // for allocator
#include <algorithm>
#include <type_traits>
#include <iterator>
#include <stdexcept>

template < typename T,typename Alloc = std::allocator<T>>
class rueue final{
    struct impl:Alloc{
        T* data = nullptr;
        size_t beg = 0;
        size_t end = 0;
        size_t cap = 0; // always be power of 2
    };
    impl _impl;
public:
    using my_type = rueue<T,Alloc>;
    using allocator_type = Alloc;

    using value_type = T;
    using size_type = size_t;
    using reference = T&;
    using pointer = T*;
    using const_reference = const T&;
    using const_point = const T*;
public:
    rueue() = default;
    
    rueue(const my_type& other);
    rueue(my_type&& other);
    
    ~rueue();
    
    template<typename Iter>
    rueue(Iter first,Iter last,typename std::iterator_traits<Iter>::iterator_category* = nullptr);

    rueue& operator=(const my_type& other);
    rueue& operator=(my_type&& other);

    void push_back(const value_type& val);
    void push_back(value_type&& val);
    
    template<typename... Args>
    void emplace_back(Args&&... args);

    void pop_back();
    reference back();
    const_reference back() const;

    void push_front(const value_type& val);
    void push_front(value_type&& val);
    
    template<typename... Args>
    void emplace_front(Args&&... args);

    void pop_front();
    reference front();
    const_reference front() const;
    
    void swap(my_type& other);

    template<typename Iter>
        typename std::enable_if<!std::is_same<typename std::iterator_traits<Iter>::value_type,
            void>::value,void>::type
    assign(Iter first,Iter last);
    void assign(size_type count,const value_type& val);

    void clear(){
        destory();
        _impl = {};
    }

    bool empty() const{
        return _impl.beg = _impl.end;
    }

    size_type size() const{
        return _impl.beg - impl.end;
    }

    size_type capacity() const{
        return _impl.cap;
    }
    void reserve(size_type count);

    reference operator[](size_type idx){
         return _impl.data[_impl.beg + idx]; 
    }
    const_reference operatoro[](size_type idx)const{
         return _impl.data[_impl.beg + idx];
    }

    reference at(size_type idx){
        if(idx >= size()) throw std::out_of_range("out of range");
        return _impl.data[_impl.beg + idx];
    }
    const_reference at(size_type idx){
        if(idx >= size()) throw std::out_of_range("out of range");
        return _impl.data[_impl.beg + idx];
    }

private:
    
    size_type mask(size_type idx)const{
        return idx & (_impl.cap - 1);
    }

    size_type count_ceil_power(size_type num){
        return size_type(1) << count_leading_zero(num - 1);
    }

    size_type count_leading_zero(size_type num){
        size_type c = 0;
        while(num){
            c++;
            num >>= 1;
        }
        return c;
    }

    void destory(point first,point last){
        for(;first != last; ++first){
            _impl.destory(first);
        }
        _impl.deallcate(_impl.data,_impl.cap);
    }
};
#endif // RING_QUEUE_H_
