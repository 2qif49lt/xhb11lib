#ifndef XHBLIB_ACTOR_H_
#define XHBLIB_ACTOR_H_

#include <exception>
#include <memory>
#include <chrono>
#include <functional>

#include "utility/optional.h"
#include "resource.h"
#include "task.h"
// #include "future.h"
#include "rueue.h"
#include "smp.h"

namespace xhb {

extern void engine_schedule(std::unique_ptr<task> t);
extern void engine_schedule_urgent(std::unique_ptr<task> t);
extern int engine_start(int ac, char** av, std::function<void ()>&& func);
extern void engine_exit(std::exception_ptr eptr);


class reactor {
    unsigned int _id;
    unsigned int _max_task_backlog;
    bool _stopped;
    rueue<std::unique_ptr<task>> _pending_tasks;

    void run_tasks(rueue<std::unique_ptr<task>>& tasks);
public:
    explicit reactor(unsigned int id);
    reactor(const reactor&) = delete;
    ~reactor();
    void operator=(const reactor&) = delete;

    void configure(const resource_config& rc);
    int run();
    void stop();
    void add_task(std::unique_ptr<task>&& t) { _pending_tasks.push_back(std::move(t)); }
    void add_urgent_task(std::unique_ptr<task>&& t) { _pending_tasks.push_front(std::move(t)); }

};

extern thread_local reactor* local_engine;

inline reactor& engine() {
    return *local_engine;
}

} // xhb namespace

#endif // XHBLIB_ACTOR_H_