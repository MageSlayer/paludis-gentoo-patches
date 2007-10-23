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

#include <paludis/util/idle_action_pool.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/instantiation_policy-impl.hh>
#include <paludis/util/action_queue.hh>
#include <paludis/util/log.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/system.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/exception.hh>
#include <ostream>

using namespace paludis;

#include <paludis/util/idle_action_pool-se.cc>

template class InstantiationPolicy<IdleActionPool, instantiation_method::SingletonTag>;

namespace
{
#ifdef PALUDIS_ENABLE_THREADS
    struct Stats
    {
        Mutex mutex;
        unsigned success, failure, already_completed, enqueued, used, unprepared;

        Stats() :
            success(0),
            failure(0),
            already_completed(0),
            enqueued(0),
            used(0),
            unprepared(0)
        {
        }

        ~Stats()
        {
            Lock l(mutex);
            Log::get_instance()->message(ll_debug, lc_no_context) << "Idle action pool stats: success "
                << success << " failure " << failure << " already completed " << already_completed
                << " forgotten " << (enqueued - success - failure - already_completed) << " used " << used
                << " unprepared " << unprepared;
        }
    };
#endif
}

namespace paludis
{
    template <>
    struct Implementation<IdleActionPool>
    {
#ifdef PALUDIS_ENABLE_THREADS
        Stats stats;
        ActionQueue pool;

        Implementation() :
            pool(destringify<int>(getenv_with_default("PALUDIS_IDLE_THREAD_COUNT", "2")), true)
        {
        }

        ~Implementation()
        {
            pool.forget_pending();
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
IdleActionPool::required_idle_action(const tr1::function<IdleActionResult () throw ()> & f)
{
#ifdef PALUDIS_ENABLE_THREADS
    _imp->pool.enqueue(tr1::bind(tr1::mem_fn(&IdleActionPool::_count_result), this, f));
#else
    f();
#endif
}

void
IdleActionPool::optional_idle_action(
#ifdef PALUDIS_ENABLE_THREADS
        const tr1::function<IdleActionResult () throw ()> & f
#else
        const tr1::function<IdleActionResult () throw ()> &
#endif
        )
{
#ifdef PALUDIS_ENABLE_THREADS
    _imp->pool.enqueue(tr1::bind(tr1::mem_fn(&IdleActionPool::_count_result), this, f));
#endif
}

void
#ifdef PALUDIS_ENABLE_THREADS
IdleActionPool::_count_result(const tr1::function<IdleActionResult () throw ()> & f)
#else
IdleActionPool::_count_result(const tr1::function<IdleActionResult () throw ()> &)
#endif
{
#ifdef PALUDIS_ENABLE_THREADS
    {
        Lock l(_imp->stats.mutex);
        ++_imp->stats.enqueued;
    }

    IdleActionResult r(f());

    {
        Lock l(_imp->stats.mutex);
        switch (r)
        {
            case iar_success:
                ++_imp->stats.success;
                break;

            case iar_failure:
                ++_imp->stats.failure;
                break;

            case iar_already_completed:
                ++_imp->stats.already_completed;
                break;

            case last_iar:
                break;
        }
    }
#endif
}

void
IdleActionPool::increase_used_stat()
{
#ifdef PALUDIS_ENABLE_THREADS
    Lock l(_imp->stats.mutex);
    ++_imp->stats.used;
#endif
}

void
IdleActionPool::increase_unprepared_stat()
{
#ifdef PALUDIS_ENABLE_THREADS
    Lock l(_imp->stats.mutex);
    ++_imp->stats.unprepared;
#endif
}

