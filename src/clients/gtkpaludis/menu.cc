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

#include "menu.hh"
#include <gtkmm/main.h>
#include <gtkmm/actiongroup.h>
#include <gtkmm/uimanager.h>
#include <gtkmm/stock.h>

using namespace paludis;
using namespace gtkpaludis;

namespace paludis
{
    template<>
    struct Implementation<Menu>
    {
        Glib::RefPtr<Gtk::ActionGroup> action_group;
        Glib::RefPtr<Gtk::UIManager> ui_manager;

        Implementation() :
            action_group(Gtk::ActionGroup::create()),
            ui_manager(Gtk::UIManager::create())
        {
        }
    };
}

Menu::Menu() :
    PrivateImplementationPattern<Menu>(new Implementation<Menu>)
{
    _imp->ui_manager->insert_action_group(_imp->action_group);
    _imp->action_group->add(Gtk::Action::create("FileMenu", "File"));
    _imp->action_group->add(Gtk::Action::create("Quit", Gtk::Stock::QUIT),
            sigc::mem_fun(this, &Menu::action_quit));
    _imp->ui_manager->add_ui_from_string(
            "<ui>"
            "  <menubar name='MenuBar'>"
            "    <menu action='FileMenu'>"
            "      <menuitem action='Quit' />"
            "    </menu>"
            "  </menubar>"
            "</ui>"
            );
}

Menu::~Menu()
{
}

Gtk::Widget *
Menu::get_menu_widget()
{
    return _imp->ui_manager->get_widget("/MenuBar");
}

void
Menu::action_quit()
{
    Gtk::Main::quit();
}

void
Menu::add_accel_group_to(Gtk::Window * w)
{
    w->add_accel_group(_imp->ui_manager->get_accel_group());
}

