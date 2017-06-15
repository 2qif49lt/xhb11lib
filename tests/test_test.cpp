/*
clang++ test_utils.cpp test_test.cpp -O2 -o test -std=c++14 -I../src
*/

#include <iostream>
#include <algorithm>
#include <random>
#include <map>

#include "utility/algos.h"
#include "test_utils.h"

TEST_CASE(test_example) {
    std::cout << "test case example" << std::endl;
    return 0;
}

TEST_CASE(test_random) {
    std::map<int, int> m;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 10);
 //   std::generate_n(std::back_inserter(vec), 1000, []{ return dis(gen); } );
    xhb::repeat_if(10000, [&] { m[dis(gen)]++;});
    std::for_each(m.begin(), m.end(), [](auto& it) { std::cout << it.first << ":" << it.second << std::endl;});
    return 0;
}

TEST_CASE(test_argv) {
    for (int i = 0; i != argc; i++) {
        std::cout << argv[i] << std::endl;
    }
    return 0;
}