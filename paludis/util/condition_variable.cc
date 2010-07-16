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

#include <paludis/util/condition_variable.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/log.hh>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <cstring>

using namespace paludis;

ConditionVariable::ConditionVariable() :
    _cond(new pthread_cond_t)
{
    pthread_cond_init(_cond, 0);
}

ConditionVariable::~ConditionVariable()
{
    pthread_cond_destroy(_cond);
    delete _cond;
}

void
ConditionVariable::broadcast()
{
    pthread_cond_broadcast(_cond);
}

void
ConditionVariable::signal()
{
    pthread_cond_signal(_cond);
}

void
ConditionVariable::acquire_then_signal(Mutex & m)
{
    Lock l(m);
    signal();
}

void
ConditionVariable::wait(Mutex & m)
{
    pthread_cond_wait(_cond, m.posix_mutex());
}

bool
ConditionVariable::timed_wait(Mutex & m, const unsigned n, const unsigned ms)
{
    struct timespec t;
    clock_gettime(CLOCK_REALTIME, &t);

    t.tv_sec += n;
    t.tv_nsec += (1000000 * ms);

    if (t.tv_nsec >= 1000000000)
    {
        t.tv_nsec -= 1000000000;
        t.tv_sec += 1;
    }

    int r(pthread_cond_timedwait(_cond, m.posix_mutex(), &t));

    if (0 == r)
        return true;
    else
    {
        if (ETIMEDOUT != r)
            Log::get_instance()->message("condition_variable.timed_wait_failed", ll_warning, lc_context)
                << "pthread_cond_timedwait returned " << std::strerror(r) << ", something icky happened";
        return false;
    }
}

