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

#include <paludis/util/idle_action_pool.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/instantiation_policy-impl.hh>
#include <paludis/util/action_queue.hh>

using namespace paludis;

template class InstantiationPolicy<IdleActionPool, instantiation_method::SingletonTag>;

namespace paludis
{
    template <>
    struct Implementation<IdleActionPool>
    {
#ifdef PALUDIS_ENABLE_THREADS
        ActionQueue pool;

        Implementation() :
            pool(5)
        {
        }
#endif
    };
}


IdleActionPool::IdleActionPool() :
    PrivateImplementationPattern<IdleActionPool>(new Implementation<IdleActionPool>)
{
}

IdleActionPool::~IdleActionPool()
{
}

void
IdleActionPool::required_idle_action(const tr1::function<void () throw ()> & f)
{
#ifdef PALUDIS_ENABLE_THREADS
    _imp->pool.enqueue(f);
#else
    f();
#endif
}

void
IdleActionPool::optional_idle_action(
#ifdef PALUDIS_ENABLE_THREADS
        const tr1::function<void () throw ()> & f
#else
        const tr1::function<void () throw ()> &
#endif
        )
{
#ifdef PALUDIS_ENABLE_THREADS
    _imp->pool.enqueue(f);
#endif
}

