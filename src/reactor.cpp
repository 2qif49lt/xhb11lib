#include "reactor.h"


namespace xhb {


thread_local bool g_need_preempt;
thread_local reactor* local_engine

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


future<> later() {
    promise<> p;
    auto f = p.get_future();
    engine().force_poll();
    schedule(make_task([p = std::move(p)] () mutable {
        p.set_value();
    }));
    return f;
}


} // xhb namespace