/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#include "sync.hh"
#include "main_window.hh"

#include <paludis/default_environment.hh>
#include <paludis/util/log.hh>
#include <paludis/syncer.hh>

#include <list>

using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<OurSyncTask> :
        InternalCounted<Implementation<OurSyncTask> >
    {
        std::list<std::string> failed_repositories;
    };
}

OurSyncTask::OurSyncTask() :
    SyncTask(DefaultEnvironment::get_instance()),
    PrivateImplementationPattern<OurSyncTask>(new Implementation<OurSyncTask>)
{
}

OurSyncTask::~OurSyncTask()
{
}

void
OurSyncTask::on_sync_all_pre()
{
}

void
OurSyncTask::on_sync_pre(const RepositoryName &)
{
}

void
OurSyncTask::on_sync_post(const RepositoryName &)
{
}

void
OurSyncTask::on_sync_skip(const RepositoryName &)
{
}

void
OurSyncTask::on_sync_succeed(const RepositoryName &)
{
}

void
OurSyncTask::on_sync_fail(const RepositoryName & r, const SyncFailedError &)
{
    Log::get_instance()->message(ll_warning, lc_no_context, "Sync of repository '"
            + stringify(r) + "' failed");
    PrivateImplementationPattern<OurSyncTask>::_imp->failed_repositories.push_back(stringify(r));
}

void
OurSyncTask::on_sync_all_post()
{
}

OurSyncTask::FailedIterator
OurSyncTask::begin_failed() const
{
    return FailedIterator(PrivateImplementationPattern<OurSyncTask>::_imp->failed_repositories.begin());
}

OurSyncTask::FailedIterator
OurSyncTask::end_failed() const
{
    return FailedIterator(PrivateImplementationPattern<OurSyncTask>::_imp->failed_repositories.end());
}

