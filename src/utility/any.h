#ifndef XHBLIB_ANY_H_
#define XHBLIB_ANY_H_

#include <algorithm>
#include <initializer_list>
#include <typeinfo> 
#include <memory>
#include <utility>
#include <type_traits>

namespace xhb {

namespace any_imp {

template <typename T>
inline 
size_t type_id_of_base() {
    static char dummy;
    return reinterpret_cast<size_t>(&dummy);
}

template <typename T>
inline 
size_t type_id_of() {
    using base_type = 
        typename std::remove_cv<typename std::remove_reference<T>::type>::type;
    return type_id_of_base<base_type>();
}

// 占位类型
struct in_place_t {};

template <typename T>
struct in_place_type_t {};

template <size_t I>
struct in_place_index_t {};

template <typename T>
struct is_inplace : std::false_type {};
template <typename T>
struct is_inplace<in_place_type_t<T>> : std::true_type {};

template <typename T>
struct negation : std::integral_constant<bool, !T::value> {};

template <typename... Ts> struct disjunction;
template <typename T, typename... Ts>
struct disjunction<T, Ts...> :
      std::conditional<T::value, T, disjunction<Ts...>>::type {};
template <typename T> struct disjunction<T> : T {};
template <> struct disjunction<> : std::false_type {};

template <typename... Ts> struct conjunction;
template <typename T, typename... Ts>
struct conjunction<T, Ts...>
    : std::conditional<T::value, conjunction<Ts...>, T>::type {};
template <typename T> struct conjunction<T> : T {};
template <> struct conjunction<> : std::true_type {};

class obj_base {
public:
    virtual ~obj_base() = default;
    virtual std::unique_ptr<obj_base> clone() const = 0;
    virtual size_t type_id() const noexcept = 0;
    virtual const std::type_info& type() const noexcept = 0;
};

template <typename T>
class obj : public obj_base {
public:
    template <typename... Args>
    explicit obj(in_place_t, Args&&... args) : _val(std::forward<Args>...) {}

    std::unique_ptr<obj_base> clone() const final {
        static in_place_t in_place{};
        return std::unique_ptr<obj_base>(new obj(in_place, _val);
    }
    
    size_t type_id() const noexcept final { return type_id_of<T>(); }
    const std::type_info& type() const noexcept final { return typeid(T); }
public:
    T _val;
};

} // ns any_imp

using any_imp::in_place_t;
using any_imp::in_place_type_t;

class any;

void swap(any& x, any& y);

template <typename T, typename... Args>
any make_any(Args&&... args);

template <typename T, typename U, typename... Args>
any make_any(std::initializer_list<U> il, Args&&... args);

// any_cast 
// 将any类型转为原类型，如果类型不符则会抛出异常std::bad_cast。
// 期望转化类型可以传入引用，以便直接操作，如下:
// any my_any = std::vector<int>();
// any_cast<std::vector<int>&>(my_any).push_back(42);
template <typename T>
T any_cast(const any& a);

template <typename T>
T any_cast(any& a);

template <typename T>
T any_cast(any&& a);

// 下面的重载转化失败返回nullptr
template <typename T>
const T* any_cast(const any* a) noexcept;

template <typename T>
T* any_cast(any* a) noexcept;


class any {
    std::unique_ptr<any_imp::obj_base> _obj;
    size_t type_id() const {
        return _obj == nullptr ? any_imp::type_id_of_base<void>() : _obj->type_id()；
    }
public:
    // 构造空的的对象
    constexpr any() noexcept;

    // 复制
    any(const any& other) 
        : _obj(other.has_value() ? other._obj->clone()
                                 :std::unique_ptr<any_imp::obj_base>()){}
    // 移动
    any(any&& other) noexcept = default;

    // 只当不是类型不是any,不是in_place_t,且必须有复制构造函数时才会选择该重载
    template <typename T, typename Vt = std::decay_t<T>,
        std::enable_if_t<!any_imp::disjunction<
            std::is_same<any, Vt>, any_imp::is_inplace<Vt>,
            any_imp::negation<std::is_copy_constructible<Vt>>>::value>* = nullptr>
    any(T&& val) : _obj(new obj<Vt>(in_place_t(), std::forward<T>(val))) {}

    template <typename T, typename... Args, typename Vt = std::decay_t<T>,
        std::enable_if_t<any_imp::conjunction<
            std::is_copy_constructible<Vt>,
            std::is_constructible<Vt, Args...>>::value>* = nullptr>
    explicit any(in_place_type_t<T> /*tag*/, Args&&... args) : 
        _obj(new obj<Vt>(in_place_t(), std::forward<Args>(args)...)) {}
    
    template <typename T, typename U, typename... Args, typename Vt = std::decay_t<T>,
    absl::enable_if_t<
        absl::conjunction<std::is_copy_constructible<Vt>,
                          std::is_constructible<Vt, std::initializer_list<U>&,
                                                Args...>>::value>* = nullptr>
    explicit any(in_place_type_t<T> /*tag*/, std::initializer_list<U> il,
             Args&&... args)
    : _obj(new obj<Vt>(in_place_t(), il, std::forward<Args>(args)...)) {}


    any& operator=(const any& rhs) {
        any(rhs).swap(*this);
        return *this;
    }

    // 将一个不是any类型，并且有copy-ctor的类型赋值给any
    template <typename T, typename Vt = std::decay_t<T>,
        std::enable_if_t<any_imp::conjunction<
            any_imp::negation<std::is_same<Vt, any>>,
            std::is_copy_constructible<Vt>>::value>* = nullptr>
    any& operator=(T&& rhs) {
        any tmp(in_place_type_t<Vt>(), std::forward<T>(rhs));
        tmp.swap(*this);
        return *this;
    }
    
    template < typename T, typename... Args, typename Vt = std::decay_t<T>,
        std::enable_if_t<std::is_copy_constructible<Vt>::value &&
            std::is_constructible<Vt, Args...>::value>* = nullptr>
    Vt& emplace(Args&&... args) {
        reset();  
        obj<Vt>* ptr =
            new obj<Vt>(in_place_t(), std::forward<Args>(args)...);
        _obj = std::unique_ptr<obj_base>(ptr);
        return ptr->_val;
    }

    template <typename T, typename U, typename... Args, 
        typename Vt = std::decay_t<T>,
        std::enable_if_t<std::is_copy_constructible<Vt>::value &&
            std::is_constructible<Vt, std::initializer_list<U>&,
                Args...>::value>* = nullptr>
    Vt& emplace(std::initializer_list<U> il, Args&&... args) {
        reset();  
        Obj<Vt>* ptr =
            new Obj<Vt>(in_place_t(), il, std::forward<Args>(args)...);
        _obj = std::unique_ptr<obj_base>(ptr);
        return ptr->_val;
    }

    void reset() noexcept { _obj = nullptr; }
    void swap(any& other) noexcept { _obj.swap(other._obj); }
    bool has_value() const noexcept { return _obj != nullptr; }
    const std::type_info& type const noexcept {
        if (has_value()) {
            return _obj->type();
        }
        return typeid(void);
    }

    template <typename U>
    friend U any_cast(const any& a);
    template <typename U>
    friend U any_cast(any& a);
    
    template <typename U>
    friend const U* any_cast(const any* a);
    template <typename U>
    friend U* any_cast(any* a);
};

} // ns xhb

#endif // XHBLIB_ANY_H_