#ifndef XHBLIB_TUPLE_HELPER_H_
#define XHBLIB_TUPLE_HELPER_H_

// tuple 方面的工具

#include <tuple>
#include <utility>

namespace xhb{

/*
// 选择某些项转变为tuple
vector<int> vec(5);
generate(vec.begin(),vec.end(),[](){int r = rand(); cout << r << endl; return r;});
decltype(auto) t = atot(vec,index_sequence<2,3,4>());
*/
template<typename A, size_t... I>
decltype(auto) atot(const A& a, std::index_sequence<I...> ) {
	return std::make_tuple(a.[I]...);
}


/*
// 像for_each算法一样,tuple_for_each 迭代tuple,执行函数，参数为tuple元素。
*/
template<typename T, typename F, size_t... I>
void tuple_for_each_impl(T&& tup, F&& func, std::index_sequence<I...>&&) {
    auto _avar = { (func(std::get<I>(std::forward<T>(tup))), 0)... };
    (void)_avar;
}

template<typename F, typename... Ts>
void tuple_for_each(const std::tuple<Ts...>& tup, F&& func) {
    return tuple_for_each_impl(tup, std::forward<F>(func), std::index_sequence_for<Ts...>())
}
// 可修改
template<typename F, typename... Ts>
void tuple_for_each(std::tuple<Ts...>& tup, F&& func) {
    return tuple_for_each_impl(tup, std::forward<F>(func), std::index_sequence_for<Ts...>())
}
template<typename F, typename... Ts>
void tuple_for_each(std::tuple<Ts...>&& tup, F&& func) {
    return tuple_for_each_impl(std::move(tup), std::forward<F>(func), std::index_sequence_for<Ts...>())
}

} // xhb namespace
#endif // XHBLIB_TUPLE_HELPER_H_