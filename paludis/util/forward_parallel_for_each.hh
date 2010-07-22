/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009 Ciaran McCreesh
 *
 * This file is part of the Paludis package manager. Paludis is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License version 2, as published by the Free Software Foundation.
 *
 * Paludis is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_FORWARD_PARALLEL_FOR_EACH_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_FORWARD_PARALLEL_FOR_EACH_HH 1

#include <paludis/util/mutex.hh>
#include <paludis/util/thread_pool.hh>
#include <functional>

/** \file
 * Declarations for the forward_parallel_for_each function.
 *
 * \ingroup g_threads
 *
 * \section Examples
 *
 * - None at this time.
 */

namespace paludis
{
    /**
     * For forward_parallel_for_each.
     *
     * \see forward_parallel_for_each
     * \since 0.36
     * \ingroup g_threads
     */
    template <typename Iter_, typename Func_>
    void forward_parallel_for_each_thread_func(Iter_ & cur, const Iter_ & end, Mutex & mutex, Func_ & func,
            unsigned n_at_once)
    {
        while (true)
        {
            unsigned n_to_do(0);
            Iter_ i(end);
            {
                Lock lock(mutex);
                if (cur == end)
                    return;

                i = cur;
                while (n_to_do < n_at_once && cur != end)
                {
                    ++cur;
                    ++n_to_do;
                }
            }

            for (unsigned n(0) ; n < n_to_do ; ++n)
                func(*i++);
        }
    }

    /**
     * Like std::for_each, but in parallel, and no return value.
     *
     * This works for forward iterators, but is only effective if each
     * calculation is quite slow.
     *
     * \since 0.36
     * \ingroup g_threads
     */
    template <typename Iter_, typename Func_>
    void forward_parallel_for_each(Iter_ cur, const Iter_ & end, Func_ func, unsigned n_threads, unsigned n_at_once)
    {
        if (cur == end)
            return;

        Mutex mutex;
        ThreadPool threads;

        for (unsigned n(0) ; n != n_threads ; ++n)
            threads.create_thread(std::bind(&forward_parallel_for_each_thread_func<Iter_, Func_>, std::ref(cur),
                        std::cref(end), std::ref(mutex), std::ref(func), n_at_once));
    }
}

#endif
