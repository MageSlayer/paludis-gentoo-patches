/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
 * Copyright (c) 2007 David Leverton <levertond@googlemail.com>
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

#ifndef PALUDIS_GUARD_SRC_COMMON_ARGS_DEP_LIST_ARGS_HH
#define PALUDIS_GUARD_SRC_COMMON_ARGS_DEP_LIST_ARGS_HH 1

#include <paludis/args/args.hh>
#include <paludis/args/deps_option_arg.hh>
#include <paludis/dep_list.hh>
#include <paludis/install_task.hh>

namespace paludis
{
    namespace args
    {
        class PALUDIS_VISIBLE DepListArgsGroup : public ArgsGroup
        {
            public:
                /// Constructor.
                DepListArgsGroup(ArgsHandler *);

                /// Destructor
                ~DepListArgsGroup();

                /// \name DepList behaviour arguments
                /// {

                paludis::args::EnumArg dl_reinstall;
                paludis::args::EnumArg dl_reinstall_scm;
                paludis::args::EnumArg dl_reinstall_targets;
                paludis::args::EnumArg dl_upgrade;
                paludis::args::EnumArg dl_new_slots;
                paludis::args::EnumArg dl_downgrade;

                paludis::args::DepsOptionArg dl_deps_default;

                paludis::args::DepsOptionArg dl_installed_deps_pre;
                paludis::args::DepsOptionArg dl_installed_deps_runtime;
                paludis::args::DepsOptionArg dl_installed_deps_post;

                paludis::args::DepsOptionArg dl_uninstalled_deps_pre;
                paludis::args::DepsOptionArg dl_uninstalled_deps_runtime;
                paludis::args::DepsOptionArg dl_uninstalled_deps_post;
                paludis::args::DepsOptionArg dl_uninstalled_deps_suggested;

                paludis::args::EnumArg dl_suggested;
                paludis::args::EnumArg dl_circular;
                paludis::args::EnumArg dl_blocks;
                paludis::args::StringSetArg dl_override_masks;

                paludis::args::EnumArg dl_fall_back;

                /// }

                void populate_dep_list_options(const paludis::Environment *, DepListOptions &) const;
                void populate_install_task(const paludis::Environment *, InstallTask &) const;

                std::string paludis_command_fragment() const;
                std::string resume_command_fragment(const InstallTask &) const;
        };
    }
}

#endif

