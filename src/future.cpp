#include "future.h"


namespace xhb {

template <typename... T, typename... A>
future<T...> make_ready_future(A&&... value) {
    return future<T...>(ready_future_marker(), std::forward<A>(value)...);
}

template <typename... T, typename... A>
future<T...> make_ready_future(std::tuple<A...>&& tup) {
    return future<T...>(ready_future_from_tuple_marker(), std::forward<std::tuple<A...>>(tup));
}


template <typename... T>
future<T...> make_exception_future(std::exception_ptr expt) noexcept {
    return future<T...>(exception_future_marker(), std::move(expt));
}

template <typename... T, typename E /*exception*/>
future<T...> make_exception_future(E&& ex) noexcept {
    return make_exception_future<T...>(std::make_exception_ptr(std::forward<E>(ex)));
}

void future_state<>::forward_to(promise<>& pr) noexcept {
    assert(_state != state::future && _state != state::invalid);
    if (_state == state::exception) {
            pr.set_urgent_exception(std::move(_expt));
    } else {
        pr.set_urgent_value(std::tuple<>());
    }
    _state = state::invalid;
}




template <typename T>
inline
future<T>
futurize<T>::from_tuple(std::tuple<T>&& value) {
    return make_ready_future<T>(std::move(value));
}

template <typename T>
inline
future<T>
futurize<T>::from_tuple(const std::tuple<T>& value) {
    return make_ready_future<T>(value);
}

inline
future<>
futurize<void>::from_tuple(std::tuple<>&& value) {
    return make_ready_future<>();
}

inline
future<>
futurize<void>::from_tuple(const std::tuple<>& value) {
    return make_ready_future<>();
}


// futurize<T>

template<typename T>
template<typename E>
typename futurize<T>::type futurize<T>::make_exception_future(E&& ex) {
    return xhb::make_exception_future<T>(std::forward<E>(ex));
}

template<typename T>
template<typename F, typename... As>
typename futurize<T>::type futurize<T>::apply(F&& func, std::tuple<As...>&& args) noexcept {
    try {
        return convert(xhb::apply(std::forward<F>(func), std::move(args)));
    } catch (...) {
        return make_exception_future(std::current_exception()); 
    }
}
template<typename T>
template<typename F, typename... As>
typename futurize<T>::type futurize<T>::apply(F&& func, As&&... args) noexcept {
    try {
        return convert(xhb::apply(std::forward<F>(func), std::forward<As>(args)...));
    } catch (...) {
        return make_exception_future(std::current_exception());
    }
}


// futurie<void>

template<typename E>
typename futurize<void>::type futurize<void>::make_exception_future(E&& ex) {
    return xhb::make_exception_future<>(std::forward<E>(ex));
}

template<typename F, typename... As>
std::enable_if_t<is_future_v<std::result_of_t<F(As&&...)>>, future<>>
do_void_futurize_apply_args(F&& func, As&&... args) noexcept {
    try {
        return xhb::apply(func, std::forward<As>(args)...);
    } catch (...) {
        return make_exception_future(std::current_exception());
    }
}
template<typename F, typename... As>
std::enable_if_t<!is_future_v<std::result_of_t<F(As&&...)>>, future<>>
do_void_futurize_apply_args(F&& func, As&&... args) noexcept {
    try {
        xhb::apply(func, std::forward<As>(args)...);
        return make_ready_future<>();
    } catch (...) {
        return make_exception_future(std::current_exception());
    }
}

template<typename F, typename... As>
std::enable_if_t<is_future_v<std::result_of_t<F(As&&...)>>, future<>>
do_void_futurize_apply_tuple(F&& func, std::tuple<As...>&& tup) noexcept {
    try {
        return xhb::apply(std::forward<F>(func), std::move(tup));
    } catch (...) {
        return make_exception_future(std::current_exception());
    }
}
template<typename F, typename... As>
std::enable_if_t<!is_future_v<std::result_of_t<F(As&&...)>>, future<>>
do_void_futurize_apply_tuple(F&& func, std::tuple<As...>&& tup) noexcept {
    try {
        xhb::apply(std::forward<F>(func), std::move(tup));
        return make_ready_future<>();
    } catch (...) {
        return make_exception_future(std::current_exception());
    }
}


template<typename F, typename... As>
typename futurize<void>::type futurize<void>::apply(F&& func, std::tuple<As...>&& args) noexcept {
    return do_void_futurize_apply_tuple(std::forward<F>(func), std::move(args));
}
template<typename F, typename... As>
typename futurize<void>::type futurize<void>::apply(F&& func, As&&... args) noexcept {
    return do_void_futurize_apply_args(std::forward<F>(func), std::forward<As>(args)...);
}


// futurize<future<T...>>

template<typename... T>
template<typename E>
future<T...> futurize<future<T...>>::make_exception_future(E&& ex) {
    return xhb::make_exception_future<T...>(std::forward<E>(ex));
}

template<typename... T>
template<typename F, typename... As>
typename futurize<future<T...>>::type futurize<future<T...>>::apply(F&& func, As&&... args) noexcept{
    try {
        return xhb::apply(std::forward<F>(func), std::forward<As>(args)...);
    } catch (...) {
        return make_exception_future(std::current_exception());
    }
}
template<typename... T>
template<typename F, typename... As>
typename futurize<future<T...>>::type futurize<future<T...>>::apply(F&& func, std::tuple<As...>&& tup) noexcept{
    try {
        return xhb::apply(std::forward<F>(func), std::move(tup));
    } catch (...) {
        return make_exception_future(std::current_exception());
    }
}


// future

template <typename... T>
future<> future<T...>::or_terminate() noexcept {
     return then_wrapped([] (auto&& f) {
        try {
            f.get();
        } catch (...) {
            engine_exit(std::current_exception());
        }
    });
}

template<typename... T>
template<typename F>
future<T...> future<T...>::handle_exception_type(F&& func) noexcept {
    using trait = func_traits<F>;
    static_assert(trait::arity == 1, "func can take only one parameter");
    using ex_type = typename trait::template arg_t<0>;
    using ret_type = typename trait::return_type;
    return then_wrapped([func = std::forward<F>(func)] (auto&& fut) -> future<T...> {
        try {
            return make_ready_future<T...>(fut.get());
        } catch (ex_type& ex) {
            return futurize<ret_type>::apply(func, ex);
        }
    });
}

// utility
template<typename F, typename... A>
auto futurize_apply(F&& func, A&&... args) {
    using futurator = futurize<std::result_of_t<F(A&&...)>>;
    return futurator::apply(std::forward<F>(func), std::forward<A>(args)...);
}
} // xhb namespace