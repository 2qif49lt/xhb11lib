#ifndef XHBLIB_TASK_H_
#define XHBLIB_TASK_H_

#include <memory>
#include <utility>

namespace xhb{

class task {
public:
    virtual ~task() noexcept {}
    virtual void run() noexcept = 0;
};

template<typename F>
class lambda_task final : public task {
    F _func;
public:
    lambda_task(const F& f) : _func(f) {}
    lambda_task(F&& f) : _func(std::move(f)) {}
    virtual void run() noexcept override { _func(); }
};

/*
    auto f = xhb::make_task([]{ cout << "hi" << endl; });
	f->run();
*/
template<typename F>
inline
std::unique_ptr<task> make_task(F&& f) {
    return std::make_unique<lambda_task<F>>(std::forward<F>(f));
}

} // xhb namespace
#endif // XHBLIB_TASK_H_