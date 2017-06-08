#ifndef XHBLIB_UTILITY_ALGO_UTILS_H_
#define XHBLIB_UTILITY_ALGO_UTILS_H_

#include <type_traits>

namespace xhb {

template<typename F>
typename std::enable_if<std::is_void<typename std::result_of<F()>::type>::value,  bool>::type unary_ret_bool_helper(F&& func) {
	 func();
	 return true;
 }
template<typename F>
typename std::enable_if<!std::is_void<typename std::result_of<F()>::type>::value,  bool>::type unary_ret_bool_helper(F&& func) {
	return func();
}

template <typename F>
void repeat_if(size_t n, F&& func) {
	for (size_t idx = 0; idx != n; ++idx) {
		if ( unary_ret_bool_helper(std::forward<F>(func)) == false) 
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