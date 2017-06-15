#pragma once

namespace xhb {

struct test_base {
    test_base ();
    virtual ~test_base() {}
    virtual const char* get_test_file() = 0;
    virtual const char* get_name() = 0;
    virtual int run(int argc, char** argv) = 0;
};

} // xhb ns


#define TEST_CASE(name) \
    struct name##_test : public xhb::test_base { \
        const char* get_test_file() override { return __FILE__; } \
        const char* get_name() override { return #name; } \
        int run (int argc, char** argv) override; \
    }; \
    static name##_test name##_test##_instance; \
    int name##_test::run(int argc, char** argv)

