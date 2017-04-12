#ifndef XHBLIB_CLEANER_H_
#define XHBLIB_CLEANER_H_

// 一种在被管理对象的生命周期结束时执行携带动作链的分离式cleaner


#include <cstdlib>
#include <utility>
#include <memory>
#include <type_traits>

class cleaner final{
public:
    struct impl{
        unsigned int refs = 1;
        cleaner next;
        impl(cleaner n):next(std::move(n)){}
        virtual ~impl(){}  
    };
    struct cleaner_raw_point_tag{};
private:
    // 优化，很多时候只是管理缓存，且没有share情况,new char 等，这时直接使用hack过的地址，而不是impl结构体。更为节省内存
    impl* _impl = nullptr;

public:
    cleaner() = default;
    cleaner(const cleaner&) = delete;

    cleaner(cleaner&& other)noexcept : _impl(other._impl){other._impl == nullptr;}
    
    explicit cleaner(impl* i):_impl(i){}
    explicit cleaner(void* ptr,cleaner_raw_point_tag):_impl(from_raw_point(ptr)){}

    ~cleaner(){
        if(_impl == nullptr) return;

        if(is_raw_point()){
            std::free(to_raw_point());
            return;
        }

        if( --_impl->refs == 0)
            delete _impl;
    }

    cleaner& operator=(const cleaner&) = delete;
    cleaner& operator=(cleaner&& other){
        if(this != &other){
            this->~cleaner();
            new(this)cleaner(std::move(other));
        }
        return *this;
    }
    // share 返回一个指向相同的cleaner,内部引用计数+1,封装的动作将在2个cleaner都析构后执行。
    inline cleaner share(){
        if(_impl == nullptr) return cleaner();
        if(is_raw_point())
            _impl = new free_cleaner_impl(to_raw_point());
        ++_impl.refs;
        return cleaner(_impl);
    }

private:
    static bool is_raw_point(impl* i){
        auto x = reinterpret_cast<uintptr_t>(i);
        return x & 1;
    }
    bool is_raw_point(){
        return is_raw_point(_impl);
    }
    static void* to_raw_point(impl* i){
        auto x = reinterpret_cast<uintptr_t>(i);
        return reinterpret_cast<void*>(x & ~uintptr_t(1));
    }
    void* to_raw_point(){
        return to_raw_point(_impl);
    }
    impl* from_raw_point(void* p){
        auto x = reinterpret_cast<uintptr_t>(p);
        return reinterpret_cast<void*>(x | 1);
    }

};

template<typename Action>
struct lambda_cleaner_impl final : public cleaner::impl{
    Action act;
    lambda_cleaner_impl(cleaner next,Action&& a):impl(std::move(next)),act(std::move(a)){}
    virtual ~lambda_cleaner_impl() override {act();}
};

template<typename Action>
inline
lambda_cleaner_impl<Action>* make_lambda_cleaner_impl(cleaner next,Action&& act){
    return new lambda_cleaner_impl<Action>(std::move(next),std::move(act));
}

template<typename Action>
cleaner make_lambda_cleaner(Action && act){
    return make_lambda_cleaner_impl(cleaner(),std::move(act));
}

template<typename Action>
cleaner make_lambda_cleaner(cleaner next,Action && act){
    return make_lambda_cleaner_impl(std::move(next),std::move(act));
}

template<typename Object>
struct object_cleaner_impl final : public cleaner::impl{
    Object obj;
    object_cleaner_impl(cleaner next,object&& o):impl(std::move(next)),obj(std::move(o)){}
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

struct free_cleaner_impl final : public cleaner::impl{
    void* ptr;
    free_cleaner_impl(void* p):impl(cleaner()),ptr(p){}
    virtual ~free_cleaner_impl()override{if(ptr != nullptr)std::free(ptr);}
};

inline cleaner make_free_cleaner(void* ptr){
    if(ptr == nullptr) return cleaner();
    return cleaner(ptr,cleaner::cleaner_raw_point_tag());
}

inline cleaner make_free_cleaner(cleaner next,void* ptr){
    return make_lambda_cleaner(std::move(next),[ptr]()mutable{std::free(ptr);});
}


// free
template<typename T>
cleaner make_cleaner(cleaner next,T* obj,std::enable_if<std::is_pod<T>::value>::type* = nullptr){
    return make_free_cleaner(std::move(next),static_cast<void*>(obj));
}

// lambda
template<typename T>
cleaner make_cleaner(cleaner next,T&& obj,std::enable_if<std::is_trivially_destructible<T>::value && 
                                                        std::is_invocable<T>::value>::type* = nullptr){
    return make_lambda_cleaner(std::move(next),std::move(obj));
}

// obj
template<typename T>
cleaner make_cleaner(cleaner next,T&& obj,std::enable_if<!std::is_trivially_destructible<T>::value && 
                                                        !std::is_invocable<T>::value>::type* = nullptr){
    return make_object_cleaner(std::move(next),std::move(obj));
}
#endif // XHBLIB_CLEANER_H_