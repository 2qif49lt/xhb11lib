#ifdef SMART_PTR_HELPER_UNIT_TEST

#include <memory>
#include <iostream>
#include <cassert>

using namespace std;

#include "src/utility/smart_ptr_helper.h"

class test_unique_base {
public:
	virtual ~test_unique_base() = default;
	virtual void func() { cout << "test_unique_base" << endl; }
};

class test_unique_derived : public test_unique_base {
public:

	virtual ~test_unique_derived() = default;
	virtual void func() override { cout << "test_unique_derived" << endl; }
};

void test_unique_cast(unique_ptr<test_unique_derived>&& pd) {
	pd->func();
}

class deleter {
public:
	void operator()(test_unique_base* p) { delete p;}
};

int main(int argc, const char * argv[]) {
	unique_ptr<test_unique_base, deleter> pb(new test_unique_derived);
	pb->func();
	unique_ptr<test_unique_derived,deleter> pd = xhb::unique_ptr_cast<test_unique_derived>(move(pb));
	assert(!pb);
	pd->func();

	unique_ptr<test_unique_base> pb2 = make_unique<test_unique_derived>();
	unique_ptr<test_unique_derived> pd2 = xhb::unique_ptr_cast<test_unique_derived>(move(pb2));
	assert(!pb2);
	pd2->func();

	unique_ptr<test_unique_base, deleter> pb3(new test_unique_derived);
	pb3->func();
	unique_ptr<test_unique_derived,deleter> pd3 = xhb::dynamic_unique_ptr_cast<test_unique_derived>(move(pb3));
	assert(!pb3);
	pd3->func();

	unique_ptr<test_unique_base> pb4 = make_unique<test_unique_derived>();
	unique_ptr<test_unique_derived> pd4 = xhb::dynamic_unique_ptr_cast<test_unique_derived>(move(pb4));
	assert(!pb4);
	pd4->func();
    return 0;
}

#endif // SMART_PTR_HELPER_UNIT_TEST
