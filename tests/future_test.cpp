// g++ test_utils.cpp ../src/thread.cpp ../src/future.cpp ../src/reactor.cpp future_test.cpp -O2 -o main -std=c++14  -I../src -lpthread
#include <iostream>
#include <string>
#include <type_traits>
using namespace std;

#include "test_utils.h"

#include <thread.h>
#include <future.h>

#include <unistd.h>

TEST_CASE(threadt) {
    namespace xt = xhb::thread_impl;
    xt::init();
    xhb::thread([]{
        auto f = xhb::later();
        f.wait();
        cout << "wait ok" << endl;
    }).join();
    cout << "thread end" << endl;
    ::sleep(5);
    cout << "main end" << endl;
    ::sleep(5);
    return 0;
}
