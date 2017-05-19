#ifndef XHBLIB_FUNCTION_TRAITS_H_
#define XHBLIB_FUNCTION_TRAITS_H_

// 函数类型traits

#include <tuple>
#include <type_traits>

namespace xhb {

template<typename T>
struct func_traits;

template<typename R /*ret*/, typename... A /*args*/>
struct func_traits<R(A...)> {
    using return_type = R;
    using args_tuple = std::tuple<A...>;
    using type = R(A...);
    static constexpr std::size_t arity = sizeof...(A);
    template<std::size_t N>
    struct arg {
        static_assert(N < arity, "index invalid.");
        using type = typename std::tuple_element<N, args_tuple>::type;
    };
    template<std::size_t N>
    using args_type = typename arg<N>::type;
};

template<typename R, typename... A>
struct func_traits<R(*)(A...)> : public func_traits<R(A...)> {};

template<typename T, typename R, typename... A>
struct func_traits<R(T::*)(A...)> : public func_traits<R(A...)> {};

template<typename T, typename R, typename... A>
struct func_traits<R(T::*)(A...) const> : public func_traits<R(A...)> {};

template<typename T>
struct func_traits : public func_traits<decltype(&T::operator())> {};

template<typename T>
struct func_traits<T&> : public func_traits<std::remove_reference_t<T>> {};

} // xhb namespace

#endif // XHBLIB_FUNCTION_TRAITS_H_