#ifdef XHBLIB_LOGGER_UNIT_TEST

// g++ ../src/logger.cpp logger_test.cpp -O2 -o main -std=c++14 -DXHBLIB_LOGGER_UNIT_TEST -I../

#include "src/logger.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
using namespace std;


int main(){
    xhb::logger l;
    l.log(xhb::log_level::info, "fmt only");
    l.log(xhb::log_level::info,"hello,%!","world");
    l.log(xhb::log_level::info,"hello,% % % % %!","world",1,2,3,4);
    l.log(xhb::log_level::debug,"hello,%!","world");
    l.log(xhb::log_level::error,"hello,%!","world");

    ostringstream ss;
    xhb::safe_printf(ss, "hello %");

    cout<<ss.str()<<endl;
    
    l.error("%", "xhb");
    l.debug("%", 10);
    l.warn("%", 3.1415926);
    
    l.set_level(xhb::log_level::debug);
    
    l.debug("1234");
    
    xhb::logger l2;
    xhb::logger l3("worker");
    auto vec = xhb::log_registry::instance().get_names();
    for_each(vec.begin(), vec.end(), [](auto s){cout<< s<<endl;});
    
    xhb::log_registry::instance().set_levels(xhb::log_level::info);
    
    l2.info("xhb");
    l3.debug("do not appear");
    l3.error("appear");
    
}
#endif // XHBLIB_LOGGER_UNIT_TEST
