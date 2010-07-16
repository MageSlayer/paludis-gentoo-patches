/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_CONDITION_VARIABLE_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_CONDITION_VARIABLE_HH 1

#include <paludis/util/attributes.hh>
#include <paludis/util/mutex.hh>
#include <pthread.h>

/** \file
 * Declarations for the ConditionVariable class.
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
     * A basic condition variable.
     *
     * If threading is disabled, waiting and signalling are no-ops.
     *
     * \ingroup g_threads
     * \since 0.26
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE ConditionVariable
    {
        private:
            ConditionVariable(const ConditionVariable &);
            ConditionVariable & operator= (const ConditionVariable &);

            pthread_cond_t * const _cond;

        public:
            ///\name Basic operations
            ///\{

            ConditionVariable();
            ~ConditionVariable();

            ///\}

            /**
             * Broadcast to all waiting threads.
             */
            void broadcast();

            /**
             * Signal one waiting thread.
             */
            void signal();

            /**
             * Acquire the specified Mutex, then signal.
             */
            void acquire_then_signal(Mutex &);

            /**
             * Wait, using the specified Mutex for synchronisation.
             */
            void wait(Mutex &);

            /**
             * Wait, using the specified Mutex for synchronisation,
             * but return false if more than n seconds m ms elapse.
             *
             * \since 0.49 for ms argument
             */
            bool timed_wait(Mutex &, const unsigned n, const unsigned m = 0);
    };
}

#endif
