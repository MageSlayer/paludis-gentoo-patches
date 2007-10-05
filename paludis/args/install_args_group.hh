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

#ifndef PALUDIS_GUARD_SRC_COMMON_ARGS_INSTALL_ARGS_HH
#define PALUDIS_GUARD_SRC_COMMON_ARGS_INSTALL_ARGS_HH 1

#include <paludis/util/set.hh>

#include <paludis/args/args.hh>
#include <paludis/args/checks_arg.hh>
#include <paludis/args/debug_build_arg.hh>
#include <paludis/dep_list-fwd.hh>
#include <paludis/install_task.hh>

namespace paludis
{
    namespace args
    {
        class PALUDIS_VISIBLE InstallArgsGroup : public ArgsGroup
        {
            public:
                /// Constructor.
                InstallArgsGroup(ArgsHandler *, const std::string &, const std::string &);

                /// Destructor
                ~InstallArgsGroup();

                /// \name (Un)Install arguments
                /// {

                /// --pretend
                paludis::args::SwitchArg a_pretend;

                /// --destinations
                paludis::args::StringSetArg a_destinations;

                /// --preserve-world
                paludis::args::SwitchArg a_preserve_world;

                /// --add-to-world-spec
                paludis::args::StringArg a_add_to_world_spec;

                /// --no-config-protection
                paludis::args::SwitchArg a_no_config_protection;

                /// --debug-build
                paludis::args::DebugBuildArg a_debug_build;

                /// --checks
                paludis::args::ChecksArg a_checks;

                /// --fetch
                paludis::args::SwitchArg a_fetch;

                /// --no-safe-resume
                paludis::args::SwitchArg a_no_safe_resume;

                /// --show-reasons
                paludis::args::EnumArg a_show_reasons;

                /// --show-use-descriptions
                paludis::args::EnumArg a_show_use_descriptions;

                /// --continue-on-failure
                paludis::args::EnumArg a_continue_on_faillure;

                /// }

                void populate_dep_list_options(const Environment *, DepListOptions &) const;
                tr1::shared_ptr<const DestinationsSet> destinations(Environment *) const;
                void populate_install_task(const Environment *, InstallTask &) const;

                bool want_full_install_reasons() const;
                bool want_install_reasons() const;
                bool want_tags_summary() const;

                bool want_use_summary() const;
                bool want_unchanged_use_flags() const;
                bool want_changed_use_flags() const;
                bool want_new_use_flags() const;

                std::string paludis_command_fragment() const;
                std::string resume_command_fragment(const InstallTask &) const;
        };
    }
}

#endif
