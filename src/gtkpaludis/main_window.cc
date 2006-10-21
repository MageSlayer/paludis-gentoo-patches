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
#include "categories_list.hh"
#include "packages_list.hh"
#include "package_info.hh"

#include <paludis/util/log.hh>

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/entry.h>
#include <gtkmm/label.h>
#include <gtkmm/notebook.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/separator.h>
#include <gtkmm/stock.h>
#include <gtkmm/table.h>

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
        Gtk::Table packages_page;
        Gtk::Table sets_page;
        Gtk::Table repositories_page;

        Gtk::ScrolledWindow status_label_box;
        Gtk::Label status_label;

        Gtk::HBox packages_search;
        Gtk::Entry packages_search_entry;
        Gtk::Button packages_search_button;
        Gtk::HSeparator packages_search_button_sep;

        Gtk::ScrolledWindow categories_list_scroll;
        CategoriesList categories_list;

        Gtk::ScrolledWindow packages_list_scroll;
        PackagesList packages_list;

        PackageInfo package_info;

        std::list<std::string> status;

        Implementation() :
            lock_count(0),
            main_table(1, 2, false),
            packages_page(2, 3, false),
            status_label("", Gtk::ALIGN_LEFT),
            packages_search_button(Gtk::Stock::FIND)
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
    _imp->main_notebook.append_page(_imp->packages_page, "Packages");
    _imp->main_notebook.append_page(_imp->sets_page, "Sets");
    _imp->main_notebook.append_page(_imp->repositories_page, "Repositories");

    add(_imp->main_table);
    _imp->main_table.attach(_imp->main_notebook, 0, 1, 0, 1);
    _imp->main_table.attach(_imp->status_label_box, 0, 1, 1, 2, Gtk::FILL, Gtk::AttachOptions(0));
    _imp->status_label_box.set_border_width(5);
    _imp->status_label_box.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_NEVER);
    _imp->status_label_box.add(_imp->status_label);

    _imp->packages_page.set_col_spacings(5);
    _imp->packages_page.set_row_spacings(5);
    _imp->packages_page.set_border_width(5);

    _imp->packages_search.pack_start(_imp->packages_search_entry, Gtk::PACK_EXPAND_WIDGET);
    _imp->packages_search.pack_start(_imp->packages_search_button_sep, Gtk::PACK_SHRINK, 5);
    _imp->packages_search.pack_end(_imp->packages_search_button, Gtk::PACK_SHRINK);
    _imp->packages_search_entry.set_width_chars(30);
    _imp->packages_page.attach(_imp->packages_search, 0, 2, 0, 1, Gtk::FILL | Gtk::EXPAND, Gtk::AttachOptions(0));

    _imp->categories_list_scroll.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_ALWAYS);
    _imp->categories_list_scroll.add(_imp->categories_list);
    _imp->packages_page.attach(_imp->categories_list_scroll, 0, 1, 1, 2, Gtk::SHRINK, Gtk::FILL | Gtk::EXPAND);

    _imp->packages_list_scroll.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_ALWAYS);
    _imp->packages_list_scroll.add(_imp->packages_list);
    _imp->packages_page.attach(_imp->packages_list_scroll, 1, 2, 1, 2);

    _imp->packages_page.attach(_imp->package_info, 0, 2, 2, 3);

    _imp->categories_list.get_selection()->signal_changed().connect(sigc::mem_fun(*this,
                &MainWindow::_category_list_selection_changed));
    _imp->packages_list.get_selection()->signal_changed().connect(sigc::mem_fun(*this,
                &MainWindow::_package_list_selection_changed));

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
    _imp->categories_list.populate();
}

void
MainWindow::_category_list_selection_changed()
{
    CategoryNamePart c("dummy");
    _imp->packages_list.populate(_imp->categories_list.current_category());
}

void
MainWindow::_package_list_selection_changed()
{
    QualifiedPackageName c("dummy/dummy");
    _imp->package_info.populate(_imp->packages_list.current_package());
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
    _imp->status.push_back(s);
    _update_status();
}

void
MainWindow::pop_status()
{
    _imp->status.pop_back();
    _update_status();
}

void
MainWindow::_update_status()
{
    if (_imp->status.empty())
        _imp->status_label.set_label("");
    else
        _imp->status_label.set_label(_imp->status.back());
}

