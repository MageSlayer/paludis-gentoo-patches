/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_MUTEX_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_MUTEX_HH 1

#include <paludis/util/attributes.hh>

#ifdef PALUDIS_ENABLE_THREADS
#  include <pthread.h>
#endif

namespace paludis
{
    class PALUDIS_VISIBLE Mutex
    {
        private:
            Mutex(const Mutex &);
            Mutex & operator= (const Mutex &);

#ifdef PALUDIS_ENABLE_THREADS
            pthread_mutexattr_t * const _attr;
            pthread_mutex_t * const _mutex;
#endif

        public:
            explicit Mutex();
            ~Mutex();

#ifdef PALUDIS_ENABLE_THREADS
            pthread_mutex_t * const posix_mutex() PALUDIS_ATTRIBUTE((warn_unused_result));
#endif
    };

    class PALUDIS_VISIBLE Lock
    {
        private:
            Lock(const Lock &);
            Lock & operator= (const Lock &);

#ifdef PALUDIS_ENABLE_THREADS
            Mutex * const _mutex;
#endif

        public:
            explicit Lock(Mutex &);
            ~Lock();
    };

    class PALUDIS_VISIBLE TryLock
    {
        private:
            TryLock(const TryLock &);
            TryLock & operator= (const TryLock &);

#ifdef PALUDIS_ENABLE_THREADS
            Mutex * _mutex;
#endif

        public:
            explicit TryLock(Mutex &);
            ~TryLock();

            bool operator() () const PALUDIS_ATTRIBUTE((warn_unused_result));
    };
}

#endif
