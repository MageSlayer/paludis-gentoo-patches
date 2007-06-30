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

#include <paludis/util/thread.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/stringify.hh>
#include <string.h>

using namespace paludis;

#ifdef PALUDIS_ENABLE_THREADS

Thread::Thread(const tr1::function<void () throw ()> & f) :
    _thread(new pthread_t),
    _func(f),
    _detached(false)
{
    int err;
    if (0 != ((err = pthread_create(_thread, 0, &thread_func, this))))
        throw InternalError(PALUDIS_HERE, "pthread_create failed: " + stringify(strerror(err)));
}

Thread::Thread(const tr1::function<void () throw ()> & f,
        const tr1::function<void (Thread * const) throw ()> & pf) :
    _thread(new pthread_t),
    _func(f),
    _post_func(pf),
    _detached(false)
{
    int err;
    if (0 != ((err = pthread_create(_thread, 0, &thread_func, this))))
        throw InternalError(PALUDIS_HERE, "pthread_create failed: " + stringify(strerror(err)));
}

void *
Thread::thread_func(void * r)
{
    static_cast<Thread *>(r)->_func();
    if (static_cast<Thread *>(r)->_post_func)
        static_cast<Thread *>(r)->_post_func(static_cast<Thread *>(r));

    return 0;
}

Thread::~Thread()
{
    if (! _detached)
        pthread_join(*_thread, 0);
    delete _thread;
}

void
Thread::detach()
{
    if (! _detached)
    {
        pthread_detach(*_thread);
        _detached = true;
    }
}

#else

Thread::Thread(const tr1::function<void () throw ()> & f)
{
    f();
}

Thread::Thread(const tr1::function<void () throw ()> & f,
        const tr1::function<void (Thread * const) throw ()> & pf)
{
    f();
    pf();
}

Thread::~Thread()
{
}

void
Thread::detach()
{
}

#endif

namespace
{
    void post_delete_func(Thread * const t) throw ()
    {
        delete t;
    }
}

void
paludis::run_detached(const tr1::function<void () throw ()> & f)
{
    (new Thread(f, &post_delete_func))->detach();
}

