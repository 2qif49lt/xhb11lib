// g++ test_utils.cpp  any_test.cpp -O2 -o main -std=c++14  -I../src
#include <iostream>
#include <string>
#include <vector>
#include <type_traits>
#include <cassert>
using namespace std;

#include "test_utils.h"

#include <utility/any.h>


TEST_CASE(any_int_t) {
    int o = 1234;
    auto a = xhb::any(o);
    auto& b = xhb::any_cast<int&>(a);
    b = 4321;
    auto c = xhb::any_cast<int>(a);
    if (o != 1234 || c != 4321 || c != b) {
        return 1;
    }
    
    return 0;
}
TEST_CASE(any_ptr_t) {
    char arr[] = {'a', 'b', 'c', 0};
    const char* o = reinterpret_cast<const char*>(&arr);

    auto a = xhb::any(o);
    auto b = xhb::any_cast<const char*>(a);
    b = "def";
    auto c = xhb::any_cast<const char*>(a);

    arr[0] = 'd';

    if (strcmp(b, c) == 0) {
        return 1;
    }

    return 0;
}
TEST_CASE(any_arr_t) {
    // 数组退化为指针
    int arr[] = {1, 2, 3};

    auto a = xhb::any(arr);
    auto b = xhb::any_cast<int*>(a);
    if (b != &arr[0]) {
        return 1;
    }

    return 0;
}

TEST_CASE(any_vec_t) {
    vector<int> vec = {1, 2, 3};

    auto a = xhb::any(vec);
    auto copy_vec = xhb::any_cast<vector<int>>(a);
    if (copy_vec.size() != 3) {
        return 1;
    }
    auto&& ref_vec = xhb::any_cast<vector<int>&>(a);
    ref_vec[0] = 10;
    if (copy_vec[0] == 10) {
        return 2;
    }
    return 0;
}

TEST_CASE(any_member_t) {
    vector<int> vec = {1, 2, 3};

    auto a = xhb::any(vec);
    auto b = xhb::any(a);
    
    auto& t1 = xhb::any_cast<vector<int>&>(a)[0];
    auto& t2 = xhb::any_cast<vector<int>&>(b)[0];
    assert(t1 == t2);

    t1 = 10;
    assert(xhb::any_cast<vector<int>>(a)[0] != xhb::any_cast<vector<int>&>(b)[0]);
    vec[0] = 1;
    assert(xhb::any_cast<vector<int>>(a)[0] != xhb::any_cast<vector<int>&>(b)[0]);


    auto c = xhb::any(string("abc"));
    assert(xhb::any_cast<string>(c) == "abc");
    xhb::any_cast<string&>(c)[0] = 'c';
    assert(xhb::any_cast<string>(c) == "cbc");

    auto d = xhb::any(xhb::in_place_type_t<string>(), 5, 'a');
    assert(xhb::any_cast<string>(d) == "aaaaa");
    xhb::any_cast<string&>(d)[0] = 'c';
    assert(xhb::any_cast<string>(d) == "caaaa");

    auto e = xhb::make_any<string>(5, 'b');
    assert(xhb::any_cast<string>(e) == "bbbbb");
    xhb::any_cast<string&>(e)[0] = 'c';
    assert(xhb::any_cast<string>(e) == "cbbbb");

    auto f = xhb::make_any<vector<int>>({1, 2, 3});
    assert(xhb::any_cast<vector<int>>(f)[0] == 1);
   
    a = c;
    assert(xhb::any_cast<string>(a) == "cbc");
    
    a = xhb::any(string("abc"));
    assert(xhb::any_cast<string>(a) == "abc");

    a.emplace<string>(3, 'a');
    assert(xhb::any_cast<string>(a) == "aaa");

    a.emplace<vector<int>>({1,2,3});
    assert(xhb::any_cast<vector<int>>(a)[0] == 1);
    
    a.swap(c);
    assert(xhb::any_cast<string>(a) == "cbc");
    
    xhb::swap(a, f);
    assert(xhb::any_cast<vector<int>>(a)[0] == 1);

    a.reset();
    assert(a.has_value() == false);

    return 0;
}