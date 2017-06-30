#include <thread>
#include <iostream>
using namespace std;

#include "reactor.h"
// #include "future_util.h"

namespace xhb {

void engine_schedule(std::unique_ptr<task> t) {
    cout << "engine_schedule" << endl;
    // 协程执行,现在暂时用线程咯
    std::thread([t = std::move(t)] { t->run(); }).detach();
}

void engine_schedule_urgent(std::unique_ptr<task> t) {
    cout << "engine_schedule" << endl;
   // 协程执行,现在暂时用线程咯
    std::thread([t = std::move(t)] { t->run(); }).detach();
}

void engine_exit(std::exception_ptr eptr) {
}

} // xhb namespace