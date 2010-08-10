/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2009, 2010 Ciaran McCreesh
 * Copyright (c) 2007 David Leverton
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

#include <paludis/args/args_option.hh>
#include <paludis/args/legacy/deps_option_arg.hh>
#include <paludis/args/args_group.hh>
#include <paludis/legacy/dep_list.hh>
#include <paludis/legacy/install_task.hh>

namespace paludis
{
    namespace args
    {
        /**
         * The standard dep command line arguments.
         *
         * \since 0.26
         * \ingroup g_args
         * \nosubgrouping
         */
        class PALUDIS_VISIBLE DepListArgsGroup : public ArgsGroup
        {
            public:
                /// Constructor.
                DepListArgsGroup(ArgsSection *);

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

                /**
                 * Populate a DepListOptions from our values.
                 */
                void populate_dep_list_options(const paludis::Environment *, DepListOptions &) const;

                /**
                 * Populate an InstallTask from our values.
                 */
                void populate_install_task(const paludis::Environment *, InstallTask &) const;

                /**
                 * Fetch a fragment for Environment::paludis_command for our
                 * values.
                 */
                std::string paludis_command_fragment() const;

                /**
                 * Fetch a fragment for a resume command for our values.
                 */
                std::string resume_command_fragment(const InstallTask &) const;
        };
    }
}

#endif

