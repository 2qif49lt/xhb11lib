#include <iostream>
#include <string>
#include <type_traits>
using namespace std;

#include "test_utils.h"

TEST_CASE(unrestrict_union) {
    struct sta_t {
        int i;
        char c;
    };
    struct stb_t {
        std::string str;
    };

    cout << is_trivial<sta_t>::value << endl;
    cout << is_trivial<stb_t>::value << endl;

    union pod_t {
        int i;
        sta_t s;
    };
    
    union nonpod_t {
        int i;
        stb_t s;
        nonpod_t() { new(&s)stb_t{"h"}; }
        ~nonpod_t() { s.~stb_t(); }
    };
    nonpod_t nu;
    return 0;
}