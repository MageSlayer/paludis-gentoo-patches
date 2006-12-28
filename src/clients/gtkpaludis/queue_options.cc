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

#include "queue_options.hh"
#include <gtkmm/table.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/label.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/frame.h>

using namespace paludis;
using namespace gtkpaludis;

namespace paludis
{
    template<>
    struct Implementation<QueueOptions> :
        InternalCounted<Implementation<QueueOptions> >
    {
        Gtk::Table basic_options_page;
        Gtk::CheckButton preserve_world_box;
        Gtk::CheckButton fetch_only_box;
        Gtk::Label reinstall_label;
        Gtk::ComboBoxText reinstall_box;
        Gtk::Label upgrade_label;
        Gtk::ComboBoxText upgrade_box;
        Gtk::Label circular_label;
        Gtk::ComboBoxText circular_box;

        Gtk::Table dependencies_page;

        Gtk::Frame installed_deps_frame;
        Gtk::Table installed_deps_table;
        Gtk::Label installed_deps_pre_label;
        Gtk::Label installed_deps_runtime_label;
        Gtk::Label installed_deps_post_label;
        Gtk::ComboBoxText installed_deps_pre_box;
        Gtk::ComboBoxText installed_deps_runtime_box;
        Gtk::ComboBoxText installed_deps_post_box;

        Gtk::Frame uninstalled_deps_frame;
        Gtk::Table uninstalled_deps_table;
        Gtk::Label uninstalled_deps_pre_label;
        Gtk::Label uninstalled_deps_runtime_label;
        Gtk::Label uninstalled_deps_post_label;
        Gtk::ComboBoxText uninstalled_deps_pre_box;
        Gtk::ComboBoxText uninstalled_deps_runtime_box;
        Gtk::ComboBoxText uninstalled_deps_post_box;

        Implementation() :
            basic_options_page(3, 3, false),
            preserve_world_box("Preserve world"),
            fetch_only_box("Fetch only"),
            reinstall_label("Reinstall:", Gtk::ALIGN_LEFT),
            upgrade_label("Upgrade:", Gtk::ALIGN_LEFT),
            circular_label("Circular dependencies:", Gtk::ALIGN_LEFT),
            dependencies_page(2, 1, false),
            installed_deps_frame("Installed packages"),
            installed_deps_table(2, 3, false),
            installed_deps_pre_label("Pre:", Gtk::ALIGN_LEFT),
            installed_deps_runtime_label("Runtime:", Gtk::ALIGN_LEFT),
            installed_deps_post_label("Post:", Gtk::ALIGN_LEFT),
            uninstalled_deps_frame("Uninstalled packages"),
            uninstalled_deps_table(2, 3, false),
            uninstalled_deps_pre_label("Pre:", Gtk::ALIGN_LEFT),
            uninstalled_deps_runtime_label("Runtime:", Gtk::ALIGN_LEFT),
            uninstalled_deps_post_label("Post:", Gtk::ALIGN_LEFT)
        {
        }
    };
}

namespace
{
    void
    populate_deps_box(Gtk::ComboBoxText & box, const std::string & def)
    {
        box.append_text("Pre");
        box.append_text("Pre or post");
        box.append_text("Post");
        box.append_text("Try post");
        box.append_text("Discard");
        box.set_active_text(def);
    }
}

QueueOptions::QueueOptions() :
    PrivateImplementationPattern<QueueOptions>(new Implementation<QueueOptions>)
{
    append_page(_imp->basic_options_page, "Basic Options");

    _imp->basic_options_page.set_col_spacings(5);
    _imp->basic_options_page.set_row_spacings(5);
    _imp->basic_options_page.set_border_width(5);

    _imp->basic_options_page.attach(_imp->preserve_world_box, 0, 1, 0, 1,
            Gtk::EXPAND | Gtk::FILL, Gtk::AttachOptions(0));
    _imp->basic_options_page.attach(_imp->fetch_only_box, 0, 1, 1, 2,
            Gtk::EXPAND | Gtk::FILL, Gtk::AttachOptions(0));

    _imp->basic_options_page.attach(_imp->reinstall_label, 1, 2, 0, 1,
            Gtk::EXPAND | Gtk::FILL, Gtk::AttachOptions(0));
    _imp->basic_options_page.attach(_imp->upgrade_label, 1, 2, 1, 2,
            Gtk::EXPAND | Gtk::FILL, Gtk::AttachOptions(0));
    _imp->basic_options_page.attach(_imp->circular_label, 1, 2, 2, 3,
            Gtk::EXPAND | Gtk::FILL, Gtk::AttachOptions(0));
    _imp->basic_options_page.attach(_imp->reinstall_box, 2, 3, 0, 1,
            Gtk::EXPAND | Gtk::FILL, Gtk::AttachOptions(0));
    _imp->basic_options_page.attach(_imp->upgrade_box, 2, 3, 1, 2,
            Gtk::EXPAND | Gtk::FILL, Gtk::AttachOptions(0));
    _imp->basic_options_page.attach(_imp->circular_box, 2, 3, 2, 3,
            Gtk::EXPAND | Gtk::FILL, Gtk::AttachOptions(0));

    _imp->reinstall_box.append_text("Never");
    _imp->reinstall_box.append_text("Always");
    _imp->reinstall_box.append_text("If USE changed");
    _imp->reinstall_box.set_active_text("Never");

    _imp->circular_box.append_text("Error");
    _imp->circular_box.append_text("Discard");
    _imp->circular_box.set_active_text("Error");

    _imp->upgrade_box.append_text("Always");
    _imp->upgrade_box.append_text("As needed");
    _imp->upgrade_box.set_active_text("Always");

    append_page(_imp->dependencies_page, "Dependencies");
    _imp->dependencies_page.attach(_imp->uninstalled_deps_frame, 0, 1, 0, 1);
    _imp->dependencies_page.attach(_imp->installed_deps_frame, 1, 2, 0, 1);

    _imp->uninstalled_deps_frame.add(_imp->uninstalled_deps_table);
    _imp->uninstalled_deps_frame.set_border_width(5);
    _imp->installed_deps_frame.add(_imp->installed_deps_table);
    _imp->installed_deps_frame.set_border_width(5);

    _imp->uninstalled_deps_table.set_row_spacings(5);
    _imp->uninstalled_deps_table.set_col_spacings(5);
    _imp->uninstalled_deps_table.set_border_width(5);
    _imp->installed_deps_table.set_row_spacings(5);
    _imp->installed_deps_table.set_col_spacings(5);
    _imp->installed_deps_table.set_border_width(5);

    _imp->uninstalled_deps_table.attach(_imp->uninstalled_deps_pre_label, 0, 1, 0, 1,
            Gtk::EXPAND | Gtk::FILL, Gtk::AttachOptions(0));
    _imp->uninstalled_deps_table.attach(_imp->uninstalled_deps_runtime_label, 0, 1, 1, 2,
            Gtk::EXPAND | Gtk::FILL, Gtk::AttachOptions(0));
    _imp->uninstalled_deps_table.attach(_imp->uninstalled_deps_post_label, 0, 1, 2, 3,
            Gtk::EXPAND | Gtk::FILL, Gtk::AttachOptions(0));

    _imp->installed_deps_table.attach(_imp->installed_deps_pre_label, 0, 1, 0, 1,
            Gtk::EXPAND | Gtk::FILL, Gtk::AttachOptions(0));
    _imp->installed_deps_table.attach(_imp->installed_deps_runtime_label, 0, 1, 1, 2,
            Gtk::EXPAND | Gtk::FILL, Gtk::AttachOptions(0));
    _imp->installed_deps_table.attach(_imp->installed_deps_post_label, 0, 1, 2, 3,
            Gtk::EXPAND | Gtk::FILL, Gtk::AttachOptions(0));

    _imp->uninstalled_deps_table.attach(_imp->uninstalled_deps_pre_box, 1, 2, 0, 1,
            Gtk::EXPAND | Gtk::FILL, Gtk::AttachOptions(0));
    _imp->uninstalled_deps_table.attach(_imp->uninstalled_deps_runtime_box, 1, 2, 1, 2,
            Gtk::EXPAND | Gtk::FILL, Gtk::AttachOptions(0));
    _imp->uninstalled_deps_table.attach(_imp->uninstalled_deps_post_box, 1, 2, 2, 3,
            Gtk::EXPAND | Gtk::FILL, Gtk::AttachOptions(0));

    _imp->installed_deps_table.attach(_imp->installed_deps_pre_box, 1, 2, 0, 1,
            Gtk::EXPAND | Gtk::FILL, Gtk::AttachOptions(0));
    _imp->installed_deps_table.attach(_imp->installed_deps_runtime_box, 1, 2, 1, 2,
            Gtk::EXPAND | Gtk::FILL, Gtk::AttachOptions(0));
    _imp->installed_deps_table.attach(_imp->installed_deps_post_box, 1, 2, 2, 3,
            Gtk::EXPAND | Gtk::FILL, Gtk::AttachOptions(0));

    populate_deps_box(_imp->installed_deps_pre_box, "Discard");
    populate_deps_box(_imp->installed_deps_runtime_box, "Try post");
    populate_deps_box(_imp->installed_deps_post_box, "Try post");

    populate_deps_box(_imp->uninstalled_deps_pre_box, "Pre");
    populate_deps_box(_imp->uninstalled_deps_runtime_box, "Pre or post");
    populate_deps_box(_imp->uninstalled_deps_post_box, "Post");
}

QueueOptions::~QueueOptions()
{
}

