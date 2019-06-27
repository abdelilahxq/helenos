/*
 * Copyright (c) 2019 Jaroslav Jindrak
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - The name of the author may not be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef LIBCPP_BITS_THREAD_FUTURE
#define LIBCPP_BITS_THREAD_FUTURE

#include <__bits/refcount_obj.hpp>
#include <__bits/thread/threading.hpp>
#include <cassert>
#include <memory>
#include <system_error>
#include <type_traits>
#include <utility>

namespace std
{
    /**
     * 30.6, futures:
     */

    enum class future_errc
    { // The 5001 start is to not collide with system_error's codes.
        broken_promise = 5001,
        future_already_retrieved,
        promise_already_satisfied,
        no_state
    };

    enum class launch
    {
        async,
        deferred
    };

    enum class future_status
    {
        ready,
        timeout,
        deferred
    };

    /**
     * 30.6.2, error handling:
     */

    template<>
    struct is_error_code_enum<future_errc>: true_type
    { /* DUMMY BODY */ };

    error_code make_error_code(future_errc) noexcept;
    error_condition make_error_condition(future_errc) noexcept;

    const error_category& future_category() noexcept;

    /**
     * 30.6.3, class future_error:
     */

    class future_error: public logic_error
    {
        public:
            future_error(error_code ec);

            const error_code& code() const noexcept;
            const char* what() const noexcept;

        private:
            error_code code_;
    };

    /**
     * 30.6.4, shared state:
     */

    namespace aux
    {
        template<class R>
        class shared_state: public aux::refcount_obj
        {
            public:
                const bool is_deferred_function;

                shared_state(bool is_deferred = false)
                    : is_deferred_function{is_deferred}, mutex_{},
                      condvar_{}, value_{}, value_set_{false},
                      exception_{}, has_exception_{false}
                {
                    threading::mutex::init(mutex_);
                    threading::condvar::init(condvar_);
                }

                void destroy() override
                {
                    if (this->refs() < 1)
                    {
                        // TODO: what to destroy? just this?
                    }
                }

                void set_value(const R& val, bool set)
                {
                    value_ = val;
                    value_set_ = set;
                }

                void set_value(R&& val, bool set)
                {
                    aux::threading::mutex::lock(mutex_);
                    value_ = std::move(val);
                    value_set_ = set;
                    aux::threading::mutex::unlock(mutex_);

                    aux::threading::condvar::broadcast(condvar_);
                }

                void set_set(bool set = true) noexcept
                {
                    value_set_ = set;
                }

                bool is_set() const noexcept
                {
                    return value_set_;
                }

                R& get()
                {
                    return value_;
                }

                void set_exception(exception_ptr ptr)
                {
                    exception_ = ptr;
                    has_exception_ = true;
                }

                bool has_exception() const noexcept
                {
                    return has_exception_;
                }

                void throw_stored_exception() const
                {
                    // TODO: implement
                }

                /**
                 * TODO: This member function is supposed to be marked
                 *       as 'const'. In such a case, however, we cannot
                 *       use the underlying fibril API because these
                 *       references get converted to pointers and the API
                 *       does not accept e.g. 'const fibril_condvar_t*'.
                 *
                 *       The same applies to the wait_for and wait_until
                 *       functions.
                 */
                void wait()
                {
                    aux::threading::mutex::lock(mutex_);
                    while (!value_set_)
                        aux::threading::condvar::wait(condvar_, mutex_);
                    aux::threading::mutex::unlock(mutex_);
                }

                template<class Rep, class Period>
                bool wait_for(const chrono::duration<Rep, Period>& rel_time)
                {
                    aux::threading::mutex::lock(mutex_);
                    aux::threading::condvar::wait_for(
                        condvar_, mutex_,
                        aux::threading::time::convert(rel_time)
                    );
                    aux::threading::mutex::unlock(mutex_);

                    return value_set_;
                }

                template<class Clock, class Duration>
                bool wait_until(const chrono::time_point<Clock, Duration>& abs_time)
                {
                    aux::threading::mutex::lock(mutex_);
                    aux::threading::condvar::wait_for(
                        condvar_, mutex_,
                        aux::threading::time::convert(abs_time - Clock::now())
                    );
                    aux::threading::mutex::unlock(mutex_);

                    return value_set_;
                }

                ~shared_state()
                {
                    // TODO: just destroy?
                }

            private:
                aux::mutex_t mutex_;
                aux::condvar_t condvar_;

                R value_;
                bool value_set_;

                exception_ptr exception_;
                bool has_exception_;
        };
    }

    template<class R>
    class future;

    template<class R>
    class promise
    {
        public:
            promise()
                : state_{new aux::shared_state<R>{}}
            { /* DUMMY BODY */ }

            template<class Allocator>
            promise(allocator_arg_t, const Allocator& a)
                : promise{}
            {
                // TODO: Use the allocator.
            }

            promise(promise&& rhs) noexcept
                : state_{}
            {
                state_ = rhs.state_;
                rhs.state_ = nullptr;
            }

            promise(const promise&) = delete;

            ~promise()
            {
                abandon_state_();
            }

            promise& operator=(promise&& rhs) noexcept
            {
                abandon_state_();
                promise{std::move(rhs)}.swap(*this);
            }

            promise& operator=(const promise&) = delete;

            void swap(promise& other) noexcept
            {
                std::swap(state_, other.state_);
            }

            future<R> get_future()
            {
                return future<R>{state_};
            }

            void set_value(const R& val)
            {
                if (!state_)
                    throw future_error{make_error_code(future_errc::no_state)};
                if (state_->is_set())
                {
                    throw future_error{
                        make_error_code(future_errc::promise_already_satisfied)
                    };
                }

                state_->set_value(val, true);
            }

            void set_value(R&& val)
            {
                if (!state_)
                    throw future_error{make_error_code(future_errc::no_state)};
                if (state_->is_set())
                {
                    throw future_error{
                        make_error_code(future_errc::promise_already_satisfied)
                    };
                }

                state_->set_value(std::forward<R>(val), true);
            }

            void set_exception(exception_ptr ptr)
            {
                assert(state_);

                state_->set_exception(ptr);
            }

            void set_value_at_thread_exit(const R& val)
            {
                if (!state_)
                    throw future_error{make_error_code(future_errc::no_state)};
                if (state_->is_set())
                {
                    throw future_error{
                        make_error_code(future_errc::promise_already_satisfied)
                    };
                }

                state_->set_value(val, false);
                // TODO: schedule it to be set as ready when thread exits
            }

            void set_value_at_thread_exit(R&& val)
            {
                if (!state_)
                    throw future_error{make_error_code(future_errc::no_state)};
                if (state_->is_set())
                {
                    throw future_error{
                        make_error_code(future_errc::promise_already_satisfied)
                    };
                }

                state_->set_value(std::forward<R>(val), false);
                // TODO: schedule it to be set as ready when thread exits
            }

            void set_exception_at_thread_exit(exception_ptr)
            {
                // TODO: No exception handling, no-op at this time.
            }

        private:
            void abandon_state_()
            {
                /**
                 * 1) If state is not ready:
                 *   a) Store exception of type future_error with
                 *      error condition broken_promise.
                 *   b) Mark state as ready.
                 * 2) Rekease the state.
                 */
            }

            aux::shared_state<R>* state_;
    };

    template<class R>
    class promise<R&>
    {
        // TODO: Copy & modify once promise is done.
    };

    template<>
    class promise<void>
    {
        // TODO: Copy & modify once promise is done.
    };

    template<class R>
    void swap(promise<R>& lhs, promise<R>& rhs) noexcept
    {
        lhs.swap(rhs);
    }

    template<class R, class Alloc>
    struct uses_allocator<promise<R>, Alloc>: true_type
    { /* DUMMY BODY */ };

    template<class R>
    class shared_future;

    template<class R>
    class future
    {
        public:
            future() noexcept
                : state_{nullptr}
            { /* DUMMY BODY */ }

            future(const future&) = delete;

            future(future&& rhs) noexcept
                : state_{std::move(rhs.state_)}
            {
                rhs.state_ = nullptr;
            }

            future(aux::shared_state<R>* state)
                : state_{state}
            {
                /**
                 * Note: This is a custom non-standard constructor that allows
                 *       us to create a future directly from a shared state. This
                 *       should never be a problem as aux::shared_state is a private
                 *       type and future has no constructor templates.
                 */
            }

            ~future()
            {
                release_state_();
            }

            future& operator=(const future) = delete;

            future& operator=(future&& rhs) noexcept
            {
                release_state_();
                state_ = std::move(rhs.state_);
                rhs.state_ = nullptr;
            }

            shared_future<R> share()
            {
                return shared_future<R>(std::move(*this));
            }

            R get()
            {
                assert(state_);

                wait();

                auto state = state_;
                state_ = nullptr;
                if (state->has_exception())
                    state->throw_stored_exception();

                return std::move(state->get());
            }

            bool valid() const noexcept
            {
                return state_ != nullptr;
            }

            void wait() const noexcept
            {
                assert(state_);

                state_->wait();
            }

            template<class Rep, class Period>
            future_status wait_for(const chrono::duration<Rep, Period>& rel_time) const
            {
                assert(state_);
                if (state_->is_deffered_function)
                    return future_status::deferred;

                auto res = state_->wait_for(rel_time);

                if (res)
                    return future_status::ready;
                else
                    return future_status::timeout;
            }

            template<class Clock, class Duration>
            future_status wait_until(const chrono::time_point<Clock, Duration>& abs_time) const
            {
                assert(state_);
                if (state_->is_deffered_function)
                    return future_status::deferred;

                auto res = state_->wait_until(abs_time);

                if (res)
                    return future_status::ready;
                else
                    return future_status::timeout;
            }

        private:
            void release_state_()
            {
                /**
                 * Last reference to state -> destroy state.
                 * Decrement refcount of state otherwise.
                 * Will not block, unless all following hold:
                 *  1) State was created by call to std::async.
                 *  2) State is not yet ready.
                 *  3) This was the last reference to the shared state.
                 */
            }

            aux::shared_state<R>* state_;
    };

    template<class R>
    class future<R&>
    {
        // TODO: Copy & modify once future is done.
    };

    template<>
    class future<void>
    {
        // TODO: Copy & modify once future is done.
    };

    // TODO: Make sure the move constructor of shared_future
    //       invalidates the state (i.e. sets to nullptr).
    template<class R>
    class shared_future
    {
        // TODO: Copy & modify once future is done.
    };

    template<class R>
    class shared_future<R&>
    {
        // TODO: Copy & modify once future is done.
    };

    template<>
    class shared_future<void>
    {
        // TODO: Copy & modify once future is done.
    };

    template<class>
    class packaged_task; // undefined

    template<class R, class... Args>
    class packaged_task<R(Args...)>
    {
        packaged_task() noexcept
        {}

        template<class F>
        explicit packaged_task(F&& f)
        {}

        template<class F, class Allocator>
        explicit packaged_task(allocator_arg_t, const Allocator& a, F&& f)
        {}

        ~packaged_task()
        {}

        packaged_task(const packaged_task&) = delete;
        packaged_task& operator=(const packaged_task&) = delete;

        packaged_task(packaged_task&& rhs)
        {}

        packaged_task& operator=(packaged_task&& rhs)
        {}

        void swap(packaged_task& other) noexcept
        {}

        bool valid() const noexcept
        {}

        future<R> get_future()
        {}

        void operator()(Args...)
        {}

        void make_ready_at_thread_exit(Args...)
        {}

        void reset()
        {}
    };

    template<class R, class... Args>
    void swap(packaged_task<R(Args...)>& lhs, packaged_task<R(Args...)>& rhs) noexcept
    {
        lhs.swap(rhs);
    };

    template<class R, class Alloc>
    struct uses_allocator<packaged_task<R>, Alloc>: true_type
    { /* DUMMY BODY */ };

    template<class F, class... Args>
    future<result_of_t<decay_t<F>(decay_t<Args>...)>>
    async(F&& f, Args&&... args)
    {
        // TODO: implement
        __unimplemented();
    }

    template<class F, class... Args>
    future<result_of_t<decay_t<F>(decay_t<Args>...)>>
    async(launch, F&& f, Args&&... args)
    {
        // TODO: implement
        __unimplemented();
    }
}

#endif
