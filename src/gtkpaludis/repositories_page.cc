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

#include "repositories_page.hh"
#include "repositories_list.hh"
#include "paludis_thread.hh"
#include "main_window.hh"

#include <paludis/environment/default/default_environment.hh>
#include <gtkmm/button.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/box.h>
#include <gtkmm/buttonbox.h>

using namespace gtkpaludis;
using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<RepositoriesPage> :
        InternalCounted<Implementation<RepositoriesPage> >
    {
        Gtk::ScrolledWindow repositories_list_scroll;
        RepositoriesList repositories_list;

        Gtk::HButtonBox buttons_box;
        Gtk::Button sync_button;

        Implementation() :
            sync_button("Sync")
        {
        }
    };
}

RepositoriesPage::RepositoriesPage() :
    Gtk::Table(2, 2, false),
    PrivateImplementationPattern<RepositoriesPage>(new Implementation<RepositoriesPage>)
{
    set_col_spacings(5);
    set_row_spacings(5);
    set_border_width(5);

    _imp->repositories_list_scroll.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_ALWAYS);
    _imp->repositories_list_scroll.add(_imp->repositories_list);
    attach(_imp->repositories_list_scroll, 0, 1, 0, 2, Gtk::SHRINK, Gtk::FILL | Gtk::EXPAND);


    _imp->buttons_box.set_border_width(5);
    _imp->buttons_box.set_spacing(5);
    _imp->buttons_box.set_layout(Gtk::BUTTONBOX_END);
    _imp->buttons_box.add(_imp->sync_button);
    attach(_imp->buttons_box, 1, 2, 1, 2, Gtk::FILL | Gtk::EXPAND, Gtk::AttachOptions(0));

    _imp->sync_button.signal_clicked().connect(sigc::mem_fun(this,
                &RepositoriesPage::_sync_button_clicked));
    _imp->repositories_list.get_selection()->signal_changed().connect(sigc::mem_fun(this,
                &RepositoriesPage::_repository_list_selection_changed));
}

RepositoriesPage::~RepositoriesPage()
{
}

void
RepositoriesPage::populate()
{
    _imp->repositories_list.populate();
}

void
RepositoriesPage::_repository_list_selection_changed()
{
}

namespace
{
    class Sync :
        public PaludisThread::Launchable
    {
        private:
            RepositoryName _name;

        public:
            Sync(const RepositoryName & name) :
                _name(name)
            {
            }

            virtual void operator() ();
    };

    void
    Sync::operator() ()
    {
        dispatch(sigc::bind<1>(sigc::mem_fun(MainWindow::get_instance(),
                        &MainWindow::push_status), "Syncing repository '" + stringify(_name) + "'..."));

        Repository::ConstPointer repo(DefaultEnvironment::get_instance()->package_database()->fetch_repository(_name));
        if (repo->syncable_interface)
        {
            dispatch(sigc::mem_fun(MainWindow::get_instance(), &MainWindow::show_messages_page));
            repo->syncable_interface->sync();
        }

        dispatch(sigc::mem_fun(MainWindow::get_instance(), &MainWindow::pop_status));
        dispatch(sigc::mem_fun(MainWindow::get_instance(), &MainWindow::populate));
    }
}

void
RepositoriesPage::_sync_button_clicked()
{
    RepositoryName current_repository(_imp->repositories_list.current_repository());
    if (current_repository.data() == "no-repository")
        return;

    PaludisThread::get_instance()->launch(Sync::Pointer(new Sync(current_repository)));
}

