/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2012 Ciaran McCreesh
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

#include <paludis/util/attributes.hh>
#include <functional>
#include <string>
#include <pthread.h>

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
            pthread_t * const _thread;
            const std::function<void () throw ()> _func;
            std::string _exception;

            static void * thread_func(void *);

        public:
            ///\name Basic operations
            ///\{

            Thread(const std::function<void () throw ()> &);
            ~Thread();

            ///\}
    };
}

#endif
