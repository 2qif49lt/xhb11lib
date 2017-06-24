#include <iostream>
#include <string>

#include "test_utils.h"

TEST_CASE(attribute_specifier) {
    volatile int x = 0;
    [[likely(true)]] if (x == 0) {
        std::cout << "x == 0 " << __cplusplus << std::endl;
    }
    switch (x) {
        case 0:
            std::cout << "case 0" << std::endl;
            [[fallthrough]];
        case 1:
            std::cout << "case 1" << std::endl;
            break;
        default:
            break;
    }
    struct st_t {
        [[gnu::always_inline]] ~st_t() {}
    };


    return 0;
}