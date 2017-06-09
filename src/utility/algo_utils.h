#ifndef XHBLIB_UTILITY_ALGO_UTILS_H_
#define XHBLIB_UTILITY_ALGO_UTILS_H_

#include <type_traits>

namespace xhb {

template<typename F, typename... As>
typename std::enable_if<std::is_void<typename std::result_of<F(As...)>::type>::value,  bool>::type unary_ret_bool_helper(F&& func, As&&... args) {
	 func(std::forward<As>(args)...);
	 return true;
 }
template<typename F, typename... As>
typename std::enable_if<!std::is_void<typename std::result_of<F(As...)>::type>::value,  bool>::type unary_ret_bool_helper(F&& func, As&&... args) {
	static_assert(std::is_convertible<typename std::result_of<F(As...)>::type, bool>::value, "return type need be convertible to cast to bool");
	return func(std::forward<As>(args)...); 
}

template <typename F, typename... As>
void repeat_if(size_t n, F&& func, As&&... args) {
	for (size_t idx = 0; idx != n; ++idx) {
		if ( unary_ret_bool_helper(std::forward<F>(func), std::forward<As>(args)...) == false) 
			break;
	}
}


} // xhb namespace


/*
int main()
{	
	repeat_if(10, [](){ cout << "1" << endl; });
	repeat_if(10, [](){ 
		static int i = 0; 
		i++;
		cout << i << endl;  
		if (i > 5) 
			return false; 
		return true;
	});
	return 0;
}
*/
#endif // XHBLIB_UTILITY_ALGO_UTILS_H_