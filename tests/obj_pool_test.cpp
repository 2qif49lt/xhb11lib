// g++ test_utils.cpp  obj_pool_test.cpp -O2 -o main -std=c++14  -I../src
#include <iostream>
#include <string>
#include <vector>
#include <type_traits>
#include <memory>
#include <cassert>
#include <functional>

using namespace std;

#include "test_utils.h"

#include <utility/object_pool.h>

namespace xhbtest{

class objtest {
public:
    objtest() {
        cout << "default ctor" << endl;
    };
    objtest(const string& str) : _str{str} { cout << "para ctor" << endl;}
    objtest(int i) {
        char buff[100];
        sprintf(buff, "%d",i);
        _str = buff;
    }
    ~objtest() {
    //   cout << "destory" << endl;
    }
    string _str = {"init"};    
};

} // xhbtest ns
// 基本操作
TEST_CASE(object_pool_a_t) {
    return 0;
    xhb::obj_pool<xhbtest::objtest> pool;

    assert(pool.size() == 0 && pool.free_items() == 127);

    xhbtest::objtest* a = pool.get_raw();
    
    assert(pool.size() == 0 && pool.free_items() == 127 &&
        a->_str == "init");

    pool.put(a);

    assert(pool.size() == 1 && pool.free_items() == 126 &&
        a->_str == "init");

    {
        auto b = pool.get();
        assert(pool.size() == 0 && pool.free_items() == 127 &&
            b->_str == "init");
    }
   
    assert(pool.size() == 1 && pool.free_items() == 126 &&
        a->_str == "init");
     
    auto c = pool.get("change");
    assert(pool.size() == 0 && pool.free_items() == 127 &&
        c->_str == "change" && a->_str == "change");
    
    return 0;
}

// 超量
TEST_CASE(object_pool_b_t) {
    xhb::obj_pool<xhbtest::objtest> pool;
    vector<decltype(pool.get())> vec;
 //   using obj_uni_ptr = unique_ptr<xhbtest::objtest, std::function<void(xhbtest::objtest*)>>;
  //  vector<obj_uni_ptr> vec;

    for (int i = 0; i != 127; ++i) {
        vec.push_back(std::move(pool.get(i)));
    }
    for (auto&& p : vec ) {
        cout << p->_str << " ";
    }
    cout << endl;
    cout << vec.size() << " " << pool.size() << " " << pool.free_items() << endl;

    vec.clear();
    
     cout << vec.size() << " " << pool.size() << " " << pool.free_items() << endl;
    return 0;
}