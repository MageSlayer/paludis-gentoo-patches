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

#include "main_window.hh"
#include "sets_page.hh"
#include "packages_page.hh"
#include "repositories_page.hh"
#include "messages.hh"
#include "tasks_page.hh"

#include <paludis/util/log.hh>

#include <gtkmm/box.h>
#include <gtkmm/notebook.h>
#include <gtkmm/table.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/main.h>
#include <gtkmm/statusbar.h>

#include <list>

using namespace gtkpaludis;
using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<MainWindow> :
        InternalCounted<Implementation<MainWindow> >
    {
        unsigned lock_count;

        Glib::Dispatcher dispatcher;

        Gtk::Table main_table;

        Gtk::Notebook main_notebook;
        TasksPage tasks_page;
        PackagesPage packages_page;
        SetsPage sets_page;
        RepositoriesPage repositories_page;
        Gtk::Table messages_page;
        Messages messages;

        Gtk::Statusbar status_bar;

        int messages_page_id;

        Implementation() :
            lock_count(0),
            main_table(1, 2, false),
            messages_page(1, 1, false)
        {
        }
    };
}

MainWindow::MainWindow() :
    PrivateImplementationPattern<MainWindow>(new Implementation<MainWindow>)
{
    set_title("gtkpaludis");
    set_border_width(2);
    set_default_size(600, 400);

    _imp->main_notebook.set_border_width(5);
//    _imp->main_notebook.append_page(_imp->tasks_page, "Common Tasks");
    _imp->main_notebook.append_page(_imp->packages_page, "Packages");
    _imp->main_notebook.append_page(_imp->sets_page, "Sets");
    _imp->main_notebook.append_page(_imp->repositories_page, "Repositories");
    _imp->messages_page_id = _imp->main_notebook.append_page(_imp->messages_page, "Messages");

    add(_imp->main_table);
    _imp->main_table.attach(_imp->main_notebook, 0, 1, 0, 1);
    _imp->status_bar.set_has_resize_grip(true);
    _imp->main_table.attach(_imp->status_bar, 0, 1, 1, 2, Gtk::FILL, Gtk::AttachOptions(0));

    _imp->messages_page.attach(_imp->messages, 0, 1, 0, 1);

    show_all_children();
}

void
MainWindow::_set_lock(bool value)
{
    set_sensitive(! value);
}

MainWindow::~MainWindow()
{
}

void
MainWindow::populate()
{
    _imp->packages_page.populate();
    _imp->repositories_page.populate();
    _imp->sets_page.populate();
}

void
MainWindow::lock_controls()
{
    if (0 == _imp->lock_count++)
        _set_lock(true);
}

void
MainWindow::maybe_unlock_controls()
{
    if (0 == --_imp->lock_count)
        _set_lock(false);
}

void
MainWindow::push_status(const std::string & s)
{
    _imp->status_bar.push(s);
}

void
MainWindow::pop_status()
{
    _imp->status_bar.pop();
}

void
MainWindow::show_messages_page()
{
    _imp->main_notebook.set_current_page(_imp->messages_page_id);
}

void
MainWindow::show_exception(const std::string & what, const std::string & message, bool fatal)
{
    Gtk::MessageDialog dialog(*this, fatal ? "Fatal Error" : "Error", false, Gtk::MESSAGE_ERROR);
    dialog.set_secondary_text(message + " (" + what + ")");
    dialog.run();

    if (fatal)
        Gtk::Main::quit();
}

void
MainWindow::message(const std::string & s)
{
    _imp->messages.message(s);
}

