#ifndef XHBLIB_APP_H_
#define XHBLIB_APP_H_

#include <functional>

#include "future.h"

namespace xhb {

class app {
public:
    app() = default;

    // Runs given function and terminates the application when the future it
    // returns resolves. The value with which the future resolves will be
    // returned by this function.
    int run(int ac, char ** av, std::function<future<int> ()>&& func);

    // Like run_sync() which takes std::function<future<int>()>, but returns
    // with exit code 0 when the future returned by func resolves
    // successfully.
    int run(int ac, char ** av, std::function<future<> ()>&& func);
private:
    int run_deprecated(int ac, char ** av, std::function<void ()>&& func);
};

} // xhb ns
#endif // XHBLIB_APP_H_