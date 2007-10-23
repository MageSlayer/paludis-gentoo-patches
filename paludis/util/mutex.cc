/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh
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

#include <paludis/util/mutex.hh>

using namespace paludis;

#ifdef PALUDIS_ENABLE_THREADS

Mutex::Mutex() :
    _attr(new pthread_mutexattr_t),
    _mutex(new pthread_mutex_t)
{
    pthread_mutexattr_init(_attr);
    pthread_mutexattr_settype(_attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(_mutex, _attr);
}

Mutex::~Mutex()
{
    pthread_mutex_destroy(_mutex);
    pthread_mutexattr_destroy(_attr);

    delete _mutex;
    delete _attr;
}

pthread_mutex_t *
Mutex::posix_mutex()
{
    return _mutex;
}

Lock::Lock(Mutex & m) :
    _mutex(&m)
{
    pthread_mutex_lock(_mutex->posix_mutex());
}

Lock::~Lock()
{
    pthread_mutex_unlock(_mutex->posix_mutex());
}

void
Lock::acquire_then_release_old(Mutex & m)
{
    pthread_mutex_lock(m.posix_mutex());
    pthread_mutex_unlock(_mutex->posix_mutex());
    _mutex = &m;
}

TryLock::TryLock(Mutex & m) :
    _mutex(&m)
{
    if (0 != pthread_mutex_trylock(_mutex->posix_mutex()))
        _mutex = 0;
}

TryLock::~TryLock()
{
    if (_mutex)
        pthread_mutex_unlock(_mutex->posix_mutex());
}

bool
TryLock::operator() () const
{
    return _mutex;
}

#else

Mutex::Mutex()
{
}

Mutex::~Mutex()
{
}

Lock::Lock(Mutex &)
{
}

void
Lock::acquire_then_release_old(Mutex &)
{
}

Lock::~Lock()
{
}

TryLock::TryLock(Mutex &)
{
}

TryLock::~TryLock()
{
}

bool
TryLock::operator() () const
{
    return true;
}

#endif

