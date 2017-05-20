#ifndef XHBLIB_ACTOR_H_
#define XHBLIB_ACTOR_H_

#include <exception>
#include <memory>

#include "task.h"

namespace xhb {

extern __thread reactor* local_engine;
extern __thread size_t task_quota;

inline reactor& engine() {
    return *local_engine;
}

inline bool engine_is_ready() {
    return local_engine != nullptr;
}

void schedule(std::unique_ptr<task> t);
void schedule_urgent(std::unique_ptr<task> t);



void engine_exit(std::exception_ptr eptr);

} // xhb namespace

#endif // XHBLIB_ACTOR_H_