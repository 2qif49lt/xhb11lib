#ifndef XHBLIB_RING_QUEUE_H_
#define XHBLIB_RING_QUEUE_H_

// 一种比vector更高效的适配标准库的自增长高效双向队列。
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
    
    rueue(size_type count){
        realloc(count);
        construct_n(count,value_type());
    }
    rueue(size_type count,const value_type& val){
        realloc(count);
        construct_n(count,val);
    }
    ~rueue(){
        destroy();
        if(_impl.data != nullptr)
        {  
            alloc_traits::deallocate(_impl,_impl.data,_impl.cap);
            _impl.data = nullptr;
            _impl.cap = 0;
        }
    }
    
    template<typename Iter >
    rueue(Iter first,Iter last,
        typename std::enable_if<std::is_same<typename std::iterator_traits<Iter>::value_type,value_type>::value>::type* = nullptr){
        assign(first,last);
    }

    my_type& operator=(const my_type& other){
        if(this != &other){
            this->~my_type();
            new(this)my_type(other);
        }
        return *this;
    }
    my_type& operator=(my_type&& other){
        if(this != &other){
            this->~my_type();
            new(this)my_type(std::move(other));
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
        alloc_traits::destroy(_impl,&back());
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
        alloc_traits::destroy(_impl,&front());
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
        construct_iter(first,last);
    }
    void assign(size_type count,const value_type& val){
        destroy();
        reserve(count);
        construct_n(count,val);
    }

    void clear(){
        destroy();
    }

    bool empty() const{
        return _impl.beg == _impl.end;
    }

    size_type size() const{
        return _impl.end - _impl.beg;
    }
    size_type max_size()const{
        return std::numeric_limits<size_type>::max() / sizeof(value_type);
    }
    size_type capacity() const{
        return _impl.cap;
    }
    void reserve(size_type count){
        if(capacity() >= count) return;
        realloc(count);
    }

    reference operator[](size_type idx){
         if(idx >= size()) throw std::out_of_range("out of range");
         return _impl.data[mask(_impl.beg + idx)]; 
    }
    const_reference operator[](size_type idx)const{
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
        if(max_size() < count) throw std::invalid_argument("count is oversize.");

        count = size_policy(count);
        
        auto new_data = alloc_traits::allocate(_impl,count);
        if(new_data == nullptr) throw std::bad_alloc();
        
        auto d = new_data;
        auto s = _impl.beg;

        for(;s != _impl.end; ++s,++d){
            alloc_traits::construct(_impl,d,std::move(_impl.data[mask(s)]));
        }
        
        destroy_index(_impl.beg,_impl.end); // fXXk off strong exception safe!
       
        std::swap(_impl.data,new_data);
        std::swap(_impl.cap,count);

        _impl.beg = 0;
        _impl.end = d - _impl.data;

        if(new_data != nullptr)
            alloc_traits::deallocate(_impl,new_data,count);
    }
    void construct_n(size_type count,const value_type val){
        for(size_type idx = 0; idx != count; ++idx)
            alloc_traits::construct(_impl,_impl.data + mask(idx),val);
        _impl.end = count;
    }
    
    template<typename Iter>
    void construct_iter(Iter first,Iter last){
        auto count = std::distance(first,last);
        
        for(size_type idx = 0; first != last; ++first,++idx){
            alloc_traits::construct(_impl,_impl.data + mask(idx),*first);
        }
        _impl.end = count;
    }
    void destroy(){
        destroy_index(_impl.beg,_impl.end);
        _impl.beg = 0;
        _impl.end = 0;
    }
    void destroy_index(size_type first,size_type last){
        for(;first != last; ++first){
            alloc_traits::destroy(_impl,_impl.data + mask(first));
        }
    }
    
private:
    template<typename R,typename V>
    class rueue_iterator :public std::iterator<std::random_access_iterator_tag,V>{
    private:
        R* queue = nullptr;
        size_type idx = 0;
        
        
    public:
        using my_base = std::iterator<std::random_access_iterator_tag,V>;
        using my_type = rueue_iterator<R,V>;

        using value_type = typename my_base::value_type;
        using pointer = typename my_base::pointer;
        using reference = typename my_base::reference;
        using difference_type = typename my_base::difference_type;
    private:
        inline void check_same_queue(const my_type& other) const{
            if(queue != other.queue) throw std::runtime_error("rueue is not the same");
        }

    public:
        // the big 5 are all default;
        rueue_iterator() = default;
        rueue_iterator(const my_type&) = default;
        rueue_iterator(my_type&&) = default;
        rueue_iterator& operator=(const my_type&) = default;
        rueue_iterator& operator=(my_type&&) = default;
        ~rueue_iterator() = default;

        rueue_iterator(R* r,size_type n):queue(r),idx(n){};
        
        value_type& operator*() const{
            return queue->_impl.data[queue->mask(idx)];
        }

        value_type* operator->()const{
            return &queue->_impl.data[queue->mask(idx)];
        }
        
        // 前
        my_type& operator++(){
            idx++;
            return *this;
        }
        my_type& operator--(){
            idx--;
            return *this;
        }
        // 后
        my_type operator++(int){
            auto ret = *this;
            idx++;
            return ret;
        }
        my_type operator--(int){
            auto ret = *this;
            idx--;
            return ret;
        }

        my_type& operator+=(difference_type n){
            idx += n;
            return *this;
        }

        my_type& operator-=(difference_type n){
            idx -= n;
            return *this;
        }

        my_type operator+(difference_type n)const{
            return my_type(queue,idx + n);
        }
        my_type operator-(difference_type n)const{
            return my_type(queue,idx - n);
        }

        difference_type operator-(const my_type& other)const{
            check_same_queue(other);
            return idx - other.idx;
        }

        bool operator== (const my_type& rhs)const{
            check_same_queue(rhs);
            return idx == rhs.idx;
        }

        bool operator!= (const my_type& rhs)const{
            check_same_queue(rhs);
            return idx != rhs.idx;
        }

        bool operator<(const my_type& rhs)const{
            check_same_queue(rhs);
            return idx - queue->_impl.beg < rhs.idx - queue->_impl.beg;
        }

        bool operator>(const my_type& rhs)const{
            check_same_queue(rhs);
            return idx - queue->_impl.beg > rhs.idx - queue->_impl.beg;
        }

        bool operator<=(const my_type& rhs)const{
            check_same_queue(rhs);
            return idx - queue->_impl.beg <= rhs.idx - queue->_impl.beg;
        }

        bool operator>=(const my_type& rhs)const{
            check_same_queue(rhs);
            return idx - queue->_impl.beg >= rhs.idx - queue->_impl.beg;
        }
        //
        friend R;
    };

public:

    using iterator = rueue_iterator<my_type,value_type>;
    using const_iterator = rueue_iterator<const my_type,const value_type>;

    friend iterator;

    iterator begin(){
        return iterator(this,_impl.beg);
    }
    const_iterator begin()const{
        return const_iterator(this,_impl.beg); 
    }
    
    iterator end(){
        return iterator(this,_impl.end);
    }
    const_iterator end()const {
        return const_iterator(this,_impl.end);
    }
    
    const_iterator cbegin() const {
        return const_iterator(this, _impl.beg);
    }
    const_iterator cend() const {
        return const_iterator(this, _impl.end);
    }
    
    iterator erase(iterator first,iterator last){
        if(first == last) return last;
        if(first < begin() || last > end())
            throw std::out_of_range("erase iterator is out of range");
        
        if(std::distance(begin(),first) < std::distance(last, end())){
            auto new_beg = std::move_backward(begin(), first, last);
            std::for_each(begin(), new_beg, [this](value_type& item){alloc_traits::destroy(_impl,&item);});
            _impl.beg = new_beg.idx;
            return last;
        }else{
            auto new_end = std::move(last,end(),first);
            std::for_each(new_end,end(),[this](value_type& item){alloc_traits::destroy(_impl,&item);});
            _impl.end = new_end.idx;
            return first;
        }
    }
    
    iterator erase(iterator where){
        if (where >= end() || where < begin())
            throw std::out_of_range("erase iterator is out of range");
        
        if(std::distance(begin(), where) < std::distance(where, end())){
            auto new_beg = std::move_backward(begin(), where, where + 1);
            std::for_each(begin(), new_beg, [this](value_type& item){alloc_traits::destroy(_impl,&item);});
            _impl.beg = new_beg.idx;
        }else{
            auto new_end = std::move(where + 1,end(),where);
            std::for_each(new_end, end(), [this](value_type& item){alloc_traits::destroy(_impl,&item);});
            _impl.end = new_end.idx;
        }
        return where;
    }
    
    // insert 后迭代器会失效.该接口只是为了容器用途更广.效率低不建议使用。
    // insert before value before pos.
    // return iterator pointing to the inserted / first value
    //
    iterator insert(iterator pos,const value_type& value){
        if (pos > end() || pos < begin())
            throw std::out_of_range("erase iterator is out of range");
        
        return insert_n(pos, 1, value);
    }
    iterator insert(iterator pos,size_type count,const value_type& value){
        return insert_n(pos, count, value);
    }
    template<typename Iter>
    iterator insert(iterator pos,Iter first,Iter last){
        return insert_n(pos,first,last);
    }
private:
    size_type unused_capacity()const{return capacity() - size();}

    //
    iterator insert_n(iterator pos,size_type count,const value_type& value){
        if(count == 0) return pos;
        
        auto idx = std::distance(begin(), pos);
        reserve(size() + count);
        pos.idx = _impl.beg + idx;
        
        if(std::distance(begin(), pos) < std::distance(pos, end())){
            auto new_beg = begin() - count;
            
            auto insert_beg = std::move(begin(),pos,new_beg);
            
            std::for_each(insert_beg,pos,[this,&value](value_type& item){alloc_traits::construct(_impl,&item,value);});
            
            _impl.beg = new_beg.idx;
            
            return insert_beg;
        }else{
            auto new_end = end() + count;
            
            auto insert_end = std::move_backward(pos, end(),new_end);
            
            std::for_each(pos,insert_end,[this,&value](value_type& item){alloc_traits::construct(_impl,&item,value);});
            
            _impl.end = new_end.idx;
            
            return pos;
        }
        
    }
    template<typename Iter>
    iterator insert_n(iterator pos,Iter first,Iter last,
                     typename std::enable_if<std::is_same<typename std::iterator_traits<Iter>::value_type,value_type>::value>::type* = nullptr){
        auto count = std::distance(first, last);
        if(count == 0) return pos;
        
        auto idx = std::distance(begin(), pos);
        reserve(size() + count);
        pos.idx = _impl.beg + idx;
        
        if(std::distance(begin(), pos) < std::distance(pos, end())){
            auto new_beg = begin() - count;
            auto old_beg = begin();
            _impl.beg = new_beg.idx;

            std::uninitialized_copy(first,last,new_beg);
            std::rotate(new_beg,old_beg,pos);            
            
            return pos - count + 1;
        }else{
            auto new_end = end() + count;
            auto old_end = end();
            _impl.end = new_end.idx;

            std::uninitialized_copy(first,last,old_end);
            std::rotate(pos,old_end,end());
            
            return pos;
        }
    }
};
#endif // XHBLIB_RING_QUEUE_H_
