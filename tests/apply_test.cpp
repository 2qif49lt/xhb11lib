#ifdef XHBLIB_APPLY_UNIT_TEST

#include "src/utility/apply.h"

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
    string s = "abc";
    apply(func_ref,make_tuple(1,std::ref(s),3));
    assert(s == "abcabc");
   
    
    s = "abc";
    apply(func, make_tuple(1,s,3));
    assert(s == "abc");
 
    decltype(auto) ret = apply(func_ret_ref, make_tuple(1,std::ref(s)));
    cout<<ret<<endl;
    assert(is_reference<decltype(ret)>::value == true);
 
    auto tup = make_tuple(1,"a",3);
    apply(func, move(tup));
    
    
    apply(func,const_cast<const decltype(tup) &>(tup));
    apply(func,move(tup));
    cout<<get<0>(tup)<<" "<<get<1>(tup)<<endl;
    
    
    s = "abc";
    decltype(auto) r = apply(func_ret_ref,1,s);
    assert(r == s);
    r = "def";
    assert(s == "def");
    
    return 0;
}

#endif // XHBLIB_APPLY_UNIT_TEST
