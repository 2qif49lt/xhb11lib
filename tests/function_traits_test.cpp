#ifdef XHBLIB_FUNC_TRAITS_UNIT_TEST

// g++ function_traits_test.cpp  -o test -std=c++1z -DXHBLIB_FUNC_TRAITS_UNIT_TEST -I../

#include "src/utility/function_traits.h"

#include <cassert>
#include <string>
#include <iostream>
#include <tuple>
#include <type_traits>
#include <functional> // ref

using namespace std;
using namespace xhb;

void func_ref(int a,string& b,int c){
    b += b;
    cout<<a<<" "<<b<<" "<<c<<endl;
}

void func(int a,string b,int c){
    b += b;
    cout<<a<<" "<<b<<" "<<c<<endl;
}

 string& func_ret_ref(int a,string& b){
    return b;
}


int main(){
   
    static_assert(std::is_same<typename func_traits<decltype(func)>::return_type, void>::value);
    static_assert(std::is_same<typename func_traits<decltype(func)>::type, void(int,string,int)>::value);
    static_assert(std::is_same<typename func_traits<decltype(func)>::arg<1>::type, string>::value);
    static_assert(std::is_same<typename func_traits<decltype(func)>::args_type<1>, string>::value);
    return 0;
}

#endif // XHBLIB_FUNC_TRAITS_UNIT_TEST
