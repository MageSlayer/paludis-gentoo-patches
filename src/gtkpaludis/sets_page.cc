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

#include "sets_page.hh"
#include "sets_list.hh"
#include "set_overview.hh"

#include <gtkmm/button.h>
#include <gtkmm/entry.h>
#include <gtkmm/label.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/separator.h>
#include <gtkmm/stock.h>
#include <gtkmm/box.h>
#include <gtkmm/buttonbox.h>

using namespace gtkpaludis;
using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<SetsPage> :
        InternalCounted<Implementation<SetsPage> >
    {
        Gtk::ScrolledWindow sets_list_scroll;
        SetsList sets_list;

        Gtk::ScrolledWindow sets_overview_scroll;
        SetOverview set_overview;

        Gtk::HButtonBox buttons_box;
        Gtk::Button install_button;

        Implementation() :
            install_button("Install")
        {
        }
    };
}

SetsPage::SetsPage() :
    Gtk::Table(2, 2, false),
    PrivateImplementationPattern<SetsPage>(new Implementation<SetsPage>)
{
    set_col_spacings(5);
    set_row_spacings(5);
    set_border_width(5);

    _imp->sets_list_scroll.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_ALWAYS);
    _imp->sets_list_scroll.add(_imp->sets_list);
    attach(_imp->sets_list_scroll, 0, 1, 0, 2, Gtk::SHRINK, Gtk::FILL | Gtk::EXPAND);

    _imp->sets_overview_scroll.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    _imp->sets_overview_scroll.add(_imp->set_overview);
    attach(_imp->sets_overview_scroll, 1, 2, 0, 1);

    _imp->buttons_box.set_border_width(5);
    _imp->buttons_box.set_spacing(5);
    _imp->buttons_box.set_layout(Gtk::BUTTONBOX_END);
    _imp->buttons_box.add(_imp->install_button);
    attach(_imp->buttons_box, 1, 2, 1, 2, Gtk::FILL | Gtk::EXPAND, Gtk::AttachOptions(0));

    _imp->install_button.signal_clicked().connect(sigc::mem_fun(this,
                &SetsPage::_install_button_clicked));
    _imp->sets_list.get_selection()->signal_changed().connect(sigc::mem_fun(this,
                &SetsPage::_sets_list_selection_changed));
}

SetsPage::~SetsPage()
{
}

void
SetsPage::populate()
{
    _imp->sets_list.populate();
}

void
SetsPage::_sets_list_selection_changed()
{
    _imp->set_overview.populate(_imp->sets_list.current_set());
}

void
SetsPage::_install_button_clicked()
{
}

