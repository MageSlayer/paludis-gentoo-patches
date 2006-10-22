/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_SRC_GTKPALUDIS_SYNC_HH
#define PALUDIS_GUARD_SRC_GTKPALUDIS_SYNC_HH 1

#include <paludis/tasks/sync_task.hh>
#include <src/gtkpaludis/paludis_thread.hh>

namespace gtkpaludis
{
    class OurSyncTask :
        public paludis::SyncTask
    {
        private:
            PaludisThread::Launchable * const _l;

        public:
            OurSyncTask(PaludisThread::Launchable * const l);
            virtual ~OurSyncTask();

            virtual void on_sync_all_pre();
            virtual void on_sync_pre(const paludis::RepositoryName &);
            virtual void on_sync_post(const paludis::RepositoryName &);
            virtual void on_sync_skip(const paludis::RepositoryName &);
            virtual void on_sync_fail(const paludis::RepositoryName &, const paludis::SyncFailedError &);
            virtual void on_sync_succeed(const paludis::RepositoryName &);
            virtual void on_sync_all_post();
    };
}

#endif
