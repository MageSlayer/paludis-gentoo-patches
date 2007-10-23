/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_THREAD_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_THREAD_HH 1

#include <paludis/util/tr1_functional.hh>
#include <paludis/util/attributes.hh>

#ifdef PALUDIS_ENABLE_THREADS
#  include <pthread.h>
#endif

/** \file
 * Declarations for the Thread class.
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
     * A basic thread class.
     *
     * If threading is disabled, the threaded function is executed immediately
     * in the current context.
     *
     * \ingroup g_threads
     * \since 0.26
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE Thread
    {
        private:
#ifdef PALUDIS_ENABLE_THREADS
            pthread_t * const _thread;
            const tr1::function<void () throw ()> _func;

            static void * thread_func(void *);
#endif

        public:
            ///\name Basic operations
            ///\{

            Thread(const tr1::function<void () throw ()> &);
            ~Thread();

            ///\}

            /**
             * Adapt a function for use in IdleActionPool.
             */
            static void idle_adapter(const tr1::function<void () throw ()> &);
    };
}

#endif
