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

#include "test_common.hh"
#include "paludis_thread.hh"
#include "main_window.hh"

#include <gtkmm/main.h>
#include <paludis/util/fs_entry.hh>
#include <test/test_framework.hh>

using namespace paludis;
using namespace gtkpaludis;
using namespace test;

namespace
{
    struct DoNothing :
        PaludisThread::Launchable
    {
        virtual void operator() ()
        {
        }
    };

    struct Tester
    {
        int exit_status;
        sigc::connection connection;

        Tester() :
            exit_status(EXIT_FAILURE)
        {
        }

        void run()
        {
            connection.disconnect();

            GtkMainQuitOnDestruction d PALUDIS_ATTRIBUTE((unused));
            exit_status = TestCaseList::run_tests() ? EXIT_SUCCESS : EXIT_FAILURE;
        }
    };
}

GtkMainQuitOnDestruction::~GtkMainQuitOnDestruction()
{
    bool busy(true);
    while (busy)
    {
        busy = false;

        while (Gtk::Main::events_pending())
        {
            busy = true;
            Gtk::Main::iteration();
        }

        while (! PaludisThread::get_instance()->try_lock_unlock())
        {
            busy = true;
            PaludisThread::get_instance()->launch(DoNothing::Pointer(new DoNothing));
        }
    }

    Gtk::Main::quit();
}

int main(int argc, char * argv[])
{
    Gtk::Main kit(argc, argv);
    setenv("PALUDIS_HOME", stringify(FSEntry::cwd() / test_dir() / "home").c_str(), 1);

    if (! Glib::thread_supported())
        Glib::thread_init();

    Tester tester;

    Glib::signal_idle().connect(sigc::bind_return(sigc::mem_fun(MainWindow::get_instance(),
                &MainWindow::populate), false));
    tester.connection = launch_signal_connection(sigc::mem_fun(tester, &Tester::run));
    Gtk::Main::run(*MainWindow::get_instance());

    return tester.exit_status;
}

