/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/package_database.hh>
#include <paludis/hook.hh>
#include <list>

using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<SyncTask>
    {
        Environment * const env;
        std::list<RepositoryName> targets;

        Implementation(Environment * const e) :
            env(e)
        {
        }
    };
}

SyncTask::SyncTask(Environment * const env) :
    PrivateImplementationPattern<SyncTask>(new Implementation<SyncTask>(env))
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

void
SyncTask::execute()
{
    Context context("When executing sync task:");

    if (_imp->targets.empty())
        for (PackageDatabase::RepositoryIterator r(_imp->env->package_database()->begin_repositories()),
                r_end(_imp->env->package_database()->end_repositories()) ; r != r_end ; ++r)
            _imp->targets.push_back((*r)->name());

    if (0 !=
        _imp->env->perform_hook(Hook("sync_all_pre")("TARGETS", join(_imp->targets.begin(),
                         _imp->targets.end(), " "))).max_exit_status)
        throw SyncFailedError("Sync aborted by hook");
    on_sync_all_pre();

    int x(0), y(std::distance(_imp->targets.begin(), _imp->targets.end()));
    for (std::list<RepositoryName>::const_iterator r(_imp->targets.begin()), r_end(_imp->targets.end()) ;
            r != r_end ; ++r)
    {
        Context context_local("When syncing repository '" + stringify(*r) + "':");
        ++x;

        try
        {
            if (0 !=
                _imp->env->perform_hook(Hook("sync_pre")("TARGET", stringify(*r))
                         ("X_OF_Y", stringify(x) + " of " + stringify(y))).max_exit_status)
                throw SyncFailedError("Sync of '" + stringify(*r) + "' aborted by hook");
            on_sync_pre(*r);

            tr1::shared_ptr<const Repository> rr(_imp->env->package_database()->fetch_repository(*r));

            if (rr->syncable_interface && rr->syncable_interface->sync())
                on_sync_succeed(*r);
            else
                on_sync_skip(*r);

            on_sync_post(*r);
            if (0 !=
                _imp->env->perform_hook(Hook("sync_post")("TARGET", stringify(*r))
                             ("X_OF_Y", stringify(x) + " of " + stringify(y))).max_exit_status)
                throw SyncFailedError("Sync of '" + stringify(*r) + "' aborted by hook");
        }
        catch (const SyncFailedError & e)
        {
            HookResult PALUDIS_ATTRIBUTE((unused)) dummy(_imp->env->perform_hook(Hook("sync_fail")("TARGET", stringify(*r))
                    ("X_OF_Y", stringify(x) + " of " + stringify(y))));
            on_sync_fail(*r, e);
        }
    }

    on_sync_all_post();
    if (0 !=
        _imp->env->perform_hook(Hook("sync_all_post")("TARGETS", join(_imp->targets.begin(),
                         _imp->targets.end(), " "))).max_exit_status)
        throw SyncFailedError("Sync aborted by hook");
}

