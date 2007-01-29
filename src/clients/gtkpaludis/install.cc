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

#include "install.hh"
#include <paludis/environment/default/default_environment.hh>
#include <paludis/util/save.hh>

using namespace paludis;
using namespace gtkpaludis;

namespace paludis
{
    template<>
    struct Implementation<OurInstallTask>
    {
        OurInstallTask::Callbacks * callbacks;
    };
}

OurInstallTask::OurInstallTask() :
    InstallTask(DefaultEnvironment::get_instance(), DepListOptions()),
    PrivateImplementationPattern<OurInstallTask>(new Implementation<OurInstallTask>)
{
}

OurInstallTask::~OurInstallTask()
{
}

void
OurInstallTask::on_build_deplist_pre()
{
}

void
OurInstallTask::on_build_deplist_post()
{
}

void
OurInstallTask::on_not_continuing_due_to_errors()
{
}

void
OurInstallTask::on_build_cleanlist_pre(const DepListEntry &)
{
}

void
OurInstallTask::on_build_cleanlist_post(const DepListEntry &)
{
}


void
OurInstallTask::on_display_merge_list_pre()
{
}

void
OurInstallTask::on_display_merge_list_post()
{
}

void
OurInstallTask::on_display_merge_list_entry(const DepListEntry & e)
{
    if (! PrivateImplementationPattern<OurInstallTask>::_imp->callbacks)
        throw InternalError(PALUDIS_HERE, "_imp->callbacks is 0");

    PrivateImplementationPattern<OurInstallTask>::_imp->callbacks->display_entry(e);
}

void
OurInstallTask::on_fetch_all_pre()
{
}

void
OurInstallTask::on_fetch_pre(const DepListEntry &)
{
}

void
OurInstallTask::on_fetch_post(const DepListEntry &)
{
}

void
OurInstallTask::on_fetch_all_post()
{
}


void
OurInstallTask::on_install_all_pre()
{
}

void
OurInstallTask::on_install_pre(const DepListEntry &)
{
}

void
OurInstallTask::on_install_post(const DepListEntry &)
{
}

void
OurInstallTask::on_install_all_post()
{
}

void
OurInstallTask::on_no_clean_needed(const DepListEntry &)
{
}

void
OurInstallTask::on_clean_all_pre(const DepListEntry &,
    const PackageDatabaseEntryCollection &)
{
}

void
OurInstallTask::on_clean_pre(const DepListEntry &,
    const PackageDatabaseEntry &)
{
}

void
OurInstallTask::on_clean_post(const DepListEntry &,
    const PackageDatabaseEntry &)
{
}

void
OurInstallTask::on_clean_all_post(const DepListEntry &,
    const PackageDatabaseEntryCollection &)
{
}

void
OurInstallTask::on_update_world_pre()
{
}

void
OurInstallTask::on_update_world(const PackageDepAtom &)
{
}

void
OurInstallTask::on_update_world_skip(const PackageDepAtom &, const std::string &)
{
}

void
OurInstallTask::on_update_world_post()
{
}

void
OurInstallTask::on_preserve_world()
{
}

OurInstallTask::Callbacks::Callbacks()
{
}

OurInstallTask::Callbacks::~Callbacks()
{
}

void
OurInstallTask::execute(OurInstallTask::Callbacks * const c)
{
    Save<OurInstallTask::Callbacks *> save_callback(&PrivateImplementationPattern<OurInstallTask>::_imp->callbacks, c);
    InstallTask::execute();
}

