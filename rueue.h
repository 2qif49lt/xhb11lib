#ifndef RING_QUEUE_H_
#define RING_QUEUE_H_

#include <memory> // for allocator
#include <algorithm>
#include <type_traits>
#include <iterator>
#include <stdexcept>
#include <limits>

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

    using iterator = T*;
    using const_iterator = const T*;

private:
    using alloc_traits = std::allocator_traits<allocator_type>;

public:
    rueue() = default;
    
    rueue(const my_type& other){
        alloc_init(other.size());
        copy_init(other);
    }
    rueue(my_type&& other):_impl(std::move(other._impl)){
        other._impl = {};
    }
    
    ~rueue(){
        destory();
        if(_impl.data != nullptr)
            alloc_traits::deallocate(_impl,_impl.data,_impl.cap);
        _impl.data = nullptr;
        _impl.cap = 0;
    }
    
    template<typename Iter >
    rueue(Iter first,Iter last,
        typename std::enable_if<std::is_same<typename std::iterator_traits<Iter>::value_type,value_type>::value>::type* = nullptr){
        assign(first,last);
    }

    rueue& operator=(const my_type& other){
        if(this != &other){
            this->~queue();
            new(this)rueue(std::begin(other),std::end(other));
        }
        return *this;
    }
    rueue& operator=(my_type&& other){
        if(this != &other){
            this->~rueue();
            new(this)rueue(std::move(other));
        }
        return *this;
    }

    void push_back(const value_type& val){
        may_realloc(1);
        auto end = _impl.data + mask(_impl.end);
        alloc_traits::construct(_impl,end,val);
        ++_impl.end;
    }
    void push_back(value_type&& val){
        may_realloc(1);
        auto end = _impl.data + mask(_impl.end);
        alloc_traits::construct(_impl,end,std::move(val));
        ++_impl.end;
    }
    
    template<typename... Args>
    void emplace_back(Args&&... args){
        may_realloc(1);
        auto end = _impl.data + mask(_impl.end);
        alloc_traits::construct(_impl,end,std::forward<Args>(args)...);
        ++_impl.end;
    }

    void pop_back(){
        if(empty()) return;
        alloc_traits::destory(_impl,&back());
        --_impl.end;
    }
    reference back(){
        return _impl.data[mask(_impl.end - 1)];
    }
    const_reference back() const{
        return _impl.data[mask(_impl.end - 1)];
    }

    void push_front(const value_type& val){
        may_realloc(1);
        auto f = _impl.data + mask(_impl.beg - 1);
        alloc_traits::construct(_impl,f,val);
        --_impl.beg;
    }
    void push_front(value_type&& val){
        may_realloc(1);
        auto f = _impl.data + mask(_impl.beg - 1);
        alloc_traits::construct(_impl,f,std::move(val));
        --_impl.beg;
    }
    
    template<typename... Args>
    void emplace_front(Args&&... args){
       may_realloc(1);
        auto f = _impl.data + mask(_impl.beg - 1);
        alloc_traits::construct(_impl,f,std::forward<Args>(args)...);
        --_impl.beg; 
    }

    void pop_front(){
        if(empty()) return;
        alloc_traits::destory(_impl,&front());
        ++_impl.beg;
    }
    reference front(){
        return _impl.data[mask(_impl.beg)];
    }
    const_reference front() const{
        return _impl.data[mask(_impl.beg)];
    }
    
    void swap(my_type& other){
        if(this == &other) return;
        std::swap(this->_impl,other._impl);
    }

    template<typename Iter>
        typename std::enable_if<!std::is_same<typename std::iterator_traits<Iter>::value_type,
            void>::value,void>::type
    assign(Iter first,Iter last){
        destroy();
        reserve(std::distance(first,last));
        for(;first != last; ++first){
            push_back(*first);
        }
    }
    void assign(size_type count,const value_type& val){
        destroy();
        reserve(count);
        while(count--){
            push_back(val);
        }
    }

    void clear(){
        destroy();
    }

    bool empty() const{
        return _impl.beg == _impl.end;
    }

    size_type size() const{
        return _impl.end - impl.beg;
    }
    size_type max_size()const{
        return std::numeric_limits<size_type>::max() / sizeof(value_type);
    }
    size_type capacity() const{
        return _impl.cap;
    }
    void reserve(size_type count){
        if(capacity() >= count) return;
        if(count >= max_size()) throw std::invalid_argument("count is oversize.");
        realloc(count);
    }

    reference operator[](size_type idx){
         if(idx >= size()) throw std::out_of_range("out of range");
         return _impl.data[mask(_impl.beg + idx)]; 
    }
    const_reference operatoro[](size_type idx)const{
        if(idx >= size()) throw std::out_of_range("out of range");
         return _impl.data[mask(_impl.beg + idx)];
    }

    reference at(size_type idx){
        if(idx >= size()) throw std::out_of_range("out of range");
        return _impl.data[mask(_impl.beg + idx)];
    }
    const_reference at(size_type idx)const{
        if(idx >= size()) throw std::out_of_range("out of range");
        return _impl.data[mask(_impl.beg + idx)];
    }

private:
    size_type mask(size_type idx)const{
        return idx & (_impl.cap - 1);
    }
    size_type size_policy(size_type count)const{
         if(count < 32) 
            return  32;
        else
            return count_ceil_power(count);
    }
    size_type count_ceil_power(size_type num)const{
        return size_type(1) << count_leading_zero(num - 1);
    }

    size_type count_leading_zero(size_type num)const{
        size_type c = 0;
        while(num){
            c++;
            num >>= 1;
        }
        return c;
    }
    void may_realloc(size_type count){
        if(size() + count > capacity()){
          reserve(size() + count);
        }
    }
    void alloc_init(size_type count){
        count = size_policy(count);
        _impl.data = alloc_traits::allocate(_impl,count);
        _impl.cap = count;
        _impl.beg = 0;
        _impl.end = 0;
    }
    void copy_init(const my_type& other){
        for(size_type idx = 0; idx != other.size(); ++idx){
            alloc_traits::construct(_impl,_impl.data + mask(_impl.beg + idx),other[idx]);
        }
        _impl.end = other.size();
    }
    void realloc(size_type count){
        count = size_policy(count);
        
        auto new_data = alloc_traits::allocate(_impl,count);
        if(new_data == nullptr) throw std::bad_alloc();
        
        auto d = new_data;
        auto s = _impl.beg;

        for(;s != _impl.end; ++s,++d){
            alloc_traits::construct(_impl,d,_impl.data[mask(s)]);
        }
       
        if(flase == std::is_move_constructible<value_type>::value)
            destroy(_impl.beg,_impl.end);
       
        std::swap(_impl.data,new_data);
        std::swap(_impl.cap,count);

        _impl.beg = 0;
        _impl.end = d - _impl.data;

        if(new_data != nullptr)
            alloc_traits::deallocate(_impl,new_data,count)
    }

    void destroy(){
        destroy(_impl.beg,_impl.end);
        _impl.beg = 0;
        _impl.end = 0;
    }
    void destroy(size_type first,size_type last){
        for(;first != last; ++first){
            alloc_traits::destroy(_impl,_impl.data + mask(first));
        }
    }
};
#endif // RING_QUEUE_H_
