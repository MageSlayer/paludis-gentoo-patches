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

#include "sync.hh"
#include "main_window.hh"
#include <paludis/environment/default/default_environment.hh>

using namespace paludis;
using namespace gtkpaludis;

OurSyncTask::OurSyncTask(PaludisThread::Launchable * const l) :
    SyncTask(DefaultEnvironment::get_instance()),
    _l(l)
{
}

OurSyncTask::~OurSyncTask()
{
}

void
OurSyncTask::on_sync_all_pre()
{
    _l->dispatch(sigc::mem_fun(MainWindow::get_instance(), &MainWindow::show_messages_page));
    _l->dispatch(sigc::bind<1>(sigc::mem_fun(MainWindow::get_instance(), &MainWindow::message),
                stringify("Starting sync")));
}

void
OurSyncTask::on_sync_pre(const RepositoryName & r)
{
    _l->dispatch(sigc::bind<1>(sigc::mem_fun(MainWindow::get_instance(), &MainWindow::message),
                stringify("Syncing '" + stringify(r) + "'")));
}

void
OurSyncTask::on_sync_post(const RepositoryName &)
{
}

void
OurSyncTask::on_sync_skip(const RepositoryName & r)
{
    _l->dispatch(sigc::bind<1>(sigc::mem_fun(MainWindow::get_instance(), &MainWindow::message),
                stringify("Sync of '" + stringify(r) + "' skipped")));
}

void
OurSyncTask::on_sync_fail(const RepositoryName & r, const SyncFailedError &)
{
    _l->dispatch(sigc::bind<1>(sigc::mem_fun(MainWindow::get_instance(), &MainWindow::message),
                stringify("Sync of '" + stringify(r) + "' failed")));
}

void
OurSyncTask::on_sync_succeed(const RepositoryName & r)
{
    _l->dispatch(sigc::bind<1>(sigc::mem_fun(MainWindow::get_instance(), &MainWindow::message),
                stringify("Sync of '" + stringify(r) + "' succeeded")));
}

void
OurSyncTask::on_sync_all_post()
{
    _l->dispatch(sigc::bind<1>(sigc::mem_fun(MainWindow::get_instance(), &MainWindow::message),
                stringify("Sync completed")));
}

