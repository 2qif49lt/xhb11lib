#include <thread>

#include "reactor.h"
#include "future_util.h"

namespace xhb {

void schedule(std::unique_ptr<task> t) {
   // 协程执行,现在暂时用线程咯
   std::thread([t = std::move(t)] { t->run(); }).detach();
}

void schedule_urgent(std::unique_ptr<task> t) {
   // 协程执行,现在暂时用线程咯
   std::thread([t = std::move(t)] { t->run(); }).detach();
}

} // xhb namespace