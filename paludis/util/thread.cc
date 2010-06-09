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

#include <paludis/util/thread.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/log.hh>
#include <string.h>
#include <errno.h>

#ifdef __linux__
#  include <sys/time.h>
#  include <sys/resource.h>
#  include <unistd.h>
#  include <sys/syscall.h>
#endif

using namespace paludis;

Thread::Thread(const std::tr1::function<void () throw ()> & f) :
    _thread(new pthread_t),
    _func(f)
{
    int err;

    if (0 != ((err = pthread_create(_thread, 0, &thread_func, this))))
        throw InternalError(PALUDIS_HERE, "pthread_create failed: " + stringify(strerror(err)));
}

void *
Thread::thread_func(void * r)
{
    try
    {
        static_cast<Thread *>(r)->_func();
    }
    catch (const Exception & e)
    {
        static_cast<Thread *>(r)->_exception = e.backtrace(": ") + e.message() + " (" + e.what() + ")";
    }
    catch (const std::exception & e)
    {
        static_cast<Thread *>(r)->_exception = e.what();
    }
    return 0;
}

Thread::~Thread()
{
    pthread_join(*_thread, 0);
    delete _thread;

    if (! _exception.empty())
        throw InternalError(PALUDIS_HERE, "Exception '" + _exception + "' uncaught in child thread");
}

void
Thread::idle_adapter(const std::tr1::function<void () throw ()> & f)
{
#ifdef __linux__
    if (-1 == setpriority(PRIO_PROCESS, syscall(SYS_gettid), std::max(19, getpriority(PRIO_PROCESS, 0) + 10)))
        Log::get_instance()->message("util.thread.setpriority", ll_warning, lc_context) << "Failed to setpriority: " << strerror(errno);
#else
#  warning "Don't know how to set thread priorities on your platform"
#endif
    f();
}

