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

#include <paludis/util/future.hh>
#include <paludis/util/future-impl.hh>
#include <paludis/util/instantiation_policy-impl.hh>
#include <paludis/util/system.hh>

using namespace paludis;

template class InstantiationPolicy<FutureActionQueue, instantiation_method::SingletonTag>;

FutureActionQueue::FutureActionQueue() :
    ActionQueue(destringify<int>(getenv_with_default("PALUDIS_FUTURE_THREAD_COUNT", "5")), true)
{
}

FutureActionQueue::~FutureActionQueue()
{
}

namespace
{
    unsigned make_non_void(const tr1::function<void () throw ()> & f)
    {
        f();
        return 0;
    }
}

namespace paludis
{
    template <>
    template <>
    struct Implementation<Future<void> >
    {
        const tr1::function<void () throw ()> f;
        const tr1::function<unsigned () throw ()> adapted_f;
        mutable tr1::shared_ptr<tr1::shared_ptr<unsigned > > result;
        mutable tr1::shared_ptr<Mutex> mutex;
        mutable tr1::shared_ptr<ConditionVariable> condition;

        Implementation(const tr1::function<void () throw ()> & fn) :
            f(fn),
            adapted_f(tr1::bind(&make_non_void, fn)),
            result(new tr1::shared_ptr<unsigned >),
            mutex(new Mutex),
            condition(new ConditionVariable)
        {
            FutureActionQueue::get_instance()->enqueue(
                    tr1::bind(&adapt_for_future<unsigned>, adapted_f, result, mutex, condition));
        }
    };
}

Future<void>::Future(const tr1::function<void () throw ()> & f) :
    PrivateImplementationPattern<Future<void> >(new Implementation<Future<void> >(f))
{
}

Future<void>::~Future()
{
    Lock l(*_imp->mutex);
    if (! *_imp->result)
    {
        _imp->f();
        _imp->result->reset(new unsigned(0));
        _imp->condition->broadcast();
    }
}

void
Future<void>::operator() () const
{
    Lock l(*_imp->mutex);
    if (! *_imp->result)
    {
        _imp->f();
        _imp->result->reset(new unsigned(0));
        _imp->condition->broadcast();
    }
}

