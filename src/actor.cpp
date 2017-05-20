#include "actor.h"


namespace xhb {

__thread bool g_need_preempt;
__thread reactor* local_engine

void schedule(std::unique_ptr<task> t) {
    engine().add_task(std::move(t));
}

void schedule_urgent(std::unique_ptr<task> t) {
    engine().add_urgent_task(std::move(t));
}

void engine_exit(std::exception_ptr eptr) {
    if (!eptr) {
        // 正常退出
    }
    // 记录日志
    // 非正常退出。
}

} // xhb namespace