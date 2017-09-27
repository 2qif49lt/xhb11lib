// g++ test_utils.cpp  rand_fair_test.cpp -O2 -o main -std=c++14  -I../src
#include <iostream>
#include <algorithm>
using std::cout;
using std::endl;
#include "test_utils.h"

#include <utility/rand_fair.h>

#include <cstdlib>
#include <ctime>

TEST_CASE(saferandt) {
    std::srand(std::time(0));
    int arr[] = {2, 10, 100, 1000, 10000, 50000};
    for (auto&& v : arr) {
        v = xhb::rand(v);
    }
    for (auto&& v : arr) {
        cout << v << endl;
    }
    return 0;
}

TEST_CASE(noramlrandt) {
    std::srand(std::time(0));
    int arr[] = {2, 10, 100, 1000, 10000, 50000};
    for (auto&& v : arr) {
        v = std::rand() % v;
    }
    for (auto&& v : arr) {
        cout << v << endl;
    }
    return 0;
}
// 27.5s
TEST_CASE(saferandlongt) {
    std::srand(std::time(0));
    int unuse = 0;
    for (int i = 1; i != RAND_MAX - 1; i++) {
        unuse += xhb::rand(i);
    }

    return 0;
}

// 16.1s
TEST_CASE(noramlrandlongt) {
    std::srand(std::time(0));
    int unuse = 0;
    for (int i = 1; i != RAND_MAX - 1; i++) {
        unuse += std::rand() % i;
    }
    return 0;
}
