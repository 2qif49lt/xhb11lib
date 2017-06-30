#ifndef XHBLIB_ACTOR_H_
#define XHBLIB_ACTOR_H_

#include <exception>
#include <memory>
#include <chrono>

#include "utility/optional.h"
#include "task.h"
#include "future.h"

namespace xhb {

void engine_schedule(std::unique_ptr<task> t);
void engine_schedule_urgent(std::unique_ptr<task> t);
void engine_exit(std::exception_ptr eptr);

} // xhb namespace

#endif // XHBLIB_ACTOR_H_