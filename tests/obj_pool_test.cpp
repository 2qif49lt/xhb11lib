// g++ test_utils.cpp  obj_pool_test.cpp -O2 -o main -std=c++14  -I../src -lpthread -DXHBLIB_OBJ_POOL_UNIT_TEST
#include <iostream>
#include <string>
#include <vector>
#include <type_traits>
#include <memory>
#include <cassert>
#include <functional>
#include <thread>
#include <chrono>
#include <random>


using namespace std;

#include "test_utils.h"

#include <utility/object_pool.h>

namespace xhbtest{

class objtest {
public:
    objtest() {
    };
    objtest(const string& str) : _str{str} { cout << "para ctor" << endl;}
    objtest(int i) {
        char buff[100];
        sprintf(buff, "%d",i);
        _str = buff;
        _i = i;
    }
    objtest(const char* pre, int i) {
        char buff[100];
        sprintf(buff, "%s %d", pre, i);
        _str = buff;
        _i = i;
    }
    ~objtest() {
    //   cout << "destory" << endl;
    }
    string _str = {"init"};   
    int _i = 0; 
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

    pool.put_raw(a);

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
    return 0;
    xhb::obj_pool<xhbtest::objtest> pool;
    vector<decltype(pool.get())> vec;

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
    for (int i = 0; i != 128; ++i) {
        vec.push_back(std::move(pool.get()));
    }
    
    return 0;
}


// 多生产
TEST_CASE(object_pool_c_t) {
    return 0;
    using namespace std::chrono;
    int cycle = 10000;
    while (cycle--) {
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dis(1, 100);

        constexpr uint32_t SIZE = (2 << 12);
        xhb::obj_pool<xhbtest::objtest, SIZE > pool;
        vector<xhbtest::objtest*> vec1;
        vector<xhbtest::objtest*> vec2;
        vector<xhbtest::objtest*> vec3;
        vector<xhbtest::objtest*> vec4;

        int value = 0;
        for (int i = 0; i != SIZE / 4; i++, value++) {
            vec1.push_back(pool.get_raw("t1", value));
        }
        for (int i = 0; i != SIZE / 4; i++, value++) {
            vec2.push_back(pool.get_raw("t2", value));
        }
        for (int i = 0; i != SIZE / 4; i++, value++) {
            vec3.push_back(pool.get_raw("t3", value));
        }
        for (int i = 0; i != (SIZE / 4) - 1; i++, value++) {
            vec4.push_back(pool.get_raw("t4", value));
        }
        thread t1([&](){
            for (auto p : vec1) {
                pool.put_raw(p);
                if (dis(gen) < 10) {
                    this_thread::sleep_for(milliseconds(1));
                }
            }
        });

        thread t2([&](){
            for (auto p : vec2) {
                pool.put_raw(p);
                if (dis(gen) < 10) {
                    this_thread::sleep_for(milliseconds(1));
                }
            }
        });

        thread t3([&](){
            for (auto p : vec3) {
                pool.put_raw(p);
                if (dis(gen) < 10) {
                    this_thread::sleep_for(milliseconds(1));
                }
            }
        });

        thread t4([&](){
            for (auto p : vec4) {
                pool.put_raw(p);
                if (dis(gen) < 10) {
                    this_thread::sleep_for(milliseconds(1));
                }
            }
        });
        t1.join();
        t2.join();
        t3.join();
        t4.join();
        
        vector<int> values(SIZE - 1, 0);
        vector<xhbtest::objtest*> dump_vec;
        pool.dump(dump_vec);

        for (auto p : dump_vec) {
            values[p->_i] = 1;
        }

        bool fail = false;
        for (auto v : values) {
            if (v == 0) {
                fail = true;
                break;
            }
        }
        assert(fail == false && pool.size() == SIZE - 1 && pool.free_items() == 0);
        cout << 10000 - cycle << " cycle test pass ,"  << " pool size: "<< pool.size() << " free items: " << pool.free_items() << endl;
    }
    return 0;
}

// 多消费者
TEST_CASE(object_pool_d_t) {
    return 0;
    using namespace std::chrono;
    int cycle = 10000;
    while (cycle--) {
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dis(1, 100);

        constexpr uint32_t SIZE = (2 << 12);
        xhb::obj_pool<xhbtest::objtest, SIZE > pool;
        vector<xhbtest::objtest*> vec1;
        vector<xhbtest::objtest*> vec2;
        vector<xhbtest::objtest*> vec3;
        vector<xhbtest::objtest*> vec4;

        int value = 0;
        for (int i = 0; i != SIZE / 4; i++, value++) {
            vec1.push_back(pool.get_raw("t1", value));
        }
        for (int i = 0; i != SIZE / 4; i++, value++) {
            vec2.push_back(pool.get_raw("t2", value));
        }
        for (int i = 0; i != SIZE / 4; i++, value++) {
            vec3.push_back(pool.get_raw("t3", value));
        }
        for (int i = 0; i != (SIZE / 4) - 1; i++, value++) {
            vec4.push_back(pool.get_raw("t4", value));
        }
        thread t1([&](){
            for (auto p : vec1) {
                pool.put_raw(p);
                if (dis(gen) < 10) {
                    this_thread::sleep_for(milliseconds(1));
                }
            }
        });

        thread t2([&](){
            for (auto p : vec2) {
                pool.put_raw(p);
                if (dis(gen) < 10) {
                    this_thread::sleep_for(milliseconds(1));
                }
            }
        });

        thread t3([&](){
            for (auto p : vec3) {
                pool.put_raw(p);
                if (dis(gen) < 10) {
                    this_thread::sleep_for(milliseconds(1));
                }
            }
        });

        thread t4([&](){
            for (auto p : vec4) {
                pool.put_raw(p);
                if (dis(gen) < 10) {
                    this_thread::sleep_for(milliseconds(1));
                }
            }
        });
        t1.join();
        t2.join();
        t3.join();
        t4.join();
        
        vector<int> values(SIZE - 1, 0);
        vector<xhbtest::objtest*> dump_vec;
        pool.dump(dump_vec);

        for (auto p : dump_vec) {
            values[p->_i] = 1;
        }

        bool fail = false;
        for (auto v : values) {
            if (v == 0) {
                fail = true;
                break;
            }
        }
        assert(fail == false && pool.size() == SIZE - 1 && pool.free_items() == 0);
        cout << 10000 - cycle << " cycle test pass ,"  << " pool size: "<< pool.size() << " free items: " << pool.free_items() << endl;

        vec1.clear();
        vec2.clear();
        vec3.clear();
        vec4.clear();

        // 消费。

        thread s1([&](){
            for (int i = 0; i != SIZE / 4; i++) {
                vec1.push_back(pool.get_raw());
                if (dis(gen) < 10) {
                    this_thread::sleep_for(milliseconds(1));
                }
            }
        });

        thread s2([&](){
            for (int i = 0; i != SIZE / 4; i++) {
                vec2.push_back(pool.get_raw());
                if (dis(gen) < 10) {
                    this_thread::sleep_for(milliseconds(1));
                }
            }
        });

        thread s3([&](){
            for (int i = 0; i != SIZE / 4; i++) {
                vec3.push_back(pool.get_raw());
                if (dis(gen) < 10) {
                    this_thread::sleep_for(milliseconds(1));
                }
            }
        });

        thread s4([&](){
            for (int i = 0; i != SIZE / 4 - 1; i++) {
                vec4.push_back(pool.get_raw());
                if (dis(gen) < 10) {
                    this_thread::sleep_for(milliseconds(1));
                }
            }
        });
        
        s1.join();
        s2.join();
        s3.join();
        s4.join();
        
        for (auto p : vec1) {
            values[p->_i] = 0;
        }
        for (auto p : vec2) {
            values[p->_i] = 0;
        }
        for (auto p : vec3) {
            values[p->_i] = 0;
        }
        for (auto p : vec4) {
            values[p->_i] = 0;
        }

        fail = false;
        for (auto v : values) {
            if (v == 1) {
                fail = true;
                break;
            }
        }
        assert(fail == false && pool.size() == 0 && pool.free_items() == (SIZE -1));
        cout << 10000 - cycle << " cycle test pass ,"  << " pool size: "<< pool.size() << " free items: " << pool.free_items() << endl;
    }
    return 0;
}

// 多生产消费者
TEST_CASE(object_pool_e_t) {
    using namespace std::chrono;
    int cycle = 10000;
    while (cycle--) {
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dis(1, 100);
        uniform_int_distribution<> dis_c(1, 10);

        constexpr uint32_t SIZE = (2 << 12);
        xhb::obj_pool<xhbtest::objtest, SIZE > pool;

        vector<decltype(pool.get())> vec0;
        for (int i = 0; i != SIZE - 1; ++i) {
            vec0.push_back(std::move(pool.get(i)));
        }
        vec0.clear();
        assert( pool.size() == SIZE - 1 && pool.free_items() == 0 && 
            pool._allocate_counter == SIZE -1 && pool._deallocate_counter == 0);
        cout << 10000 - cycle  << " pool size: "<< pool.size() << " free items: " << pool.free_items() << endl;

        
        thread t1([&](){
            vector<decltype(pool.get())>  vec;
            auto c = dis_c(gen);
            for (int i = 0; i != c; ++i) {
                vec.push_back(std::move(pool.get()));
                if (dis(gen) < 10) {
                    this_thread::sleep_for(milliseconds(1));
                }
            }
        });

        thread t2([&](){
            vector<decltype(pool.get())>  vec;
            auto c = dis_c(gen);
            for (int i = 0; i != c; ++i) {
                vec.push_back(std::move(pool.get()));
                if (dis(gen) < 10) {
                    this_thread::sleep_for(milliseconds(1));
                }
            }
        });

        thread t3([&](){
            vector<decltype(pool.get())>  vec;
            auto c = dis_c(gen);
            for (int i = 0; i != c * 1000; ++i) {
                vec.push_back(std::move(pool.get()));
                if (dis(gen) < 10) {
                    this_thread::sleep_for(milliseconds(1));
                }
            }
        });

        thread t4([&](){
            vector<decltype(pool.get())>  vec;
            auto c = dis_c(gen);
            for (int i = 0; i != c; ++i) {
                vec.push_back(std::move(pool.get()));
                if (dis(gen) < 10) {
                    this_thread::sleep_for(milliseconds(1));
                }
            }
        });

        t1.join();
        t2.join();
        t3.join();
        t4.join();
        
        cout << "_allocate_counter: " << pool._allocate_counter << " " << pool._deallocate_counter << endl;
        bool fail = false;

        cout << 10000 - cycle << " cycle test pass ,"  << " pool size: "<< pool.size() << " free items: " << pool.free_items() << endl;

        assert(fail == false && pool.size() == SIZE - 1 && pool.free_items() == 0);

    }
    return 0;
}

