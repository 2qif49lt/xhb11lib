#include <algorithm>
#include <cassert>

#include <iostream>
#include <iomanip>
#include <vector>
#include <chrono>
namespace stdcho = std::chrono;

#include "test_utils.h"

static std::vector<xhb::test_base*> tests;

namespace xhb {

test_base::test_base() {
    tests.push_back(this);
}

} // xhb ns


int main (int argc, char** argv) {
    std::for_each(tests.begin(), tests.end(), [argc, argv](xhb::test_base* test) { 
       
        auto tbeg = stdcho::high_resolution_clock::now();
        int ret = test->run(argc, argv);
        auto tend = stdcho::high_resolution_clock::now();

        std::cout << std::left << std::setw(5) << ret << test->get_name() 
            << ":" << test->get_test_file() 
            << " cost(us): " << stdcho::duration_cast<stdcho::microseconds>(tend - tbeg).count()
            << std::endl;
        assert(ret == 0);
        });
}