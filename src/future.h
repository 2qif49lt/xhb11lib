#ifndef XHBLIB_FUTURE_H_
#define XHBLIB_FUTURE_H_

#include <type_traits>
#include <tuple>
#include <exception>
#include <utility>
#include <cassert>

#include "utility/apply.h"
#include "utility/function_traits.h"
#include "task.h"
#include "thread_impl.h"

namespace xhb {

extern void engine_schedule(std::unique_ptr<task> t);
extern void engine_schedule_urgent(std::unique_ptr<task> t);
extern void engine_exit(std::exception_ptr eptr);

template <class... T>
class promise;

template <class... T>
class future;

template <typename... T, typename... A>
future<T...> make_ready_future(A&&... value);

template <typename... T, typename... A>
future<T...> make_ready_future(std::tuple<A...>&& tup);

template <typename... T>
future<T...> make_exception_future(std::exception_ptr value) noexcept;

template <typename... T, typename E /*exception*/>
future<T...> make_exception_future(E&& ex) noexcept;

template<typename... T>
class future_state {
    static_assert(std::is_nothrow_move_constructible<std::tuple<T...>>::value, "types need no-throw constructor");
   
    enum class state :unsigned char{
        invalid,
        future,
        result,
        exception,
    };
    union any {
        any() {}
        ~any() {}
        std::tuple<T...> value;
        std::exception_ptr expt;
    };
    state _state = state::future;
    any _any;

public:
    future_state() noexcept {}
    future_state(future_state&& other) noexcept : _state(other.state) {
        other._state = state::invalid;
        switch (_state) {
            case state::result:
                new (&_any.value) std::tuple<T...>(std::move(other._any.value));
                other._any.value.~tuple();
                break;
            case state::exception:
                new (&_any.expt) std::exception_ptr(std::move(other._any.expt));
                other._any.expt.~exception_ptr();
                break;
            default:
                break;
        }
    }
    ~future_state() noexcept {
        switch (_state) {
            case state::result:
                _any.value.~tuple();
                break;
            case state::exception:
                _any.expt.~exception_ptr();
                break;
            default:
                break;
        }
    }
    future_state& operator=(future_state&& other) noexcept {
        if (this != &other) {
            this->~future_state();
            new (this) future_state(std::move(other));
        }
        return *this;
    }
    using first_element_t = std::tuple_element_t<0, std::tuple<T...>>;
    static first_element_t get_first(std::tuple<T...>&& t) {
        return std::get<0>(t);
    }
    bool available() const noexcept { return _state == state::result || _state == state::exception; }
    bool failed() const noexcept { return _state == state::exception; }
    
    void set(const std::tuple<T...>& value) noexcept {
        assert(_state == state::future);
        new (&_any.value) std::tuple<T...>(value);
        _state = state::result; 
    }

    void set(std::tuple<T...>&& value) noexcept {
        assert(_state == state::future);
        new (&_any.value) std::tuple<T...>(std::move(value));
        _state = state::result; 
    }

    template<typename... V>
    void set(V&&... vals) noexcept {
        assert(_state == state::future);
        new (&_any.value) std::tuple<T...>(std::forward<V>(vals)...);
        _state = state::result;
    }

    void set_exception(std::exception_ptr expt) noexcept {
        assert(_state == state::future);
        new (&_any.expt) std::exception_ptr(std::move(expt)); 
        _state = state::exception;
    }
    
    // future().get_exception() 重载版本
    std::exception_ptr get_exception() && noexcept {
        assert(_state == state::exception);
        _state = state::invalid;
        auto ret = std::move(_any.expt);
        _any.expt.~exception_ptr();
        return ret;
    }
    std::exception_ptr get_exception() const& noexcept {
         assert(_state == state::exception);
         return _any.expt;
    }

    std::tuple<T...> get_value() && noexcept {
        assert(_state == state::result);
        return std::move(_any.value);
    }

    template<typename U = std::tuple<T...>>
    U get_value() const& {
        assert(_state == state::result);
        return _any.value;
    }

    std::tuple<T...> get() && {
        assert(_state != state::future);
        if (_state == state::exception) {
            _state = state::invalid;
            auto ex = std::move(_any.expt);
            _any.expt.~exception_ptr();
            std::rethrow_exception(std::move(ex));
        }
        return std::move(_any.value);
    }
    std::tuple<T...>get() const& {
        assert(_state != state::future);
        if (_state == state::exception) {
            std::rethrow_exception(_any.expt);
        }
        return _any.value;
    }
    void ignore() noexcept {
        assert(_state != state::future);
        this->~future_state();
        _state = state::invalid;
    }
    
    void forward_to(promise<T...>& pr) noexcept {
        assert(_state != state::future);
        if (_state == state::exception) {
            pr.set_urgent_exception(std::move(_any.expt));
            _any.expt.~exception_ptr();
        } else {
            pr.set_urgent_value(std::move(_any.value));
            _any.value.~tuple();
        }
        _state = state::invalid;
    }
};

// 没有值的特化，
template<>
class future_state<> {
    enum class state :unsigned char{
        invalid,
        future,
        result,
        exception,
    };
    state _state = state::future;
    std::exception_ptr _expt;
public:
    future_state() noexcept {}
    future_state(future_state&& other) noexcept : _state(other._state), _expt(std::move(other._expt)) {
        other._state = state::invalid;
    }
    ~future_state() noexcept {}
    future_state& operator=(future_state&& other) noexcept {
        if (this != &other) {
            this->~future_state();
            new (this) future_state(std::move(other)); // g++ 6.3 测试可以真正支持exception_ptr的移动语义
        }
        return *this;
    }
    using first_element_t = void;
    static first_element_t get_first(std::tuple<>&& t) {}

    bool available() const noexcept { return _state == state::result || _state == state::exception; }
    bool failed() const noexcept { return _state == state::exception; }
    void set() noexcept {
        assert(_state == state::future);
        _state = state::result;
    }
    void set(const std::tuple<>& val) noexcept {
        assert(_state == state::future);
        _state = state::result;
    }
    void set(std::tuple<>&& val) noexcept {
        assert(_state == state::future);
        _state = state::result;
    }
    void set_exception(std::exception_ptr e) noexcept {
        assert(_state == state::future);
        _expt = std::move(e);
    }
    std::exception_ptr get_exception() && noexcept{
        assert(_state == state::exception);
        return std::move(_expt);
    }
    std::exception_ptr get_exception() const& noexcept {
        assert(_state == state::exception);
        return _expt;
    }
    std::tuple<> get_value() const noexcept {
        assert(_state == state::result);
        return {};
    }
    std::tuple<> get() && {
        assert(_state != state::future);
        if (_state == state::exception) {
            std::rethrow_exception(std::move(_expt));
        }
        return {};
    }
    std::tuple<> get() const& {
        assert(_state != state::future);
        if (_state == state::exception) {
            std::rethrow_exception(_expt);
        }
        return {};
    }
    void ignore() {
         assert(_state != state::future);
         _state = state::invalid;
      //   _expt = std::exception_ptr();
    }

    void forward_to(promise<>& pr) noexcept;
    
};

template<typename F, typename... T>
struct continuation final : public task {
    future_state<T...> _state;
    F _func;

    continuation(F&& func, future_state<T...>&& st) : _state(std::move(st)), _func(std::move(func)) {}
    continuation(F&& func) : _func(std::move(func)) {}
    virtual void run() noexcept override {
        _func(std::move(_state));
    }

};

template<typename... T>
class promise {
    enum class urgent { no, yes };
    future<T...>* _future = nullptr;
    future_state<T...> _local_state; // 
    future_state<T...>* _state;
    std::unique_ptr<task> _task;
public:
    // 创建无future关联的promise
    promise() noexcept : _state(&_local_state) {}
    promise(promise&& other) noexcept : _future(other._future), _state(other._state), _task(std::move(other._task)) {
        if (other._state == &other._local_state) {
            _local_state = std::move(other._local_state);
            _state = &_local_state;
        }
        other._future = nullptr;
        other._state = nullptr;
        migrated();
    }
    promise(const promise&) = delete;
    ~promise() noexcept {
        abandoned();
    }
    promise& operator=(promise&& other) {
        if (this != &other) {
            this->~promise();
            new (this) promise(std::move(other));
        }
        return *this;
    }
    promise& operator=(const promise&) = delete;

    // 获取关联的future,只能调用一次。
    // 关联后无论双方是否被move,当promise执行set_value/set_exception,future状态会立即就绪
    // 如果future附带了continuation，则会被执行。
    future<T...> get_future() {
        assert(_future == nullptr && _state && !_task);
        return future<T...>(this);
    }

    // set_* 系列可以在get_future调用之前或之后调用。
    void set_value(const std::tuple<T...>& rst) {
        do_set_value<urgent::no>(rst);
    } 
    void set_value(std::tuple<T...>&& rst) {
        do_set_value<urgent::no>(std::move(rst));
    }

    template<typename... V>
    void set_value(V&&... vals) {
        assert(_state);
        _state->set(std::forward<V>(vals)...);
         make_ready<urgent::no>();
    }

    void set_exception(std::exception_ptr expt) {
        assert(_state);
        _state->set_exception(std::move(expt));
        make_ready<urgent::no>();
    }
    template<typename E>
    void set_exception(E&& e) {
        set_exception(std::make_exception_ptr(std::forward<E>(e)));
    }
private:
    // 调整对应的future内容
    void migrated();
    void abandoned() {
        if (_future) {
            assert(_state != nullptr);
            _future->_local_state = std::move(*_state);
            _future->_promise = nullptr;
        } else if (_state && _state->failed()) {
            // log and warn exception ignored.
        }
    }
    template<urgent U>
    void make_ready();
private:
    template<urgent U>
    void do_set_value(std::tuple<T...> rst) {
        assert(_state);
        _state->set(std::move(rst));
        make_ready<U>();
    }
    template<urgent U>
    void do_set_exception(std::exception_ptr expt) {
        assert(_state);
        _state->set_exception(std::move(expt));
        make_ready<U>();
    }
    void set_urgent_value(const std::tuple<T...>& rst) {
        do_set_value<urgent::yes>(rst);
    }
    void set_urgent_value(std::tuple<T...>&& rst) {
        do_set_value<urgent::yes>(std::move(rst));
    }
    void set_urgent_exception(std::exception_ptr expt) {
        do_set_exception<urgent::yes>(std::move(expt));
    }
private:
    template<typename F>
    void schedule(F&& func) {
        auto p = std::make_unique<continuation<F, T...>>(std::move(func));
        _state = &p->_state;
        _task = std::move(p);
    }
    
    friend class future_state<T...>;
    
    template<typename... U>
    friend class future;
};
template<typename... T>
void promise<T...>::migrated() {
    if (_future) {
        _future->_promise = this;
    }
}
template<typename...T>
template<typename promise<T...>::urgent U>
void promise<T...>::make_ready() {
    if (_task) {
        _state = nullptr;
        if (U == urgent::yes) {
            engine_schedule_urgent(std::move(_task));
        } else {
            engine_schedule(std::move(_task));
        }
    }
}

template<>
class promise<void> : public promise<> {};


// 判断是否future类型
template<typename... T>
struct is_future : std::false_type {};
template<typename... T>
struct is_future<future<T...>> : std::true_type {};

template<typename... T>
constexpr bool is_future_v = is_future<T...>::value;

// tag
struct ready_future_marker {};
struct ready_future_from_tuple_marker {};
struct exception_future_marker {};


// 将类型转换为future
template<typename T>
struct futurize;

template<typename T>
struct futurize {
    using type = future<T>;
    using promise_type = promise<T>;
    using value_type = std::tuple<T>;

    template<typename E>
    static type make_exception_future(E&& ex);

    // 将函数返回值作为一个future返回。
    template<typename F, typename... As>
    static inline type apply(F&& func, std::tuple<As...>&& args) noexcept;
    template<typename F, typename... As>
    static inline type apply(F&& func, As&&... args) noexcept;

    // 将值转换为future.
    static inline type convert(T&& value) { return make_ready_future<T>(std::move(value)); }
    static inline type convert(type&& value) { return std::move(value); }

    static inline type from_tuple(value_type&& value);
    static inline type from_tuple(const value_type& value);
};

template<>
struct futurize<void> {
    using type = future<>;
    using promise_type = promise<>;
    using value_type = std::tuple<>;

    template<typename E>
    static type make_exception_future(E&& ex);

    template<typename F, typename... As>
    static inline type apply(F&& func, std::tuple<As...>&& args) noexcept;
    template<typename F, typename... As>
    static inline type apply(F&& func, As&&... args) noexcept;

    static inline type from_tuple(value_type&& value);
    static inline type from_tuple(const value_type& value);
};

template<typename... T>
struct futurize<future<T...>> {
    using type = future<T...>;
    using promise_type = promise<T...>;

    template<typename E>
    static type make_exception_future(E&& ex);

    template<typename F, typename... As>
    static inline type apply(F&& func, As&&... args) noexcept;
    template<typename F, typename... As>
    static inline type apply(F&& func, std::tuple<As...>&& tup) noexcept;

    static inline type convert(T&&... values) {
        return make_ready_future<T...>(std::forward<T>(values)...);
    }
    static inline type convert(type&& value) { return std::move(value); }
};

// 
template<typename T>
using futurize_t = typename futurize<T>::type;



template<typename... T>
class future {
    promise<T...>* _promise;
    future_state<T...> _local_state; // 当_promise无效时有效

private:
    future(promise<T...>* pr) noexcept : _promise(pr) {
        _promise->_future = this;
    }
    template<typename... As>
    future(ready_future_marker, As&&... args) : _promise(nullptr) {
        _local_state.set(std::forward<As>(args)...);
    }
    template<typename... As>
    future(ready_future_from_tuple_marker, std::tuple<As...>&& data) : _promise(nullptr) {
        _local_state.set(std::move(data));
    }
    future(exception_future_marker, std::exception_ptr ex) noexcept : _promise(nullptr) {
        _local_state.set_exception(std::move(ex));
    }
    explicit future(future_state<T...>&& state) : _promise(nullptr), _local_state(std::move(state)) {}
    
    future_state<T...>* state() {
        return _promise ? _promise->_state : &_local_state;
    }

    template<typename F>
    void schedule(F&& func) {
        if (state()->available()) {
            engine_schedule(std::make_unique<continuation<F, T...>>(std::forward<F>(func), std::move(*state())));
        } else {
            _promise->schedule(std::forward<F>(func));
            _promise->_future = nullptr;
            _promise = nullptr;
        }
    }

    future_state<T...> get_available_state() {
        auto st = state();
        if (_promise) {
            _promise->_future = nullptr;
            _promise = nullptr;
        }
        return std::move(*st);
    }

    future<T...> rethrow_with_nested() {
        if (!failed()) {
            // 如果成功 则返回个空异常future
            return make_exception_future<T...>(std::current_exception()); // ?
        } else {
            // 如果失败则抛出get的异常
            std::nested_exception f_ex;
            try {
                get();
            } catch (...) {
                std::throw_with_nested(f_ex);
            }
        }
    }

    template<typename... U>
    friend class shared_future;
public:
    using value_type = std::tuple<T...>;
    using promise_type = promise<T...>;

    future(future&& other) noexcept : _promise(other._promise) {
        if (!_promise) {
            _local_state = std::move(other._local_state);
        }
        other._promise = nullptr;
        if (_promise) {
            _promise->_future = this;
        }
    }
    future(const future&) = delete;
    future& operator=(future&& other) noexcept {
        if (this != &other) {
            this->~future();
            new (this) future(std::move(other));
        }
        return *this;
    }
    future& operator=(const future&) = delete;
    ~future() {
        if (_promise) {
            _promise->_future = nullptr;
        }
        #ifdef DEBUG
        if (failed()) {
            // log fail information
        }
        #endif
    }
public:
    bool available() {
        return state()->available();
    }
    bool failed() {
        return state()->failed();
    }

    std::tuple<T...> get() {
        if (!state()->available()) {
            wait();
        } 
        return get_available_state().get();
    }
    std::exception_ptr get_exception() {
        return get_available_state().get_exception();
    }

    typename future_state<T...>::first_element_t get_first() {
        return future_state<T...>::get_first(get());
    }

    void wait() {
        auto thd = thread_impl::get();
        assert(thd);
        schedule([this, thd] (future_state<T...>&& new_state) {
            *state() = std::move(new_state);
            thread_impl::switch_in(thd);
        });
        thread_impl::switch_out(thd);
    }

    // pr 未关联的promise
    void forward_to(promise<T...>&& pr) {
        if (state()->available()) {
            state()->forward_to(pr);
        } else {
            _promise->_future = nullptr;
            *_promise = std::move(pr);
            _promise = nullptr;
        }
    }
    // then，安排代码在当前future完成后接着执行
    // 如果当前future出现异常，则直接返回当前异常
    template<typename F, typename R = futurize_t<std::result_of_t<F(T&&...)>>>
    R then(F&& func) noexcept {
        using futurator = futurize<std::result_of_t<F(T&&...)>>;
        if (available()) {
            if (failed()) {
                return futurator::make_exception_future(get_available_state().get_exception());
            } else {
                return futurator::apply(std::forward<F>(func), get_available_state().get_value());
            }
        }
        typename futurator::promise_type pr;
        auto fut = pr.get_future();
        try {
            schedule([pr = std::move(pr), func = std::forward<F>(func)] (auto&& state) mutable {
                if (state.failed()) {
                    pr.set_exception(std::move(state.get_exception()));
                } else {
                    futurator::apply(std::forward<F>(func), std::move(state).get_value()).forward_to(std::move(pr));
                }
            });
        } catch (...) {
            abort();
        }
        return fut;
    }

    // then_warpped 
    // 不论当前future的结果是否是异常，继续执行F
    template<typename F, typename R = futurize_t<std::result_of_t<F(future)>>>
    R then_warpped(F&& func) noexcept {
        using futurator = futurize<std::result_of_t<F(future)>>;
        if (available()) {
            return futurator::apply(std::forward<F>(func), future(get_available_state()));
        }

        typename futurator::promise_type pr;
        auto fut = pr.get_future();
        try {
            schedule([pr = std::move(pr), func = std::forward<F>(func)] (auto&& state) mutable {
                futurator::apply(std::forward<F>(func), future(std::move(state))).forward_to(std::move(pr));
            } );
        } catch (...) {
            abort();
        }
        return fut;
    }
    

    template<typename F>
    future<T...> finally(F&& func) noexcept {
        return then_warpped(finally_body<F, is_future_v<std::result_of_t<F()>>>(std::forward<F>(func)));
    }

    template <typename F, bool BReturnFuture>
    struct finally_body;

    template <typename F>
    struct finally_body<F, true> {
        F _func;
        finally_body(F&& func) : _func(std::forward<F>(func)) {}
        future<T...> operator()(future<T...>&& rst) {
            using futurator = futurize<std::result_of_t<F()>>;
            return futurator::apply(_func).then_warpped([rst = std::move(rst)](auto state) mutable {
                if (!state.failed()) {
                    return std::move(rst);
                } else {
                    try {
                        state.get();
                    } catch (...) {
                        return rst.rethrow_with_nested();
                    }
                }
            });
        }
    };
    
    template <typename F>
    struct finally_body<F, false> {
        F _func;
        finally_body(F&& func) : _func(std::forward<F>(func)) {}
        future<T...> operator()(future<T...>&& rst) {
            try {
                _func();
                return std::move(rst);
            } catch (...) {
                return rst.rethrow_with_nested();
            }
        }
    };

    // 当前future fail时终止程序
    future<> or_terminate() noexcept {
        return then_wrapped([] (auto&& f) {
            try {
                f.get();
            } catch (...) {
                engine_exit(std::current_exception());
            }
        });
    }
    // 放弃当前future的值,如果是异常则继续扩散。
    future<> discard_result() noexcept {
        return then([] (T&&...) {} );
    }
    // 如果当前future携带exception,将调用func,参数为当前future的异常exception_ptr
    // func可以返回T、tuple<T...>、future、甚至异常
    template <typename F>
    future<T...> handle_exception(F&& func) noexcept {
        using func_ret_t = std::result_of_t<F(std::exception_ptr)>;
        return then_wrapped([func = std::forward<F>(func)] (auto&& fut) -> future<T...> {
            if (fut.failed()) {
                return make_ready_future<T...>(fut.get());
            } else {
                return futurize<func_ret_t>::apply(func, fut.get_exception());
            }
        });
    }

    template <typename F>
    future<T...> handle_exception_type(F&& func) noexcept {
        using trait = func_traits<F>;
        static_assert(trait::arity == 1, "func can take only one parameter");
        using ex_type = typename trait::template arg_t<0>;
        using ret_type = typename trait::return_type;
        return then_wrapped([func = std::forward<F>(func)] (auto&& fut) -> future<T...> {
            try {
                return make_ready_future<T...>(fut.get());
            } catch (ex_type& ex) {
                return futurize<ret_type>::apply(func, ex);
            }
        });
    }

    void ignore_ready_future() noexcept {
        state()->ignore();
    }

    template<typename... U>
    friend class promise;

    template<typename... U, typename... A>
    friend future<U...> make_ready_future(A&&... value);

    template<typename... U, typename... A>
    friend future<U...> make_ready_future(std::tuple<A...>&& tup);

    template<typename... U>
    friend future<U...> make_exception_future(std::exception_ptr expt) noexcept;

    template<typename...U, typename E>
    friend future<U...> make_exception_future(E&& ex) noexcept;
};

inline 
void future_state<>::forward_to(promise<>& pr) noexcept {
    assert(_state != state::future && _state != state::invalid);
    if (_state == state::exception) {
            pr.set_urgent_exception(std::move(_expt));
    } else {
        pr.set_urgent_value(std::tuple<>());
    }
    _state = state::invalid;
}

template <typename... T, typename... A>
inline
future<T...> make_ready_future(A&&... value) {
    return future<T...>(ready_future_marker(), std::forward<A>(value)...);
}

template <typename... T, typename... A>
inline
future<T...> make_ready_future(std::tuple<A...>&& tup) {
    return future<T...>(ready_future_from_tuple_marker(), std::forward<std::tuple<A...>>(tup));
}


template <typename... T>
inline
future<T...> make_exception_future(std::exception_ptr expt) noexcept {
    return future<T...>(exception_future_marker(), std::move(expt));
}

template <typename... T, typename E /*exception*/>
inline
future<T...> make_exception_future(E&& ex) noexcept {
    return make_exception_future<T...>(std::make_exception_ptr(std::forward<E>(ex)));
}

template <typename T>
inline
future<T>
futurize<T>::from_tuple(std::tuple<T>&& value) {
    return make_ready_future<T>(std::move(value));
}

template <typename T>
inline
future<T>
futurize<T>::from_tuple(const std::tuple<T>& value) {
    return make_ready_future<T>(value);
}

inline
future<>
futurize<void>::from_tuple(std::tuple<>&& value) {
    return make_ready_future<>();
}

inline
future<>
futurize<void>::from_tuple(const std::tuple<>& value) {
    return make_ready_future<>();
}

// futurie<void>

template<typename E>
inline
typename futurize<void>::type futurize<void>::make_exception_future(E&& ex) {
    return xhb::make_exception_future<>(std::forward<E>(ex));
}

template<typename F, typename... As>
inline
std::enable_if_t<is_future_v<std::result_of_t<F(As&&...)>>, future<>>
do_void_futurize_apply_args(F&& func, As&&... args) noexcept {
    try {
        return xhb::apply(func, std::forward<As>(args)...);
    } catch (...) {
        return make_exception_future(std::current_exception());
    }
}
template<typename F, typename... As>
inline
std::enable_if_t<!is_future_v<std::result_of_t<F(As&&...)>>, future<>>
do_void_futurize_apply_args(F&& func, As&&... args) noexcept {
    try {
        xhb::apply(func, std::forward<As>(args)...);
        return make_ready_future<>();
    } catch (...) {
        return make_exception_future(std::current_exception());
    }
}

template<typename F, typename... As>
inline
std::enable_if_t<is_future_v<std::result_of_t<F(As&&...)>>, future<>>
do_void_futurize_apply_tuple(F&& func, std::tuple<As...>&& tup) noexcept {
    try {
        return xhb::apply(std::forward<F>(func), std::move(tup));
    } catch (...) {
        return make_exception_future(std::current_exception());
    }
}
template<typename F, typename... As>
inline
std::enable_if_t<!is_future_v<std::result_of_t<F(As&&...)>>, future<>>
do_void_futurize_apply_tuple(F&& func, std::tuple<As...>&& tup) noexcept {
    try {
        xhb::apply(std::forward<F>(func), std::move(tup));
        return make_ready_future<>();
    } catch (...) {
        return make_exception_future(std::current_exception());
    }
}


template<typename F, typename... As>
inline
typename futurize<void>::type futurize<void>::apply(F&& func, std::tuple<As...>&& args) noexcept {
    return do_void_futurize_apply_tuple(std::forward<F>(func), std::move(args));
}
template<typename F, typename... As>
inline
typename futurize<void>::type futurize<void>::apply(F&& func, As&&... args) noexcept {
    return do_void_futurize_apply_args(std::forward<F>(func), std::forward<As>(args)...);
}


// futurize<T>

template<typename T>
template<typename E>
inline
typename futurize<T>::type futurize<T>::make_exception_future(E&& ex) {
    return xhb::make_exception_future<T>(std::forward<E>(ex));
}

template<typename T>
template<typename F, typename... As>
inline
typename futurize<T>::type futurize<T>::apply(F&& func, std::tuple<As...>&& args) noexcept {
    try {
        return convert(xhb::apply(std::forward<F>(func), std::move(args)));
    } catch (...) {
        return make_exception_future(std::current_exception()); 
    }
}
template<typename T>
template<typename F, typename... As>
inline
typename futurize<T>::type futurize<T>::apply(F&& func, As&&... args) noexcept {
    try {
        return convert(xhb::apply(std::forward<F>(func), std::forward<As>(args)...));
    } catch (...) {
        return make_exception_future(std::current_exception());
    }
}




// futurize<future<T...>>

template<typename... T>
template<typename E>
inline
future<T...> futurize<future<T...>>::make_exception_future(E&& ex) {
    return xhb::make_exception_future<T...>(std::forward<E>(ex));
}

template<typename... T>
template<typename F, typename... As>
inline
typename futurize<future<T...>>::type futurize<future<T...>>::apply(F&& func, As&&... args) noexcept{
    try {
        return xhb::apply(std::forward<F>(func), std::forward<As>(args)...);
    } catch (...) {
        return make_exception_future(std::current_exception());
    }
}
template<typename... T>
template<typename F, typename... As>
inline
typename futurize<future<T...>>::type futurize<future<T...>>::apply(F&& func, std::tuple<As...>&& tup) noexcept{
    try {
        return xhb::apply(std::forward<F>(func), std::move(tup));
    } catch (...) {
        return make_exception_future(std::current_exception());
    }
}


template<typename F, typename... A>
auto futurize_apply(F&& func, A&&... args) {
    using futurator = futurize<std::result_of_t<F(A&&...)>>;
    return futurator::apply(std::forward<F>(func), std::forward<A>(args)...);
}

} // xhb namespace

#endif // XHBLIB_FUTURE_H_