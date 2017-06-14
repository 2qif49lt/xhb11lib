#ifndef XHBLIB_ACTOR_H_
#define XHBLIB_ACTOR_H_

#include <exception>
#include <memory>
#include <chrono>

#include "utility/optional.h"
#include "task.h"
#include "future.h"

namespace xhb {

// thread_local 与 __thread 在特殊情况下有细微区别
// https://stackoverflow.com/questions/13106049/what-is-the-performance-penalty-of-c11-thread-local-variables-in-gcc-4-8/13123870#13123870

void schedule(std::unique_ptr<task> t);
void schedule_urgent(std::unique_ptr<task> t);



} // xhb namespace

#endif // XHBLIB_ACTOR_H_