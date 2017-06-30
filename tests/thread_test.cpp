// g++ test_utils.cpp ../src/thread.cpp  thread_test.cpp -O2 -o main -std=c++14  -I../src
#include <iostream>
#include <string>
#include <type_traits>
using namespace std;

#include "test_utils.h"

#include <thread.h>


TEST_CASE(threadt) {
    namespace xt = xhb::thread_impl;
    xt::init();
    
    xhb::thread([]{ 
        cout << "beg routine a" << endl;
        auto ctx = xt::get();
        assert(ctx);
        xhb::thread([ctx]{ 
            cout << "routine b" << endl;
         }).join();
        cout << "end routine a" << endl;
    }).join();
    cout << "end" << endl;
    return 0;
}
