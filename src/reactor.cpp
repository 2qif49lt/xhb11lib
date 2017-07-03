#include <signal.h>

#include <iostream>
using namespace std;

#include "reactor.h"
// #include "future_util.h"
#include "thread.h"
#include "smp.h"

namespace xhb {

thread_local reactor* local_engine;

void engine_schedule(std::unique_ptr<task> t) {
    cout << "engine_schedule" << endl;
    engine().add_task(std::move(t));
}

void engine_schedule_urgent(std::unique_ptr<task> t) {
    cout << "add_urgent_task" << endl;
    engine().add_urgent_task(std::move(t));
}

int engine_start(int ac, char ** av, std::function<void ()>&& func) {
    cout << "engine_start" << endl;

    return smp::run_one(std::move(func));
}

void engine_exit(std::exception_ptr eptr) {
    engine().stop();
}

reactor::reactor(unsigned int id) : _id(id) {
    _max_task_backlog = 10;
    _stopped = false;
    cout << "reactor" << endl;
    thread_impl::init();
}
reactor::~reactor() {

}
void reactor::configure(const resource_config& rc) {
    //
}
void reactor::run_tasks(rueue<std::unique_ptr<task>>& tasks) {
    cout << "run_tasks" << endl;

    while (!tasks.empty()) {
        auto tsk = std::move(tasks.front());
        tasks.pop_front();
        tsk->run();
        tsk.reset();
        // check at end of loop, to allow at least one task to run
        if (tasks.size() <= _max_task_backlog) {
            break;
        }
    }
}
int reactor::run() {
    cout << "run" << endl;

    while (true) {
        run_tasks(_pending_tasks);
        if (_stopped) {
            while (!_pending_tasks.empty()) {
                run_tasks(_pending_tasks);
            }
            break;
        }
    }

    return 0;
}

void reactor::stop() {
    _stopped = true;
}




} // xhb namespace