/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_THREAD_POOL_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_THREAD_POOL_HH 1

#include <paludis/util/attributes.hh>
#include <paludis/util/pimp.hh>
#include <functional>

/** \file
 * Declarations for the ThreadPool class.
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
     * A thread pool holds a number of related threads.
     *
     * \ingroup g_threads
     * \nosubgrouping
     * \since 0.26
     */
    class PALUDIS_VISIBLE ThreadPool
    {
        private:
            Pimp<ThreadPool> _imp;

        public:
            ///\name Basic operations
            ///\{

            ThreadPool();
            ~ThreadPool();

            ///\}

            /**
             * Create a new thread in our pool.
             */
            void create_thread(const std::function<void () throw ()> &);

            /**
             * How many threads does our pool contain?
             */
            unsigned number_of_threads() const;
    };

    extern template class Pimp<ThreadPool>;
}

#endif
