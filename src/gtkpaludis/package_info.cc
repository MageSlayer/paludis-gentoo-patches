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

#include "package_info.hh"
#include "package_overview.hh"
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>

using namespace gtkpaludis;
using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<PackageInfo> :
        InternalCounted<Implementation<PackageInfo> >
    {
        Gtk::HBox overview_page;
        Gtk::ScrolledWindow overview_page_main;
        Gtk::VButtonBox overview_page_buttons;

        Gtk::Button install_button;
        Gtk::Button uninstall_button;

        Gtk::ScrolledWindow metadata_page;

        PackageOverview overview;

        Implementation() :
            install_button("Install"),
            uninstall_button("Uninstall")
        {
        }
    };
}

PackageInfo::PackageInfo() :
    PrivateImplementationPattern<PackageInfo>(new Implementation<PackageInfo>)
{
    append_page(_imp->overview_page, "Overview");
    append_page(_imp->metadata_page, "Metadata");

    _imp->overview_page.pack_start(_imp->overview_page_main);
    _imp->overview_page.pack_end(_imp->overview_page_buttons, Gtk::PACK_SHRINK);

    _imp->overview_page_main.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    _imp->overview_page_main.add(_imp->overview);

    _imp->overview_page_buttons.set_border_width(5);
    _imp->overview_page_buttons.set_spacing(5);
    _imp->overview_page_buttons.set_layout(Gtk::BUTTONBOX_START);
    _imp->overview_page_buttons.add(_imp->install_button);
    _imp->overview_page_buttons.add(_imp->uninstall_button);
}

PackageInfo::~PackageInfo()
{
}

void
PackageInfo::populate(const QualifiedPackageName & n)
{
    _imp->overview.populate(n);
}

