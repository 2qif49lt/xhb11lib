#include <algorithm>
#include <iostream>
#include <iomanip>
#include <vector>

#include "test_utils.h"

static std::vector<xhb::test_base*> tests;

namespace xhb {

test_base::test_base() {
    tests.push_back(this);
}

} // xhb ns


int main (int argc, char** argv) {
    std::for_each(tests.begin(), tests.end(), [argc, argv](xhb::test_base* test) { 
        int ret = test->run(argc, argv);
        std::cout << std::left << std::setw(5) << ret << test->get_name() << ":" << test->get_test_file() << std::endl;
        });
}