#ifndef XHBLIB_CLEANER_H_
#define XHBLIB_CLEANER_H_

// 一种在被管理对象的生命周期结束时执行携带动作链的分离式cleaner


#include <cstdlib>
#include <utility>
#include <memory>
#include <type_traits>


#ifdef XHBLIB_CLEANER_UNIT_TEST

#include <string>
#include <sstream>
#include <iostream>

using std::string;
using std::stringstream;
using std::cout;
using std::endl;

string assert_tags;
stringstream assert_ss;

#define XHBLIB_UNIT_TEST(tag) { assert_ss << tag; cout<<tag<<endl;}

#else

#define XHBLIB_UNIT_TEST(tag) 0

#endif



class cleaner final{
public:
    struct impl;
    struct cleaner_raw_point_tag{};
private:
    // 优化，很多时候只是管理缓存，且没有share情况,new char 等，这时直接使用hack过的地址，而不是impl结构体。更为节省内存
    impl* _impl = nullptr;

public:
    cleaner() = default;
    cleaner(const cleaner&) = delete;

    cleaner(cleaner&& other)noexcept : _impl(other._impl){other._impl = nullptr;}
    
    explicit cleaner(impl* i):_impl(i){}
    explicit cleaner(void* ptr,cleaner_raw_point_tag):_impl(from_raw_point(ptr)){}

    ~cleaner();

    cleaner& operator=(const cleaner&) = delete;
    cleaner& operator=(cleaner&& other){
        if(this != &other){
            this->~cleaner();
            new(this)cleaner(std::move(other));
        }
        return *this;
    }
    // share 返回一个指向相同的cleaner,内部引用计数+1,封装的动作将在2个cleaner都析构后执行。
    cleaner share();
    
    operator bool()const{return _impl != nullptr;}
     unsigned int refs()const;
private:
    static bool is_raw_point(impl* i){
        auto x = reinterpret_cast<uintptr_t>(i);
        return x & 1;
    }
    bool is_raw_point() const{
        return is_raw_point(_impl);
    }
    static void* to_raw_point(impl* i){
        auto x = reinterpret_cast<uintptr_t>(i);
        return reinterpret_cast<void*>(x & ~uintptr_t(1));
    }
    void* to_raw_point() const{
        return to_raw_point(_impl);
    }
    impl* from_raw_point(void* p){
        auto x = reinterpret_cast<uintptr_t>(p);
        return reinterpret_cast<impl*>(x | 1);
    }

};

struct cleaner::impl{
    unsigned int refs = 1;
    cleaner next;
    impl(cleaner n):next(std::move(n)){}
    virtual ~impl(){}
};


cleaner::~cleaner(){
    if(_impl == nullptr) return;
    
    if(is_raw_point()){
        std::free(to_raw_point());
        XHBLIB_UNIT_TEST('f');
        return;
    }
    
    if( --_impl->refs == 0){
        delete _impl;
    }
}


template<typename Action>
struct lambda_cleaner_impl final : public cleaner::impl{
    Action act;
    lambda_cleaner_impl(cleaner next,Action&& a):impl(std::move(next)),act(std::move(a)){}
    virtual ~lambda_cleaner_impl() override {
        act();
        XHBLIB_UNIT_TEST('l');
    }
};

template<typename Action>
inline
lambda_cleaner_impl<Action>* make_lambda_cleaner_impl(cleaner next,Action&& act){
    return new lambda_cleaner_impl<Action>(std::move(next),std::move(act));
}

template<typename Action>
cleaner make_lambda_cleaner(Action && act){
    return cleaner(make_lambda_cleaner_impl(cleaner(),std::move(act)));
}

template<typename Action>
cleaner make_lambda_cleaner(cleaner next,Action && act){
    return cleaner(make_lambda_cleaner_impl(std::move(next),std::move(act)));
}


struct free_cleaner_impl final : public cleaner::impl{
    void* ptr;
    free_cleaner_impl(void* p):impl(cleaner()),ptr(p){}
    virtual ~free_cleaner_impl()override{
        if(ptr != nullptr)std::free(ptr);
        XHBLIB_UNIT_TEST('f');
    }
};

inline cleaner make_free_cleaner(void* ptr){
    if(ptr == nullptr) return cleaner();
    return cleaner(ptr,cleaner::cleaner_raw_point_tag());
}

inline cleaner make_free_cleaner(cleaner next,void* ptr){
    return make_lambda_cleaner(std::move(next),[ptr]()mutable{std::free(ptr);});
}

inline cleaner cleaner::share(){
    if(_impl == nullptr) return cleaner();
    if(is_raw_point())
        _impl = new free_cleaner_impl(to_raw_point());
    ++_impl->refs;
    return cleaner(_impl);
}

unsigned int cleaner::refs()const{
    if(_impl == nullptr) return 0;
    if(is_raw_point()) return 1;
    return _impl->refs;
}

template<typename Object>
struct object_cleaner_impl final : public cleaner::impl{
    Object obj;
    object_cleaner_impl(cleaner next,Object&& o):impl(std::move(next)),obj(std::move(o)){}
    virtual ~object_cleaner_impl() override {
        XHBLIB_UNIT_TEST('o');
    }
};

template<typename Object>
inline
object_cleaner_impl<Object>* make_object_cleaner_impl(cleaner next,Object o){
    return new object_cleaner_impl<Object>(std::move(next),std::move(o));
}

template<typename T>
cleaner make_object_cleaner(T&& obj){
    return cleaner{make_object_cleaner_impl(cleaner(),std::move(obj))};
}

template<typename T>
cleaner make_object_cleaner(cleaner d,T&& obj){
    return cleaner{make_object_cleaner_impl(std::move(d),std::move(obj))};
}

// helper

template <class C>
class has_function_operator{
    template <class T>
    static std::true_type test_sign(void (T::*)(void));
    
    template <class T>
    static std::true_type test_sign(void (T::*)(void)const);
    
    template <class T>
    static decltype(test_sign(&T::operator())) test(std::nullptr_t);
    
    template <class T>
    static std::false_type test(...);
    
public:
    using type = decltype(test<C>(nullptr));
    static const bool value = type::value;
};



// free
template<typename T>
cleaner make_cleaner(cleaner next,T* obj,typename std::enable_if<std::is_pod<T>::value ||
                                            std::is_void<T>::value>::type* = nullptr){
    return make_free_cleaner(std::move(next),static_cast<void*>(obj));
}
template<typename T>
cleaner make_cleaner(T* obj,typename std::enable_if<std::is_pod<T>::value ||
                                std::is_void<T>::value>::type* = nullptr){
    return make_free_cleaner(static_cast<void*>(obj));
}



// lambda void() 
template<typename T>
cleaner make_cleaner(cleaner next,T&& obj,typename std::enable_if<has_function_operator<typename std::remove_cv<typename std::remove_reference<T>::type>::type>::value ||
                                                        (std::is_function<typename std::remove_reference<T>::type>::value &&
                                                            std::is_same<typename std::remove_reference<T>::type,void()>::value)>::type* = nullptr){
    return make_lambda_cleaner(std::move(next),std::move(obj));
}
template<typename T>
cleaner make_cleaner(T&& obj,typename std::enable_if<(std::is_function<typename std::remove_reference<T>::type>::value &&
                                                      std::is_same<typename std::remove_reference<T>::type,void()>::value) ||
                            has_function_operator<typename std::remove_reference<T>::type>::value>::type* = nullptr){
    return make_lambda_cleaner(std::move(obj));
}

// obj
template<typename T>
cleaner make_cleaner(cleaner next,T&& obj,typename std::enable_if<!std::is_trivially_destructible<T>::value &&
                                            !std::is_function<typename std::remove_reference<T>::type>::value &&
                                            !has_function_operator<typename std::remove_reference<T>::type>::value>::type* = nullptr){
    return make_object_cleaner(std::move(next),std::move(obj));
}
template<typename T>
cleaner make_cleaner(T&& obj,typename std::enable_if<!std::is_trivially_destructible<typename std::remove_reference<T>::type>::value &&
                     !std::is_function<typename std::remove_reference<T>::type>::value &&
                     !has_function_operator<typename std::remove_reference<T>::type>::value>::type* = nullptr){
    return make_object_cleaner(std::move(obj));
}

#endif // XHBLIB_CLEANER_H_
