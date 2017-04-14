#ifdef XHBLIB_CLEANER_UNIT_TEST

#include "cleaner.h"


#include <cassert>
#include <cstdlib>
#include <type_traits>
using namespace std;

void* test_normal_function_global = malloc(10);
void test_normal_function(){
    free(test_normal_function_global);
}
void test_simple_cleaner(){
    {
        void* p = malloc(10);
        auto c = make_cleaner(p);
    }
    assert(assert_ss.str() == "f");
    assert_ss.str("");
    
    {
        char* p = static_cast<char*>(malloc(10));
        auto c = make_cleaner(p);
    }
    assert(assert_ss.str() == "f");
    assert_ss.str("");
    
    {
        void* p = malloc(10);
        auto c= make_cleaner([p]()mutable{free(p);});
    }
    assert(assert_ss.str() == "l");
    assert_ss.str("");
    
    {
        struct test_functor{
            void* ptr = nullptr;
            test_functor(void* p):ptr(p){}
            void operator()(){
                free(ptr);
            }
        };
        
        void* p = malloc(10);
        
        auto c = make_cleaner(test_functor(p));
    }
    assert(assert_ss.str() == "l");
    assert_ss.str("");
    
    {
        auto c = make_cleaner(test_normal_function);
    }
    assert(assert_ss.str() == "l");
    assert_ss.str("");
    
    {
        struct test_object{
            ~test_object(){}
        };
        test_object obj;
        auto c = make_cleaner(obj);
    }
    assert(assert_ss.str() == "o");
    assert_ss.str("");

}
void test_next_cleaner(){
    {
        void* p = malloc(10);
        auto f = make_cleaner(p);
        
        char* pc = static_cast<char*>(malloc(100));
        auto lf = make_cleaner(move(f),pc);
        
        struct test_object{
            ~test_object(){}
        };
        
        test_object obj;
        
        auto olf = make_cleaner(move(lf), move(obj));
        
        auto po = new test_object();
        auto lolf = make_cleaner(move(olf), [po]()mutable{delete po;});
        
        auto llolf = make_cleaner(move(lolf), [](){cout<<"do nothing"<<endl;});
        
        
    }
    assert(assert_ss.str() == "llolf");
    assert_ss.str("");
}


void test_share(){
    
    cout<< (char*)&__func__<<endl;
    int n = 0;
    {
        auto a = make_cleaner([&n](){++n;});
        {
            auto b = a.share();
        }
        auto c = a.share();
    }
    assert(n == 1 && assert_ss.str() == "l");
    assert_ss.str("");
    
    n = 0;
    void* p = malloc(10);
    {
        auto a = make_cleaner(p);
        auto b = a.share();
        {
            auto c = b.share();
        }
        assert(b.refs() == 2);
        assert(assert_ss.str() == "");
    }
    assert(assert_ss.str() == "f");
    assert_ss.str("");
}
int main(){
    test_simple_cleaner();
    test_next_cleaner();
    test_share();
}
#endif // XHBLIB_CLEANER_UNIT_TEST
