
#ifndef XHBLIB_UTILTIY_SAFE_BOOL_H_
#define XHBLIB_UTILTIY_SAFE_BOOL_H_

#include <iostream>

/*
    struct foo_tag { };
    using foo = safe_bool<foo_tag>;

    struct bar_tag { };
    using bar = safe_bool<bar_tag>;

    foo v1 = foo::yes; // OK
    bar v2 = foo::yes; // ERROR, no implicit cast
    foo v4 = v1 || foo::no; // OK
    bar v5 = bar::yes && bar(true); // OK
    bool v6 = v5; // ERROR, no implicit cast
*/ 


namespace xhb {

// Tag type used as a tag
template<typename Tag>
class safe_bool {
    bool _value;
public:
    static const safe_bool yes;
    static const safe_bool no;
    // 默认构造一个false
     safe_bool() noexcept : _value(false) { }

    // 显示构造
     explicit safe_bool(bool v) noexcept : _value(v) { }

    // 显示转换为bool
    explicit operator bool() const noexcept { return _value; }

    // Logical OR.
    friend safe_bool operator||(safe_bool x, safe_bool y) noexcept {
        return safe_bool(x._value || y._value);
    }

    /// Logical AND.
    friend safe_bool operator&&(safe_bool x, safe_bool y) noexcept {
        return safe_bool(x._value && y._value);
    }

    /// Logical NOT.
    friend safe_bool operator!(safe_bool x) noexcept {
        return safe_bool(!x._value);
    }

    /// Equal-to operator.
    friend bool operator==(safe_bool x, safe_bool y) noexcept {
        return x._value == y._value;
    }

    /// Not-equal-to operator.
    friend bool operator!=(safe_bool x, safe_bool y) noexcept {
        return x._value != y._value;
    }

    /// Prints safe_bool value to an output stream.
    // std::cout << safe_bool 会成功是因为根据ADL( Argument Dependent Lookup )，当前namespace 和operator<<参数所在的namespace
    // 都在搜索范围
    friend std::ostream& operator<<(std::ostream& os, safe_bool v) {
        return os << (v._value ? "true" : "false");
    }

    
};

template<typename Tag>
const safe_bool<Tag> safe_bool<Tag>::yes(true);
template<typename Tag>
const safe_bool<Tag> safe_bool<Tag>::no(false);

} // xhb namespace


#endif // XHBLIB_UTILTIY_SAFE_BOOL_H_