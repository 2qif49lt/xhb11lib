// g++ test_utils.cpp ../src/resource.cpp ../src/resource_linux.cpp ../src/thread.cpp ../src/posix.cpp ../src/smp.cpp ../src/future.cpp ../src/reactor.cpp future_test.cpp -O2 -o main -std=c++14  -I../src -lpthread
#include <iostream>
#include <string>
#include <type_traits>
#include <functional>
using namespace std;

#include "test_utils.h"

#include <thread.h>
#include <reactor.h>
#include <future.h>

TEST_CASE(futuret) {
    namespace xt = xhb::thread_impl;

    xhb::engine_start(argc, argv, []{ 
        cout << "hello world" << endl; 
        auto ctx = xt::get();
        assert(ctx);
        });
    return 0;
}
