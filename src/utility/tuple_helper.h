#ifndef XHBLIB_TUPLE_HELPER_H_
#define XHBLIB_TUPLE_HELPER_H_

// tuple 方面的工具

#include <tuple>
#include <utility>
#include <iostream>
#include <type_traits>

/*
 example:
 
 #include "src/utility/tuple_helper.h"
 #include "src/utility/pretty_printer.h"
 
 int main(int argc, const char * argv[]) {
 
 
     auto t1 = make_tuple(1, 'a', "bcd");
     cout << t1 << endl;
     
     
     vector<int> vec(5);
     generate(vec.begin(), vec.end(), [](){return rand();});
     cout << vec << endl;
     decltype(auto) t2 = xhb::atot(vec, index_sequence<2,3,4>());
     cout << t2 << endl;
     
     auto t3 = xhb::tuple_pick(t1, std::index_sequence<0,2>());
     cout << t3 << endl;
     
     bool bfind = xhb::tuple_visit(t1, 2, [](auto& a) { cout << a << endl; } );
     cout << bfind << endl;
     
     size_t idx = 4;
     assert(xhb::tuple_visit(t1, idx, [](auto& a) { cout << a << endl; } ) == false);
     
     
     int count = 0;
     xhb::tuple_for_each(t1, [&count](auto& a) {
     if (is_same<decltype(a), int&>::value) a++;
     ++count;
     } );
     assert(count == 3 && std::get<0>(t1) == 2);
     
     auto t4 = xhb::make_tuple_transform(t1, [](auto a)->int {
     if (is_same<decltype(a), int>::value) return 1;
     return 3;
     } );
     cout << t4 << endl;
     return 0;
 }
 */

namespace xhb {
/*
// 选择数组某些项转变为tuple
vector<int> vec(5);
generate(vec.begin(),vec.end(),[](){int r = rand(); cout << r << endl; return r;});
decltype(auto) t = atot(vec,index_sequence<2,3,4>());
*/
template<typename A, size_t... I>
auto atot(const A& a, std::index_sequence<I...> ) {
	return std::make_tuple(a[I]...);
}


// 由提供的tuple和sequence选择对应的元素生成一个新的tuple
template<typename... Ts, size_t... I>
auto tuple_pick(const std::tuple<Ts...>& tup, std::index_sequence<I...>&&) {
    return std::make_tuple(std::get<I>(tup)...);
}


template<size_t I>
struct tuple_visit_helper {
    template<typename T, typename F>
    static bool visit(T& tup, size_t i, F&& func) {
        if(i == I) {
             func(std::get<I>(tup));
			 return true;
        }else{
            return tuple_visit_helper<I - 1>::visit(tup,i,std::forward<F>(func));
        }
    }
};
template<>
struct tuple_visit_helper<0> {
    template<typename T, typename F>
    static bool visit(T& tup, size_t i, F&& func) {
		if (i != 0) return false;
        func(std::get<0>(tup));
		return true;
    }
};

// 通过变量访问tuple,如果存在对应的idx,则执行func并且返回true, 无则返回false
template<typename F, typename... Ts>
bool tuple_visit(const tuple<Ts...>& tup, size_t idx, F&& func) {
    return tuple_visit_helper<sizeof...(Ts) - 1>::visit(tup, idx, std::forward<F>(func));
}


/*
// 像for_each算法一样,tuple_for_each 迭代tuple,执行函数，参数为tuple元素。
*/
template<typename T, typename F, size_t... I>
void tuple_for_each_impl(T&& tup, F&& func, std::index_sequence<I...>&&) {
    auto unuse = { (func(std::get<I>(std::forward<T>(tup))), 0)... };
    (void)unuse;
}

template<typename F, typename... Ts>
void tuple_for_each(const std::tuple<Ts...>& tup, F&& func) {
    return tuple_for_each_impl(tup, std::forward<F>(func), std::index_sequence_for<Ts...>());
}
// 可修改
template<typename F, typename... Ts>
void tuple_for_each(std::tuple<Ts...>& tup, F&& func) {
    return tuple_for_each_impl(tup, std::forward<F>(func), std::index_sequence_for<Ts...>());
}
template<typename F, typename... Ts>
void tuple_for_each(std::tuple<Ts...>&& tup, F&& func) {
    return tuple_for_each_impl(std::move(tup), std::forward<F>(func), std::index_sequence_for<Ts...>());
}


template<typename T, typename F, size_t... I>
auto make_tuple_transform_impl(T&& tup, F&& func, std::index_sequence<I...>&&) {
    return std::make_tuple(func(std::get<I>(std::forward<T>(tup)))...);
}

// 由提供的tuple 和 auto func(auto) 函数 生成一个新的tuple
// 意义不大，func限制了返回值只能是一种。
template<typename F, typename... Ts>
auto make_tuple_transform(const std::tuple<Ts...>& tup, F&& func) {
    return make_tuple_transform_impl(tup, std::forward<F>(func), std::index_sequence_for<Ts...>());
}



} // xhb namespace
#endif // XHBLIB_TUPLE_HELPER_H_
