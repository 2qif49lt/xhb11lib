#ifndef XHBLIB_SHARED_FUTURE_H_
#define XHBLIB_SHARED_FUTURE_H_

#include "future.h"
#include "expiring_fifo.hh"

namespace xhb {

/// Changes the clock used by shared_future<> and shared_promise<> when passed as the first template parameter.
template<typename Clock>
struct with_clock {};

/// \cond internal

template <typename... T>
struct future_option_traits;

template <typename Clock, typename... T>
struct future_option_traits<with_clock<Clock>, T...> {
    using clock_type = Clock;

    template<template <typename...> class Class>
    struct parametrize {
        using type = Class<T...>;
    };
};

template <typename... T>
struct future_option_traits {
    using clock_type = lowres_clock;

    template<template <typename...> class Class>
    struct parametrize {
        using type = Class<T...>;
    };
};

/// \endcond

/// \brief Like \ref future except the result can be waited for by many fibers.
///
/// Represents a value which may not yet be ready. A fiber can wait for the value using
/// the \ref future obtained by calling \ref get_future() or casting to \ref future type.
/// Multiple fibers are allowed to obtain a \ref future for the result using the same
/// instance of \ref shared_future.
///
/// All futures obtained from shared_future should end up in the same state. However,
/// if the value's copy constructor throws, some of the futures may end up in a failed state
/// with an exception thrown from the copy constructor and end up with a state
/// different than other futures.
///
/// The scope of shared_future instance doesn't have to include scopes of the futures
/// obtained from that instance. In that sense the returned futures are independent.
///
/// shared_future can be copied at any time and all copies will resolve with the same value.
///
/// shared_future can be in a disengaged state when it's default-constructed or moved-from.
/// When it's in such a state we say it's invalid and obtaining futures must not be attempted.
///
/// The types in the parameter pack T must all be copy-constructible.
///
/// When the first type in the parameter pack is \ref with_clock then it has the effect
/// of changing the clock used for timeouts by this instance. This type is omitted from
/// the parameter of the future<> objects.
///
/// Example:
///
///    future<int> f;
///    shared_future<with_clock<manual_clock>, int> sf(std::move(f));
///    future<int> f2 = sf;
///
template<typename... T>
class shared_future {
    template <typename... U> friend class shared_promise;
    using options = future_option_traits<T...>;
public:
    using clock = typename options::clock_type;
    using time_point = typename clock::time_point;
    using future_type = typename future_option_traits<T...>::template parametrize<future>::type;
    using promise_type = typename future_option_traits<T...>::template parametrize<promise>::type;
    using value_tuple_type = typename future_option_traits<T...>::template parametrize<std::tuple>::type;
private:
    using future_state_type = typename future_option_traits<T...>::template parametrize<future_state>::type;
    using promise_expiry = typename future_option_traits<T...>::template parametrize<promise_expiry>::type;

    /// \cond internal
    class shared_state {
        future_state_type _future_state;
        expiring_fifo<promise_type, promise_expiry, clock> _peers;
    public:
        void resolve(future_type&& f) noexcept {
            _future_state = f.get_available_state();
            if (_future_state.failed()) {
                while (_peers) {
                    _peers.front().set_exception(_future_state.get_exception());
                    _peers.pop_front();
                }
            } else {
                while (_peers) {
                    auto& p = _peers.front();
                    try {
                        p.set_value(_future_state.get_value());
                    } catch (...) {
                        p.set_exception(std::current_exception());
                    }
                    _peers.pop_front();
                }
            }
        }

        future_type get_future(time_point timeout = time_point::max()) {
            if (!_future_state.available()) {
                promise_type p;
                auto f = p.get_future();
                _peers.push_back(std::move(p), timeout);
                return f;
            } else if (_future_state.failed()) {
                return future_type(exception_future_marker(), _future_state.get_exception());
            } else {
                try {
                    return future_type(ready_future_marker(), _future_state.get_value());
                } catch (...) {
                    return future_type(exception_future_marker(), std::current_exception());
                }
            }
        }
    };
    /// \endcond
    lw_shared_ptr<shared_state> _state;
public:
    /// \brief Forwards the result of future \c f into this shared_future.
    shared_future(future_type&& f)
        : _state(make_lw_shared<shared_state>())
    {
        f.then_wrapped([s = _state] (future_type&& f) mutable {
            s->resolve(std::move(f));
        });
    }

    shared_future() = default;
    shared_future(const shared_future&) = default;
    shared_future& operator=(const shared_future&) = default;
    shared_future(shared_future&&) = default;
    shared_future& operator=(shared_future&&) = default;

    /// \brief Creates a new \c future which will resolve with the result of this shared_future
    ///
    /// \param timeout When engaged, the returned future will resolve with \ref timed_out_error
    /// if this shared_future doesn't resolve before timeout is reached.
    ///
    /// This object must be in a valid state.
    future_type get_future(time_point timeout = time_point::max()) const {
        return _state->get_future(timeout);
    }

    /// \brief Equivalent to \ref get_future()
    operator future_type() const {
        return get_future();
    }

    /// \brief Returns true if the instance is in valid state
    bool valid() const {
        return bool(_state);
    }
};

/// \brief Like \ref promise except that its counterpart is \ref shared_future instead of \ref future
///
/// When the shared_promise is made ready, every waiter is also made ready.
///
/// Like the shared_future, the types in the parameter pack T must all be copy-constructible.
template <typename... T>
class shared_promise {
public:
    using shared_future_type = shared_future<T...>;
    using future_type = typename shared_future_type::future_type;
    using promise_type = typename shared_future_type::promise_type;
    using clock = typename shared_future_type::clock;
    using time_point = typename shared_future_type::time_point;
    using value_tuple_type = typename shared_future_type::value_tuple_type;
    using future_state_type = typename shared_future_type::future_state_type;
private:
    promise_type _promise;
    shared_future_type _shared_future;
    static constexpr bool copy_noexcept = future_state_type::copy_noexcept;
public:
    shared_promise(const shared_promise&) = delete;
    shared_promise(shared_promise&&) = default;
    shared_promise& operator=(shared_promise&&) = default;
    shared_promise() : _promise(), _shared_future(_promise.get_future()) {
    }

    /// \brief Gets new future associated with this promise.
    /// If the promise is not resolved before timeout the returned future will resolve with \ref timed_out_error.
    /// This instance doesn't have to be kept alive until the returned future resolves.
    future_type get_shared_future(time_point timeout = time_point::max()) {
        return _shared_future.get_future(timeout);
    }

    /// \brief Sets the shared_promise's value (as tuple; by copying), same as normal promise
    void set_value(const value_tuple_type& result) noexcept(copy_noexcept) {
        _promise.set_value(result);
    }

    /// \brief Sets the shared_promise's value (as tuple; by moving), same as normal promise
    void set_value(value_tuple_type&& result) noexcept {
        _promise.set_value(std::move(result));
    }

    /// \brief Sets the shared_promise's value (variadic), same as normal promise
    template <typename... A>
    void set_value(A&&... a) noexcept {
        _promise.set_value(std::forward<A>(a)...);
    }

    /// \brief Marks the shared_promise as failed, same as normal promise
    void set_exception(std::exception_ptr ex) noexcept {
        _promise.set_exception(std::move(ex));
    }

    /// \brief Marks the shared_promise as failed, same as normal promise
    template<typename Exception>
    void set_exception(Exception&& e) noexcept {
        set_exception(make_exception_ptr(std::forward<Exception>(e)));
    }
};

} // xhb namespace
#endif // XHBLIB_SHARED_FUTURE_H_