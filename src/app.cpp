#include <memory>
#include <iostream>
#include <exception>

#include "app.h"
#include "reactor.h"
#include "utility/resource.h"

namespace xhb {

int app::run(int ac, char ** av, std::function<future<int> ()>&& func) {
    return run_deprecated(ac, av, [func = std::move(func)] () mutable {
        auto func_done = make_shared<promise<>>();
        engine().at_exit([func_done] { return func_done->get_future(); });
        futurize_apply(func).finally([func_done] {
            func_done->set_value();
        }).then([] (int exit_code) {
            return engine().exit(exit_code);
        }).or_terminate();
    });
}

int app::run(int ac, char ** av, std::function<future<> ()>&& func) {
    return run(ac, av, [func = std::move(func)] {
        return func().then([] () {
            return 0;
        });
    });
}

bool init_resource_config(resource_config& rc) {
    return true;
}

int app::run_deprecated(int ac, char ** av, std::function<void ()>&& func) {
#ifdef DEBUG
    print("WARNING: debug mode. Not for benchmarking or production\n");
#endif
    resource_config rc;
    init_resource_config(rc);

    smp::configure(rc);

    engine().when_started().then(
        std::move(func)
    ).then_wrapped([] (auto&& f) {
        try {
            f.get();
        } catch (std::exception& ex) {
            std::cout << "program failed with uncaught exception: " << ex.what() << "\n";
            engine().exit(1);
        }
    });
    auto exit_code = engine().run();
    smp::cleanup();
    return exit_code;
}

} // xhb ns