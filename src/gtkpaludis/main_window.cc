/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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
#include "browse_tree.hh"
#include "information_tree.hh"
#include "vte_message_window.hh"

#include <paludis/about.hh>
#include <paludis/util/stringify.hh>

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/entry.h>
#include <gtkmm/frame.h>
#include <gtkmm/label.h>
#include <gtkmm/paned.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/stock.h>
#include <gtkmm/textview.h>

using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<MainWindow> :
        InternalCounted<MainWindow>
    {
        Gtk::VBox main_container;

        Gtk::HBox search_container;
        Gtk::Label search_label;
        Gtk::Entry search_box;
        Gtk::Button search_button;

        Gtk::VPaned browse_information_messages_pane;
        Gtk::HPaned browse_information_pane;

        Gtk::Frame information_frame;
        Gtk::ScrolledWindow information_window;
        InformationTree information_tree;

        Gtk::Frame browse_frame;
        Gtk::ScrolledWindow browse_window;
        BrowseTree browse_tree;

        Gtk::Frame messages_frame;
        Gtk::ScrolledWindow messages_window;
        VteMessageWindow vte_messages;

        Implementation(MainWindow * const main_window);
    };
}

Implementation<MainWindow>::Implementation(MainWindow * const main_window) :
    main_container(false, 5),
    search_container(false, 5),
    search_label(" Search: "),
    search_button(Gtk::Stock::FIND),
    information_frame(" Information: "),
    browse_frame(" Browse: "),
    browse_tree(main_window, &information_tree),
    messages_frame(" Messages: ")
{
}

MainWindow::MainWindow() :
    PrivateImplementationPattern<MainWindow>(new Implementation<MainWindow>(this))
{
    set_title("gtkPaludis " + stringify(PALUDIS_VERSION_MAJOR) + "." +
            stringify(PALUDIS_VERSION_MINOR) + "." +
            stringify(PALUDIS_VERSION_MICRO));

    set_default_size(600, 400);
    set_border_width(5);

    add(_imp->main_container);

    _imp->main_container.pack_start(_imp->search_container, Gtk::PACK_SHRINK);
    _imp->main_container.pack_end(_imp->browse_information_messages_pane, Gtk::PACK_EXPAND_WIDGET);

    _imp->search_container.pack_start(_imp->search_label, Gtk::PACK_SHRINK);
    _imp->search_container.pack_start(_imp->search_box);
    _imp->search_container.pack_end(_imp->search_button, Gtk::PACK_SHRINK);

    _imp->browse_information_messages_pane.pack1(_imp->browse_information_pane);
    _imp->browse_information_messages_pane.pack2(_imp->messages_frame);

    _imp->browse_information_pane.pack1(_imp->browse_frame);
    _imp->browse_information_pane.pack2(_imp->information_frame);

    _imp->browse_frame.add(_imp->browse_window);
    _imp->browse_window.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    _imp->browse_window.set_border_width(5);
    _imp->browse_window.add(_imp->browse_tree);

    _imp->information_frame.add(_imp->information_window);
    _imp->information_window.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    _imp->information_window.set_border_width(5);
    _imp->information_window.add(_imp->information_tree);

    _imp->messages_frame.add(_imp->messages_window);
    _imp->messages_window.set_border_width(5);
    _imp->messages_window.add(_imp->vte_messages);
    _imp->messages_window.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

    show_all_children();
}

MainWindow::~MainWindow()
{
}

void
MainWindow::set_children_sensitive(bool value)
{
    _imp->browse_tree.set_sensitive(value);
    _imp->information_tree.set_sensitive(value);
    _imp->search_box.set_sensitive(value);
    _imp->search_button.set_sensitive(value);
}

