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

#ifndef PALUDIS_GUARD_SRC_GTKPALUDIS_MENU_HH
#define PALUDIS_GUARD_SRC_GTKPALUDIS_MENU_HH 1

#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <gtkmm/widget.h>
#include <gtkmm/window.h>

namespace gtkpaludis
{
    class Menu :
        private paludis::PrivateImplementationPattern<Menu>
    {
        private:
            void action_quit();

        public:
            Menu();
            ~Menu();

            Gtk::Widget * get_menu_widget();
            void add_accel_group_to(Gtk::Window *);
    };
}

#endif
