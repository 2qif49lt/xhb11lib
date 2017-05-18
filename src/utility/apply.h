#ifndef XHBLIB_APPLY_H_
#define XHBLIB_APPLY_H_

// http://en.cppreference.com/w/cpp/experimental/apply
// http://en.cppreference.com/w/cpp/utility/integer_sequence

// apply Invoke the Callable object f with a tuple of arguments.

#include <tuple>
#include <utility> // forward

namespace xhb {

template<typename F, typename T, typename I>
struct apply_helper;

template<typename F, typename T, size_t... I>
struct apply_helper<F, T, std::index_sequence<I...>> {
    static decltype(auto) apply(F&& fun, T&& tup) {
        return fun(std::get<I>(std::forward<T>(tup))...);
    }
};


template<typename F, typename... T>
decltype(auto) apply(F&& fun, std::tuple<T...>&& tup) {
    constexpr auto SIZE = std::tuple_size<std::tuple<T...>>::value;
    return apply_helper<F, std::tuple<T...>&&, std::make_index_sequence<SIZE>>::apply(std::forward<F>(fun),
        std::move(tup));
}

template<typename F, typename... T>
decltype(auto) apply(F&& fun, std::tuple<T...>& tup) {
    constexpr auto SIZE = std::tuple_size<std::tuple<T...>>::value;
    return apply_helper<F, std::tuple<T...>&, std::make_index_sequence<SIZE>>::apply(std::forward<F>(fun),
        tup);
}

template<typename F, typename... T>
decltype(auto) apply(F&& fun, const std::tuple<T...>& tup) {
    constexpr auto SIZE = std::tuple_size<std::tuple<T...>>::value;
    return apply_helper<F, const std::tuple<T...>&, std::make_index_sequence<SIZE>>::apply(std::forward<F>(fun),
        tup);
}

template<typename F, typename... A>
decltype(auto) apply(F&& fun, A&&... args) {
    return fun(std::forward<A>(args)...);
}

} // xhb namespace

#endif // XHBLIB_APPLY_H_
