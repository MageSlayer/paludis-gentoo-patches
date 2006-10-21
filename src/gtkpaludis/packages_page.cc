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

#include "packages_page.hh"
#include "categories_list.hh"
#include "packages_list.hh"
#include "package_info.hh"

#include <gtkmm/button.h>
#include <gtkmm/entry.h>
#include <gtkmm/label.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/separator.h>
#include <gtkmm/stock.h>
#include <gtkmm/box.h>

using namespace gtkpaludis;
using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<PackagesPage> :
        InternalCounted<Implementation<PackagesPage> >
    {
        Gtk::HBox packages_search;
        Gtk::Entry packages_search_entry;
        Gtk::Button packages_search_button;
        Gtk::HSeparator packages_search_button_sep;

        Gtk::ScrolledWindow categories_list_scroll;
        CategoriesList categories_list;

        Gtk::ScrolledWindow packages_list_scroll;
        PackagesList packages_list;

        PackageInfo package_info;

        Implementation() :
            packages_search_button(Gtk::Stock::FIND)
        {
        }
    };
}

PackagesPage::PackagesPage() :
    Gtk::Table(2, 3, false),
    PrivateImplementationPattern<PackagesPage>(new Implementation<PackagesPage>)
{
    set_col_spacings(5);
    set_row_spacings(5);
    set_border_width(5);

    _imp->packages_search.pack_start(_imp->packages_search_entry, Gtk::PACK_EXPAND_WIDGET);
    _imp->packages_search.pack_start(_imp->packages_search_button_sep, Gtk::PACK_SHRINK, 5);
    _imp->packages_search.pack_end(_imp->packages_search_button, Gtk::PACK_SHRINK);
    _imp->packages_search_entry.set_width_chars(30);
    attach(_imp->packages_search, 0, 2, 0, 1, Gtk::FILL | Gtk::EXPAND, Gtk::AttachOptions(0));

    _imp->categories_list_scroll.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_ALWAYS);
    _imp->categories_list_scroll.add(_imp->categories_list);
    attach(_imp->categories_list_scroll, 0, 1, 1, 2, Gtk::SHRINK, Gtk::FILL | Gtk::EXPAND);

    _imp->packages_list_scroll.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_ALWAYS);
    _imp->packages_list_scroll.add(_imp->packages_list);
    attach(_imp->packages_list_scroll, 1, 2, 1, 2);

    attach(_imp->package_info, 0, 2, 2, 3);

    _imp->categories_list.get_selection()->signal_changed().connect(sigc::mem_fun(this,
                &PackagesPage::_category_list_selection_changed));
    _imp->packages_list.get_selection()->signal_changed().connect(sigc::mem_fun(this,
                &PackagesPage::_package_list_selection_changed));
}

PackagesPage::~PackagesPage()
{
}

void
PackagesPage::populate()
{
    _imp->categories_list.populate();
}

void
PackagesPage::_category_list_selection_changed()
{
    _imp->packages_list.populate(_imp->categories_list.current_category());
}

void
PackagesPage::_package_list_selection_changed()
{
    _imp->package_info.populate(_imp->packages_list.current_package());
}


