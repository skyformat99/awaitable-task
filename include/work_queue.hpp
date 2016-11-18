//
// dsa is a utility library of data structures and algorithms built with C++11.
// This file (bst.hpp) is part of the dsa project.
//
// bstree; a generic binary search-tree (with unique keys) implementation for
// C++11 or later.
//
// A description of a binary search tree can be found here:
//
//      https://en.wikipedia.org/wiki/Binary_search_tree
//
// author: Dalton Woodard
// contact: daltonmwoodard@gmail.com
// repository: https://github.com/daltonwoodard/bstree.hpp
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

#ifndef DSA_WORK_QUEUE_HPP
#define DSA_WORK_QUEUE_HPP

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <future>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>
#include "utilities/include/functions.hpp"


namespace dsa
{
    /*
     * task; a std::packaged_task that also contains its own arguments.
     */
    class task
    {
        template <class F, class ... Args>
        friend std::pair <
            task, std::future <typename std::result_of <F (Args...)>::type>
        > make_ready_task (F && f, Args && ... args)
        {
            using pair_type = std::pair <
                task, std::future <typename std::result_of <F (Args...)>::type>
            >;
            using model_type = ready_task_model <
                typename std::result_of <F (Args...)>::type (Args...)
            >;

            task t (std::forward <F> (f), std::forward <Args> (args)...);
            auto fut = dynamic_cast <model_type &> (*t._t).get_future ();
            return pair_type (std::move (t), std::move (fut));
        }

        void operator() (void)
        {
            this->_t->invoke_ ();
        }

        bool ready (void) const noexcept
        {
            return this->_t->ready_ ();
        }

    private:
        struct ready_task_t {};
        struct async_task_t {};

        template <class F, class ... Args>
        task (ready_task_t, F && f, Args && ... args)
            : _t (
                new ready_task_model <
                    typename std::result_of <F (Args...)>::type (Args...)
                > (std::forward <F> (f), std::forward <Args> (args)...)
            )
        {}

        struct task_concept
        {
            virtual ~task_concept (void) noexcept {}
            virtual void invoke_ (void) = 0;
            virtual bool ready_ (void) const noexcept = 0;
        };

        template <class> struct ready_task_model;

        template <class R, class ... Args>
        struct ready_task_model <R (Args...)> : task_concept
        {
            template <class F>
            explicit ready_task_model (F && f, Args && ... args)
                : _f    (std::forward <F> (f))
                , _args (std::make_tuple (std::forward <Args> (args)...))
            {}

            template <class F, class Allocator>
            explicit ready_task_model (
                std::allocator_arg_t, Allocator const & alloc,
                F && f, Args && ... args
            )
                : _f    (std::allocator_arg_t {}, alloc, std::forward <F> (f))
                , _args (std::make_tuple (std::forward <Args> (args)...))
            {}

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

        std::unique_ptr <task_concept> _t;
    };

    template <class Allocator /*= std::allocator <...>*/>
    class work_queue
    {
        class notification_queue
        {
        };
    };
}   // namespace dsa

#endif  // #ifndef DSA_WORK_QUEUE_HPP
