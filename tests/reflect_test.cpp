// g++ test_utils.cpp  reflect_test.cpp -O2 -o main -std=c++14  -I../src
#include <iostream>
#include <string>
#include <vector>
#include <type_traits>
#include <memory>
#include <cassert>
using namespace std;

#include "test_utils.h"

#include <utility/reflect.h>
#include <utility/reflect_std.h>
#include <utility/reflect_primitives.h>



TEST_CASE(reflect_primitives_t) {
    int i = 10;
    double d = 3.1415;
    long l = 1000;
    char c = 'a';

    auto desc_int = xhb::get_primitive_descri<int>();
    auto desc_double = xhb::get_primitive_descri<double>();
    auto desc_long = xhb::get_primitive_descri<long>();
    auto desc_char = xhb::get_primitive_descri<char>();

    cout << desc_int->name() << " " << desc_int->size() << " " << desc_int->dump(&i) << endl;
    cout << desc_long->name() << " " << desc_long->size() << " " << desc_long->dump(&l) << endl;
    cout << desc_char->name() << " " << desc_char->size() << " " << desc_char->dump(&c) << endl;

    assert(desc_int->name() == "int" && desc_int->size() == sizeof(int) && 
        desc_int->dump(&i) == "int{10}");
    assert(desc_double->name() == "double" && desc_double->size() == sizeof(double) && 
        desc_double->dump(&d) == "double{3.1415}");
    assert(desc_long->name() == "long" && desc_long->size() == sizeof(long) && 
        desc_long->dump(&l) == "long{1000}");
    assert(desc_char->name() == "char" && desc_char->size() == sizeof(char) && 
        desc_char->dump(&c) == "char{'a'}");
    

    return 0;
}


TEST_CASE(reflect_std_t) {
    vector<int> vec{1,2,3,4,5,6};
    string s{"hello world!"};

    auto desc_vec = xhb::type_resolver<vector<int>>::get();
    auto desc_string = xhb::type_resolver<string>::get();

    
    return 0;
}

TEST_CASE(reflect_all_t) {
    int i = 10;
    double d = 3.1415;
    long l = 1000;
    char c = 'a';

    vector<int> vec{1,2,3,4,5,6};
    string s{"hello world!"};
    
    auto desc_int = xhb::type_resolver<int>::get();
    auto desc_double = xhb::type_resolver<double>::get();
    auto desc_long = xhb::type_resolver<long>::get();
    auto desc_char = xhb::type_resolver<char>::get();

    auto desc_vec = xhb::type_resolver<vector<int>>::get();
    auto desc_string = xhb::type_resolver<string>::get();


    cout << desc_int->name() << " " << desc_int->size() << " " << desc_int->dump(&i) << endl;
    cout << desc_long->name() << " " << desc_long->size() << " " << desc_long->dump(&l) << endl;
    cout << desc_char->name() << " " << desc_char->size() << " " << desc_char->dump(&c) << endl;
    cout << desc_vec->name() << " " << desc_vec->size() << " " << desc_vec->dump(&vec) << endl;
    cout << desc_string->name() << " " << desc_string->size() << " " << desc_string->dump(&s) << endl;
    

    assert(desc_int->name() == "int" && desc_int->size() == sizeof(int) && 
        desc_int->dump(&i) == "int{10}");
    assert(desc_double->name() == "double" && desc_double->size() == sizeof(double) && 
        desc_double->dump(&d) == "double{3.1415}");
    assert(desc_long->name() == "long" && desc_long->size() == sizeof(long) && 
        desc_long->dump(&l) == "long{1000}");
    assert(desc_char->name() == "char" && desc_char->size() == sizeof(char) && 
        desc_char->dump(&c) == "char{'a'}");
    

    return 0;
}

struct st {
    std::string key;
    int value;
    std::vector<st> children;

    XHB_REFLECT()     
}; 

XHB_REFLECT_STRUCT_BEGIN(st)
XHB_REFLECT_STRUCT_MEMBER(key)
XHB_REFLECT_STRUCT_MEMBER(value)
XHB_REFLECT_STRUCT_MEMBER(children)
XHB_REFLECT_STRUCT_END()


TEST_CASE(reflect_struct_t) {

    st s{"apple", 3, {{"banana", 7, {}}, {"cherry", 11, {}}}};
    auto desc_struct = xhb::type_resolver<st>::get();
    cout << desc_struct->name() << " " << desc_struct->size() << " " << desc_struct->dump(&s) << endl;
    
    return 0;
}


struct stp {
    std::string key;
    int value;
    std::unique_ptr<int> ptr;

    XHB_REFLECT()     
}; 

XHB_REFLECT_STRUCT_BEGIN(stp)
XHB_REFLECT_STRUCT_MEMBER(key)
XHB_REFLECT_STRUCT_MEMBER(value)
XHB_REFLECT_STRUCT_MEMBER(ptr)
XHB_REFLECT_STRUCT_END()


TEST_CASE(reflect_struct_ptr_t) {

    stp s{"apple", 3, make_unique<int>(10)};
    auto desc_struct = xhb::type_resolver<stp>::get();
    cout << desc_struct->name() << " " << desc_struct->size() << " " << desc_struct->dump(&s) << endl;
    
    return 0;
}

struct stall {
    std::string key;
    int value;
    std::unique_ptr<int> ptr;
    st s;
    XHB_REFLECT()     
}; 

XHB_REFLECT_STRUCT_BEGIN(stall)
XHB_REFLECT_STRUCT_MEMBER(key)
XHB_REFLECT_STRUCT_MEMBER(value)
XHB_REFLECT_STRUCT_MEMBER(ptr)
XHB_REFLECT_STRUCT_MEMBER(s)

XHB_REFLECT_STRUCT_END()

TEST_CASE(reflect_struct_all_t) {

    stall s{"apple", 3, make_unique<int>(10),{"apple", 3, {{"banana", 7, {}}, {"cherry", 11, {}}}}};
    auto desc_struct = xhb::type_resolver<stall>::get();
    cout << desc_struct->name() << " " << desc_struct->size() << " " << desc_struct->dump(&s) << endl;
    
    /*
    stall {
        key = std::string{"apple"}
        value = int{3}
        ptr = std::unique_ptr<int>{int{10}}
        s = st {
            key = std::string{"apple"}
            value = int{3}
            children = std::vector<st>{
                [0]st {
                    key = std::string{"banana"}
                    value = int{7}
                    children = std::vector<st>{}
                }
                [1]st {
                    key = std::string{"cherry"}
                    value = int{11}
                    children = std::vector<st>{}
                }
            }
        }
    }
    */
    return 0;
}
