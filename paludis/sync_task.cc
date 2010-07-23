/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010 Ciaran McCreesh
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

#include "sync_task.hh"
#include <paludis/environment.hh>
#include <paludis/syncer.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/action_queue.hh>
#include <paludis/util/mutex.hh>
#include <paludis/package_database.hh>
#include <paludis/hook.hh>
#include <paludis/create_output_manager_info.hh>
#include <paludis/output_manager.hh>
#include <functional>
#include <algorithm>
#include <list>

using namespace paludis;

namespace paludis
{
    template<>
    struct Imp<SyncTask>
    {
        Environment * const env;
        std::list<RepositoryName> targets;
        const bool parallel;

        Imp(Environment * const e, const bool p) :
            env(e),
            parallel(p)
        {
        }
    };

    template <>
    struct WrappedForwardIteratorTraits<SyncTask::TargetsConstIteratorTag>
    {
        typedef std::list<RepositoryName>::const_iterator UnderlyingIterator;
    };
}

SyncTask::SyncTask(Environment * const env, const bool p) :
    Pimp<SyncTask>(env, p)
{
}

SyncTask::~SyncTask()
{
}

void
SyncTask::add_target(const std::string & t)
{
    Context context("When adding sync target '" + t + "':");
    _imp->targets.push_back(RepositoryName(t));
}

namespace
{
    struct ItemSyncer
    {
        typedef void result;

        Mutex mutex;
        int x, y, a;
        Environment * const env;
        SyncTask * const task;

        ItemSyncer(int yy, Environment * const e, SyncTask * const t) :
            x(0),
            y(yy),
            a(0),
            env(e),
            task(t)
        {
        }

        void sync(const RepositoryName & r)
        {
            Context context_local("When syncing repository '" + stringify(r) + "':");

            {
                Lock l(mutex);
                ++x;
                ++a;
                task->on_sync_status(x, y, a);
            }

            try
            {
                if (0 !=
                        env->perform_hook(Hook("sync_pre")("TARGET", stringify(r))
                            ("X_OF_Y", stringify(x) + " of " + stringify(y) + " (" + stringify(a) + " active)")
                            ).max_exit_status())
                    throw SyncFailedError("Sync of '" + stringify(r) + "' aborted by hook");

                {
                    Lock l(mutex);
                    task->on_sync_pre(r);
                }

                std::shared_ptr<const Repository> rr(env->package_database()->fetch_repository(r));
                CreateOutputManagerForRepositorySyncInfo info(rr->name(), oe_exclusive, ClientOutputFeatures());
                std::shared_ptr<OutputManager> output_manager(env->create_output_manager(info));
                if (rr->sync(output_manager))
                {
                    Lock l(mutex);
                    task->on_sync_succeed(r);
                }
                else
                {
                    Lock l(mutex);
                    task->on_sync_skip(r);
                }
                output_manager->succeeded();

                {
                    Lock l(mutex);
                    task->on_sync_post(r);
                }

                if (0 !=
                        env->perform_hook(Hook("sync_post")("TARGET", stringify(r))
                            ("X_OF_Y", stringify(x) + " of " + stringify(y) + " (" + stringify(a) + " active)")
                            ).max_exit_status())
                    throw SyncFailedError("Sync of '" + stringify(r) + "' aborted by hook");

                {
                    Lock l(mutex);
                    --a;
                    task->on_sync_status(x, y, a);
                }
            }
            catch (const SyncFailedError & e)
            {
                HookResult PALUDIS_ATTRIBUTE((unused)) dummy(env->perform_hook(Hook("sync_fail")("TARGET", stringify(r))
                            ("X_OF_Y", stringify(x) + " of " + stringify(y) + " (" + stringify(a) + " active)")
                            ));
                Lock l(mutex);
                task->on_sync_fail(r, e);
                --a;
                task->on_sync_status(x, y, a);
            }
        }
    };
}

void
SyncTask::execute()
{
    Context context("When executing sync task:");

    if (_imp->targets.empty())
        for (PackageDatabase::RepositoryConstIterator r(_imp->env->package_database()->begin_repositories()),
                r_end(_imp->env->package_database()->end_repositories()) ; r != r_end ; ++r)
            _imp->targets.push_back((*r)->name());

    if (0 !=
        _imp->env->perform_hook(Hook("sync_all_pre")("TARGETS", join(_imp->targets.begin(),
                         _imp->targets.end(), " "))).max_exit_status())
        throw SyncFailedError("Sync aborted by hook");
    on_sync_all_pre();

    ItemSyncer s(std::distance(_imp->targets.begin(), _imp->targets.end()), _imp->env, this);

    using namespace std::placeholders;
    if (_imp->parallel)
    {
        ActionQueue actions(5);
        for (std::list<RepositoryName>::const_iterator t(_imp->targets.begin()), t_end(_imp->targets.end()) ;
                t != t_end ; ++t)
            actions.enqueue(std::bind(&ItemSyncer::sync, &s, *t));
    }
    else
        std::for_each(_imp->targets.begin(), _imp->targets.end(), std::bind(&ItemSyncer::sync, &s, _1));

    for (PackageDatabase::RepositoryConstIterator r(_imp->env->package_database()->begin_repositories()),
            r_end(_imp->env->package_database()->end_repositories()) ; r != r_end ; ++r)
    {
        (*r)->invalidate();
        (*r)->purge_invalid_cache();
    }

    on_sync_all_post();
    if (0 !=
        _imp->env->perform_hook(Hook("sync_all_post")("TARGETS", join(_imp->targets.begin(),
                         _imp->targets.end(), " "))).max_exit_status())
        throw SyncFailedError("Sync aborted by hook");
}

SyncTask::TargetsConstIterator
SyncTask::begin_targets() const
{
    return TargetsConstIterator(_imp->targets.begin());
}

SyncTask::TargetsConstIterator
SyncTask::end_targets() const
{
    return TargetsConstIterator(_imp->targets.end());
}

template class WrappedForwardIterator<SyncTask::TargetsConstIteratorTag, const RepositoryName>;

