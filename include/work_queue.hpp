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
#include <memory>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>


namespace dsa
{
    template <class Callable, class ... Args>
    inline typename std::result_of <Callable (Args...)>::type
        invoke (Callable && c, Args && ... args)
        noexcept (noexcept (
            std::forward <Callable> (c) (std::forward <Args> (args)...)
        ))
    {
        return std::forward <Callable> (c) (std::forward <Args> (args)...);
    }

    template <class B, class T, class D>
    inline auto invoke (T B::*p, D && d) -> decltype (std::forward <D> (d).*p)
    {
        return std::forward <D> (d).*p;
    }

    template <class M, class Ptr>
    inline auto invoke (M && m, Ptr && p)
        -> decltype (*std::forward <Ptr> (p).*std::forward <M> (m))
    {
        return *std::forward <Ptr> (p).*std::forward <M> (m);
    }

    template <class B, class T, class D, class ... Args>
    inline auto invoke (T B::*f, D && d, Args && ... args)
        -> decltype ((std::forward <D> (d).*f) (std::forward <Args> (args)...))
    {
        return (std::forward <D> (d).*f) (std::forward <Args> (args)...);
    }

    template <class F, class Ptr, class ... Args>
    inline auto invoke (F && f, Ptr && p, Args && ... args)
        -> decltype (
            (*std::forward <Ptr> (p)).*std::forward <F> (f) (
                std::forward <Args> (args)...
            )
        )
    {
        return (*std::forward <Ptr> (p)).*std::forward <F> (f) (
            std::forward <Args> (args)...
        );
    }

    class packaged_task
    {
        friend void invoke (packaged_task &);

    private:
        std::unique_ptr <void *> _t;
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
