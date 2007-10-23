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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_FUTURE_IMPL_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_FUTURE_IMPL_HH 1

#include <paludis/util/future.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/condition_variable.hh>
#include <paludis/util/tr1_memory.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>

namespace paludis
{
    template <typename T_>
    void adapt_for_future(
            tr1::function<T_ () throw ()> f,
            tr1::shared_ptr<tr1::shared_ptr<T_> > result,
            tr1::shared_ptr<Mutex> mutex,
            tr1::shared_ptr<ConditionVariable> condition);

    /**
     * Implementation data for a Future.
     *
     * \ingroup g_threads
     * \nosubgrouping
     */
    template <>
    template <typename T_>
    struct Implementation<Future<T_> >
    {
        const tr1::function<T_ () throw ()> f;
        mutable tr1::shared_ptr<tr1::shared_ptr<T_> > result;
        mutable tr1::shared_ptr<Mutex> mutex;
        mutable tr1::shared_ptr<ConditionVariable> condition;

        Implementation(const tr1::function<T_ () throw ()> & fn) :
            f(fn),
            result(new tr1::shared_ptr<T_>),
            mutex(new Mutex),
            condition(new ConditionVariable)
        {
            FutureActionQueue::get_instance()->enqueue(tr1::bind(paludis::adapt_for_future<T_>, f, result, mutex, condition));
        }
    };
}

template <typename T_>
paludis::Future<T_>::Future(const tr1::function<T_ () throw ()> & f) :
    PrivateImplementationPattern<Future<T_> >(new Implementation<Future<T_> >(f))
{
}

template <typename T_>
paludis::Future<T_>::~Future()
{
    Lock l(*_imp->mutex);
    if (! *_imp->result)
    {
        _imp->result->reset(new T_(_imp->f()));
        _imp->condition->broadcast();
    }
}

template <typename T_>
T_
paludis::Future<T_>::operator() () const
{
    Lock l(*_imp->mutex);

    if (! *_imp->result)
    {
        _imp->result->reset(new T_(_imp->f()));
        _imp->condition->broadcast();
    }

    return **_imp->result;
}

template <typename T_>
void paludis::adapt_for_future(
        tr1::function<T_ () throw ()> f,
        tr1::shared_ptr<tr1::shared_ptr<T_> > result,
        tr1::shared_ptr<Mutex> mutex,
        tr1::shared_ptr<ConditionVariable> condition)
{
    try
    {
        TryLock l(*mutex);
        if (l())
        {
            if (! *result)
                result->reset(new T_(f()));
            condition->broadcast();
        }
    }
    catch (...)
    {
        // exception will be raised when operator() is called
    }
}

#endif
