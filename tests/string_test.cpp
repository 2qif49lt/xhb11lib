// g++ test_utils.cpp  string_test.cpp -O2 -o main -std=c++14  -I../src
#include <iostream>
#include <string>
#include <type_traits>
using namespace std;

#include "test_utils.h"

#include <utility/string.h>


TEST_CASE(stringt) {
    auto s = "hello world";
    if (xhb::split(s, ' ').size() != 2) {
        return 1;
    }
    s = " hello world ";
    if (xhb::split(s, ' ').size() != 2) {
        cout<<xhb::split(s, ' ').size()<<endl;
        return 1;
    }
    s = " hello world    ";
    if (xhb::split(s, ' ').size() != 2) {
        cout<<xhb::split(s, ' ').size()<<endl;
        return 1;
    }
    return 0;
}