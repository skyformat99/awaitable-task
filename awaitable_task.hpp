//
// dsa is a utility library of data structures and algorithms built with C++11.
// This file (awaitable_task.hpp) is part of the dsa project.
//
// author: Dalton Woodard
// contact: daltonmwoodard@gmail.com
// repository: https://github.com/daltonwoodard/awaitable-task.git
// license:
//
// Copyright (c) 2016 DaltonWoodard. See the COPYRIGHT.md file at the top-level
// directory or at the listed source repository for details.
//
//      Licensed under the Apache License. Version 2.0:
//          https://www.apache.org/licenses/LICENSE-2.0
//      or the MIT License:
//          https://opensource.org/licenses/MIT
//      at the licensee's option. This file may not be copied, modified, or
//      distributed except according to those terms.
//

#ifndef DSA_AWAITABLE_TASK_HPP
#define DSA_AWAITABLE_TASK_HPP

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <exception>
#include <forward_list>
#include <future>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>
#include "utilities/functions.hpp"
#include "utilities/sequence.hpp"
#include "utilities/traits.hpp"


namespace dsa
{
    /*
     * awaitable_task; a type-erased, allocator-aware std::packaged_task that
     * also contains its own arguments. The underlying packaged_task and the
     * stored argument tuple can be heap allocated or allocated with a provided
     * allocator.
     *
     * There are two forms of awaitable_tasks: ready tasks and async tasks.
     *
     *      Ready tasks are assumed to be immediately invokable; that is,
     *      invoking the underlying pakcaged_task with the provided arguments
     *      will not block. This is contrasted with async tasks where some or
     *      all of the provided arguments may be futures waiting on results of
     *      other tasks.
     *
     *      Async tasks are assumed to take arguments where some or all are
     *      backed by futures waiting on results of other tasks. This is
     *      contrasted with ready tasks that are assumed to be immediately
     *      invokable.
     *
     * There are two helper methods for creating awaitable_task objects:
     * make_ready_task and make_awaitable_task, both of which return a pair of
     * the newly constructed awaitable_task and a std::future object to the
     * return value.
     */
    class awaitable_task
    {
        template <class T>
        using decay_if_future = typename std::conditional <
            utility::is_future <typename std::decay <T>::type>::value,
            typename utility::decay_future <T>::type, T
        >::type;

        struct ready_task_tag {};
        struct awaitable_task_tag {};

    public:
        awaitable_task (void) = default;
        ~awaitable_task (void) = default;

        awaitable_task (awaitable_task const &) = delete;
        awaitable_task (awaitable_task &&) noexcept = default;

        awaitable_task & operator= (awaitable_task const &) = delete;
        awaitable_task & operator= (awaitable_task &&) noexcept = default;

        void swap (awaitable_task & other) noexcept
        {
            std::swap (this->_t, other._t);
        }

        operator bool (void) const noexcept
        {
            return static_cast <bool> (this->_t);
        }

        template <class>
        friend class awaitable_task_system;

        template <class F, class ... Args>
        friend std::pair <
            awaitable_task,
            std::future <typename std::result_of <F (Args...)>::type>
        > make_ready_task (F && f, Args && ... args)
        {
            using pair_type = std::pair <
                awaitable_task,
                std::future <typename std::result_of <F (Args...)>::type>
            >;
            using model_type = ready_task_model <
                typename std::result_of <F (Args...)>::type (Args...)
            >;

            awaitable_task t (
                ready_task_tag (),
                std::forward <F> (f), std::forward <Args> (args)...
            );
            auto fut = dynamic_cast <model_type &> (*t._t).get_future ();
            return pair_type (std::move (t), std::move (fut));
        }

        template <class Allocator, class F, class ... Args>
        friend std::pair <
            awaitable_task,
            std::future <typename std::result_of <F (Args...)>::type>
        > make_ready_task (std::allocator_arg_t, Allocator const & alloc,
                           F && f, Args && ... args)
        {
            using pair_type = std::pair <
                awaitable_task,
            std::future <typename std::result_of <F (Args...)>::type>
            >;
            using model_type = ready_task_model <
                typename std::result_of <F (Args...)>::type (Args...)
            >;

            awaitable_task t (
                ready_task_tag (), std::allocator_arg_t (), alloc,
                std::forward <F> (f), std::forward <Args> (args)...
            );
            auto fut = dynamic_cast <model_type &> (*t._t).get_future ();
            return pair_type (std::move (t), std::move (fut));
        }

        template <class F, class ... Args>
        friend std::pair <
            awaitable_task,
            std::future <typename std::result_of <
                F (decay_if_future <Args>...)
            >::type>
        > make_awaitable_task (F && f, Args && ... args)
        {
            using pair_type = std::pair <
                awaitable_task,
                std::future <typename std::result_of <
                    F (decay_if_future <Args>...)
                >::type>
            >;
            using model_type = awaitable_task_model <
                typename std::result_of <
                    F (decay_if_future <Args>...)
                >::type (decay_if_future <Args>...)
            >;

            awaitable_task t (
                awaitable_task_tag (),
                std::forward <F> (f), std::forward <Args> (args)...
            );
            auto fut = dynamic_cast <model_type &> (*t._t).get_future ();
            return pair_type (std::move (t), std::move (fut));
        }

        template <class Allocator, class F, class ... Args>
        friend std::pair <
            awaitable_task,
            std::future <typename std::result_of <
                F (decay_if_future <Args>...)
            >::type>
        > make_awaitable_task (std::allocator_arg_t, Allocator const & alloc,
                           F && f, Args && ... args)
        {
            using pair_type = std::pair <
                awaitable_task,
                std::future <typename std::result_of <
                    F (decay_if_future <Args>...)
                >::type>
            >;
            using model_type = awaitable_task_model <
                typename std::result_of <
                    F (decay_if_future <Args>...)
                >::type (decay_if_future <Args>...)
            >;

            awaitable_task t (
                awaitable_task_tag (), std::allocator_arg_t (), alloc,
                std::forward <F> (f), std::forward <Args> (args)...
            );
            auto fut = dynamic_cast <model_type &> (*t._t).get_future ();
            return pair_type (std::move (t), std::move (fut));
        }

        void operator() (void)
        {
            if (this->_t)
                this->_t->invoke_ ();
            else
                throw std::logic_error ("bad task access");
        }

        bool ready (void) const noexcept
        {
            if (this->_t)
                return this->_t->ready_ ();
            else
                throw std::logic_error ("bad task access");
        }

    private:
        template <class F, class ... Args>
        awaitable_task (ready_task_tag, F && f, Args && ... args)
            : _t (
                new ready_task_model <
                    typename std::result_of <F (Args...)>::type (Args...)
                > (std::forward <F> (f), std::forward <Args> (args)...)
            )
        {}

        template <class Allocator, class F, class ... Args>
        awaitable_task (ready_task_tag,
              std::allocator_arg_t,
              Allocator const & alloc,
              F && f, Args && ... args)
            : _t (
                new ready_task_model <
                    typename std::result_of <F (Args...)>::type (Args...)
                > (std::allocator_arg_t (), alloc,
                   std::forward <F> (f), std::forward <Args> (args)...)
            )
        {
        }

        template <class F, class ... Args>
        awaitable_task (awaitable_task_tag, F && f, Args && ... args)
            : _t (
                new awaitable_task_model <
                    typename std::result_of <
                        F (decay_if_future <Args>...)
                    >::type (decay_if_future <Args>...),
                    Args...
                > (std::forward <F> (f), std::forward <Args> (args)...)
            )
        {}

        template <class Allocator, class F, class ... Args>
        awaitable_task (awaitable_task_tag,
              std::allocator_arg_t,
              Allocator const & alloc,
              F && f, Args && ... args)
            : _t (
                new awaitable_task_model <
                    typename std::result_of <
                        F (decay_if_future <Args>...)
                    >::type (decay_if_future <Args>...),
                    Args...
                > (std::allocator_arg_t (), alloc,
                   std::forward <F> (f), std::forward <Args> (args)...)
            )
        {}

        struct task_concept
        {
            virtual ~task_concept (void) noexcept {}
            virtual void invoke_ (void) = 0;
            virtual bool ready_ (void) const noexcept = 0;
        };

        template <class> struct ready_task_model;

        /*
         * Ready tasks are assumed to be immediately invokable; that is,
         * invoking the underlying pakcaged_task with the provided arguments
         * will not block. This is contrasted with async tasks where some or all
         * of the provided arguments may be futures waiting on results of other
         * tasks.
         */
        template <class R, class ... Args>
        struct ready_task_model <R (Args...)> : task_concept
        {
            template <class F>
            explicit ready_task_model (F && f, Args && ... args)
                : _f    (std::forward <F> (f))
                , _args (std::forward <Args> (args)...)
            {}

            template <class Allocator, class F>
            explicit ready_task_model (
                std::allocator_arg_t, Allocator const & alloc,
                F && f, Args && ... args
            )
                : _f    (std::allocator_arg_t (), alloc, std::forward <F> (f))
                , _args (std::allocator_arg_t (), alloc,
                         std::forward <Args> (args)...)
            {}

            std::future <R> get_future (void)
            {
                return this->_f.get_future ();
            }

            void invoke_ (void) override
            {
                utility::apply (this->_f, this->_args);
            }

            bool ready_ (void) const noexcept override
            {
                return true;
            }

        private:
            std::packaged_task <R (Args...)> _f;
            std::tuple <Args...> _args;
        };

        template <class ...> struct awaitable_task_model;

        /*
         * Async tasks are assumed to take arguments where some or all are
         * backed by futures waiting on results of other tasks. This is
         * contrasted with ready tasks that are assumed to be immediately
         * invokable.
         */
        template <class R, class ... CallArgs, class ... FutArgs>
        struct awaitable_task_model <R (CallArgs...), FutArgs...> : task_concept
        {
            template <class F, class ... Args>
            explicit awaitable_task_model (F && f, Args && ... args)
                : _f    (std::forward <F> (f))
                , _args (std::forward <Args> (args)...)
            {}

            template <class Allocator, class F, class ... Args>
            explicit awaitable_task_model (
                std::allocator_arg_t, Allocator const & alloc,
                F && f, Args && ... args
            )
                : _f    (std::allocator_arg_t (), alloc, std::forward <F> (f))
                , _args (std::allocator_arg_t (), alloc,
                         std::forward <Args> (args)...)
            {}

            std::future <R> get_future (void)
            {
                return this->_f.get_future ();
            }

            void invoke_ (void) override { this->do_invoke_ (); }

            bool ready_ (void) const noexcept override
            {
                return this->do_ready_ ();
            }

        private:
            template <class A, class T>
            static inline auto call_get (T && t) noexcept
                -> decltype (std::forward <A> (t))
            {
                return std::forward <A> (t);
            }

            template <class A, class T>
            static inline auto call_get (std::future <T> && t) noexcept
                -> decltype (std::forward <A> (std::move (t).get ()))
            {
                return std::forward <A> (std::move (t).get ());
            }

            template <std::size_t ... I>
            inline void do_invoke_ (utility::index_sequence <I...>)
            {
                utility::invoke (
                    this->_f, call_get <CallArgs> (
                        std::get <I> (std::move (this->_args))
                    )...
                );
            }

            template <class T>
            static inline bool call_ready (T const &) noexcept { return true; }

            template <class T>
            static inline bool call_ready (std::future <T> const & t) noexcept
            {
                return t.wait_for (std::chrono::seconds (0)) ==
                    std::future_status::ready;
            }

            template <std::size_t ... I>
            inline bool do_ready_ (utility::index_sequence <I...>) const
                noexcept
            {
                for (bool r : {call_ready (std::get <I> (this->_args))...}) {
                    if (!r) return false;
                }

                return true;
            }

            std::packaged_task <R (CallArgs...)> _f;
            std::tuple <FutArgs...> _args;
        };

        std::unique_ptr <task_concept> _t;
    };

    /*
     * awaitable_task_system; a work-stealing tasking system for awaitable_tasks
     * partly inspired by Sean Parent's "Better Code: Concurrency" talk; see
     * http://sean-parent.stlab.cc.
     */
    template <class Allocator = std::allocator <awaitable_task>>
    class awaitable_task_system
    {
        class task_queue
        {
            using iterator_type =
                typename std::forward_list <awaitable_task>::iterator;

            std::forward_list <awaitable_task> tasks_;
            iterator_type last_;
            std::condition_variable cv_;
            std::mutex mutex_;
            std::atomic_bool done_ {false};

            /* rotates the first element of the list onto the end */
            void rotate_with_ (std::forward_list <awaitable_task> & side_buf)
            {
                /* zero or one element list -- trivial to rotate */
                if (this->tasks_.empty () ||
                        this->last_ == this->tasks_.begin ())
                    return;

                side_buf.splice_after (side_buf.begin (), this->tasks_);
                this->tasks_.splice_after (
                    this->tasks_.begin (), side_buf,
                    side_buf.begin (), side_buf.end ()
                );

                auto new_last = side_buf.begin ();
                this->tasks_.splice_after (this->last_, side_buf);
                this->last_ = new_last;
            }

        public:
            task_queue (void)
                : tasks_ {}
            {}

            task_queue (task_queue const &) = delete;

            task_queue (task_queue && other) noexcept
                : tasks_ (std::move (other).tasks_)
                , last_  (std::move (other).last_)
                , done_  (other.done_.load ())
            {}

            void set_done (void)
            {
                this->done_.store (true);
                this->cv_.notify_all ();
            }

            std::pair <bool, awaitable_task> try_pop (void)
            {
                std::unique_lock <std::mutex>
                    lock (this->mutex_, std::try_to_lock);
                if (!lock || this->tasks_.empty ()) {
                    return std::make_pair (false, awaitable_task {});
                } else {
                    auto t = std::move (this->tasks_.front ());
                    this->tasks_.pop_front ();
                    return std::make_pair (true, std::move (t));
                }
            }

            bool try_push (awaitable_task & t)
            {
                {
                    std::unique_lock <std::mutex>
                        lock (this->mutex_, std::try_to_lock);
                    if (!lock)
                        return false;

                    if (this->tasks_.empty ()) {
                        this->tasks_.emplace_front (std::move (t));
                        this->last_ = this->tasks_.begin ();
                    } else {
                        this->last_ = this->tasks_.emplace_after (
                            this->last_, std::move (t)
                        );
                    }
                }

                this->cv_.notify_one ();
                return true;
            }

            std::pair <bool, awaitable_task> pop (void)
            {
                std::unique_lock <std::mutex> lock (this->mutex_);
                while (this->tasks_.empty () && !this->done_)
                    this->cv_.wait (lock);

                if (this->tasks_.empty ())
                    return std::make_pair (false, awaitable_task {});

                auto iter = this->tasks_.begin ();
                auto const old_last = this->last_;
                std::forward_list <awaitable_task> side_buf;

                if (iter->ready ()) {
                    auto t = std::move (*iter);
                    this->tasks_.pop_front ();
                    return std::make_pair (true, std::move (t));
                } else {
                    this->rotate_with_ (side_buf);
                }

                for (auto lag = iter++; lag != old_last; lag = iter++) {
                    if (iter->ready ()) {
                        if (this->last_ == iter)
                            this->last_ = lag;
                        auto t = std::move (*iter);
                        this->tasks_.erase_after (lag);
                        return std::make_pair (true, std::move (t));
                    } else {
                        this->rotate_with_ (side_buf);
                    }
                }

                /*
                 * If we get to this point the best we can do is pop from the
                 * front of the task list, release the lock, and wait for the
                 * task to be ready.
                 */
                auto t = std::move (this->tasks_.front ());
                this->tasks_.pop_front ();
                lock.unlock ();

                while (!t.ready ())
                    std::this_thread::yield ();

                return std::make_pair (true, std::move (t));
            }

            void push (awaitable_task t)
            {
                {
                    std::unique_lock <std::mutex> lock (this->mutex_);
                    this->last_ = this->tasks_.emplace_after (
                        this->last_, std::move (t)
                    );
                }
                this->cv_.notify_one ();
            }
        };

        std::vector <task_queue> queues_;
        std::vector <std::thread> threads_;
        typename Allocator::template rebind <awaitable_task::task_concept>::other
            alloc_;
        std::size_t nthreads_;
        std::size_t current_index_ {0};

        void run (std::size_t id)
        {
            while (true) {
                std::pair <bool, awaitable_task> p;

                for (std::size_t k = 0; k < 10 * this->nthreads_; ++k) {
                    p = this->queues_ [(id + k) % this->nthreads_].try_pop ();
                    if (p.first)
                        break;
                }

                if (!p.first) {
                    p = this->queues_ [id].pop ();
                    if (!p.first)
                        return;
                }

                p.second ();
            }
        }

    public:
        awaitable_task_system (void)
            : awaitable_task_system (std::thread::hardware_concurrency ())
        {}

        awaitable_task_system (std::size_t nthreads,
                           Allocator const & alloc = Allocator ())
            : queues_   {}
            , threads_  {}
            , alloc_    (alloc)
            , nthreads_ {nthreads}
        {
            this->queues_.reserve (nthreads);
            for (std::size_t th = 0; th < nthreads; ++th)
                this->queues_.emplace_back ();

            this->threads_.reserve (nthreads);
            for (std::size_t th = 0; th < nthreads; ++th)
                this->threads_.emplace_back (
                    &awaitable_task_system::run, this, th
                );
        }

        ~awaitable_task_system (void)
        {
            this->done ();
            for (auto & th : this->threads_)
                th.join ();
        }

        void done (void) noexcept
        {
            for (auto & q : this->queues_)
                q.set_done ();
        }

        template <class F, class ... Args>
        auto push_ready (F && f, Args && ... args)
            -> typename std::remove_reference <
                decltype (make_ready_task (
                    std::allocator_arg_t {}, this->alloc_,
                    std::forward <F> (f), std::forward <Args> (args)...
                ).second)
            >::type
        {
            auto t = make_ready_task (
                std::allocator_arg_t {}, this->alloc_,
                std::forward <F> (f), std::forward <Args> (args)...
            );

            auto const idx = this->current_index_++;
            for (std::size_t k = 0; k < 10 * this->nthreads_; ++k)
                if (this->queues_ [(idx + k) % this->nthreads_]
                        .try_push (t.first))
                    return t.second;

            this->queues_ [idx % this->nthreads_].push (std::move (t.first));
            return t.second;
        }

        template <class F, class ... Args>
        auto push_async (F && f, Args && ... args)
            -> typename std::remove_reference <
                decltype (make_awaitable_task (
                    std::allocator_arg_t {}, this->alloc_,
                    std::forward <F> (f), std::forward <Args> (args)...
                ).second)
            >::type
        {
            auto t = make_awaitable_task (
                std::allocator_arg_t {}, this->alloc_,
                std::forward <F> (f), std::forward <Args> (args)...
            );

            auto const idx = this->current_index_++;
            for (std::size_t k = 0; k < 10 * this->nthreads_; ++k)
                if (this->queues_ [(idx + k) % this->nthreads_]
                        .try_push (t.first))
                    return t.second;

            this->queues_ [idx % this->nthreads_].push (std::move (t.first));
            return t.second;
        }

        template <class F, class ... Args>
        auto push (F && f, Args && ... args)
            -> decltype (this->push_async (
                std::forward <F> (f), std::forward <Args> (args)...
            ))
        {
            return this->push_async (
                std::forward <F> (f), std::forward <Args> (args)...
            );
        }

        void push (awaitable_task && t)
        {
            auto const idx = this->current_index_++;
            for (std::size_t k = 0; k < 10 * this->nthreads_; ++k)
                if (this->queues_ [(idx + k) % this->nthreads_].try_push (t))
                    return;

            this->queues_ [idx % this->nthreads_].push (std::move (t));
        }
    };
}   // namespace dsa

#endif  // #ifndef DSA_AWAITABLE_TASK_HPP
