/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008 Ciaran McCreesh
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

#include <src/output/colour.hh>
#include "sync.hh"
#include <paludis/sync_task.hh>
#include <paludis/syncer.hh>
#include <iomanip>
#include <iostream>
#include <string>

/** \file
 * Handle the --sync action for the main paludis program.
 */

using namespace paludis;
using std::cerr;
using std::cout;
using std::endl;

namespace
{
    class OurSyncTask :
        public SyncTask
    {
        private:
            int _return_code;

        public:
            OurSyncTask(std::shared_ptr<Environment> env, const bool p) :
                SyncTask(env.get(), p),
                _return_code(0)
            {
            }

            virtual void on_sync_all_pre();
            virtual void on_sync_pre(const RepositoryName &);
            virtual void on_sync_post(const RepositoryName &);
            virtual void on_sync_skip(const RepositoryName &);
            virtual void on_sync_fail(const RepositoryName &, const SyncFailedError &);
            virtual void on_sync_succeed(const RepositoryName &);
            virtual void on_sync_all_post();

            virtual void on_sync_status(const int x, const int y, const int a);

            int return_code() const
            {
                return _return_code;
            }
    };

    void
    OurSyncTask::on_sync_all_pre()
    {
    }

    void
    OurSyncTask::on_sync_pre(const RepositoryName & r)
    {
        cout << colour(cl_heading, "Sync " + stringify(r)) << endl;
    }

    void
    OurSyncTask::on_sync_post(const RepositoryName &)
    {
    }

    void
    OurSyncTask::on_sync_skip(const RepositoryName & r)
    {
        cout << "Sync " << r << " skipped" << endl;
    }

    void
    OurSyncTask::on_sync_succeed(const RepositoryName & r)
    {
        cout << "Sync " << r << " completed" << endl;
    }

    void
    OurSyncTask::on_sync_fail(const RepositoryName & r, const SyncFailedError & e)
    {
        _return_code |= 1;
        cout << endl;
        cerr << "Sync error:" << endl;
        cerr << "  * " << e.backtrace("\n  * ") << e.message() << endl;
        cerr << endl;
        cout << "Sync " << r << " failed" << endl;
    }

    void
    OurSyncTask::on_sync_all_post()
    {
        cout << endl;
    }

    void
    OurSyncTask::on_sync_status(const int x, const int y, const int a)
    {
        cerr << xterm_title("Syncing " + stringify(x) + " of " + stringify(y) + ", "
                + stringify(a) + " active");
    }
}

int do_sync(const std::shared_ptr<Environment> & env)
{
    Context context("When performing sync action from command line:");

    bool parallel(false);
    parallel = CommandLine::get_instance()->a_multitask.specified();

    OurSyncTask task(env, parallel);

    for (CommandLine::ParametersConstIterator q(CommandLine::get_instance()->begin_parameters()),
            q_end(CommandLine::get_instance()->end_parameters()) ; q != q_end ; ++q)
        task.add_target(*q);

    task.execute();
    cout << endl;

    return task.return_code();
}

