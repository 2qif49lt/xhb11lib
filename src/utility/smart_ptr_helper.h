#ifndef XHBLIB_UTILITY_SMART_POINTER_HELPER_H_
#define XHBLIB_UTILITY_SMART_POINTER_HELPER_H_

#include <memory>

namespace xhb {

// 直接转换，如果不是父子类型则会出现异常。
template<typename D /*derived class*/, typename B /*base class*/, typename Del /*deleter*/>
std::unique_ptr<D, Del> 
unique_ptr_cast(std::unique_ptr<B, Del>&& p) {
    auto d = static_cast<D *>(p.release());
    return std::unique_ptr<D, Del>(d, std::move(p.get_deleter()));
}

template<typename D /*derived class*/, typename B /*base class*/>
std::unique_ptr<D> 
unique_ptr_cast(std::unique_ptr<B>&& p) {
    auto d = static_cast<D *>(p.release());
    return std::unique_ptr<D>(d);
}

// 安全版
template<typename D /*derived class*/, typename B /*base class*/, typename Del /*deleter*/>
std::unique_ptr<D, Del> 
dynamic_unique_ptr_cast(std::unique_ptr<B, Del>&& p) {
    if (D *result = dynamic_cast<D*>(p.get())) {
        p.release();
        return std::unique_ptr<D, Del>(result, std::move(p.get_deleter()));
    }
    return std::unique_ptr<D, Del>(nullptr, p.get_deleter());
}

template<typename D /*derived class*/, typename B /*base class*/>
std::unique_ptr<D> 
dynamic_unique_ptr_cast(std::unique_ptr<B>&& p) {
    if (D *result = dynamic_cast<D*>(p.get())) {
        p.release();
        return std::unique_ptr<D>(result);
    }
    return std::unique_ptr<D>(nullptr);
}

// 判断是否是智能指针

template<typename T>
struct is_smart_ptr : std::false_type {};

template<typename T>
struct is_smart_ptr<std::unique_ptr<T>> : std::true_type {};

template<typename T, typename D>
struct is_smart_ptr<std::unique_ptr<T, D>> : std::true_type {};
    

template<typename T>
struct is_smart_ptr<std::shared_ptr<T>> : std::true_type {};

} // xhb namespace
#endif // XHBLIB_UTILITY_SMART_POINTER_HELPER_H_
