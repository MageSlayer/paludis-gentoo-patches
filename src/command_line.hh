/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

#ifndef PALUDIS_GUARD_SRC_COMMAND_LINE_HH
#define PALUDIS_GUARD_SRC_COMMAND_LINE_HH 1

#include <paludis/args/args.hh>
#include <paludis/instantiation_policy.hh>

/** \file
 * Declarations for the CommandLine class.
 */

/**
 * Our command line.
 */
class CommandLine :
    public paludis::args::ArgsHandler,
    public paludis::InstantiationPolicy<CommandLine, paludis::instantiation_method::SingletonAsNeededTag>
{
    friend class paludis::InstantiationPolicy<CommandLine, paludis::instantiation_method::SingletonAsNeededTag>;

    private:
        /// Constructor.
        CommandLine();

        /// Destructor.
        ~CommandLine();

    public:
        /// \name Action arguments
        ///{

        /// Action arguments.
        paludis::args::ArgsGroup action_args;

        /// --query
        paludis::args::SwitchArg a_query;

        /// --install
        paludis::args::SwitchArg a_install;

        /// --list-repositories
        paludis::args::SwitchArg a_list_repositories;

        /// --list-categories
        paludis::args::SwitchArg a_list_categories;

        /// --version
        paludis::args::SwitchArg a_version;

        /// --help
        paludis::args::SwitchArg a_help;

        ///}

        /// \name General arguments
        ///{

        /// General arguments.
        paludis::args::ArgsGroup general_args;

        /// --no-colour
        paludis::args::SwitchArg a_no_colour;

        /// --no-color
        paludis::args::AliasArg a_no_color;

        ///}

        /// \name Query arguments
        ///{

        /// Query arguments.
        paludis::args::ArgsGroup query_args;

        /// --show-slot
        paludis::args::SwitchArg a_show_slot;

        /// --show-license
        paludis::args::SwitchArg a_show_license;

        /// --show-licence
        paludis::args::AliasArg a_show_licence;

        /// --show-deps
        paludis::args::SwitchArg a_show_deps;

        /// --show-metadata
        paludis::args::SwitchArg a_show_metadata;

        /// }

        /// \name Install arguments
        /// {

        /// Install arguments.
        paludis::args::ArgsGroup install_args;

        /// --pretend
        paludis::args::SwitchArg a_pretend;

        /// }

        /// \name DepList behaviour arguments
        /// {

        /// DepList behaviour arguments.
        paludis::args::ArgsGroup dl_args;

        /// --dl-rdepend-post
        paludis::args::EnumArg a_dl_rdepend_post;

        /// --dl-drop-self-circular
        paludis::args::SwitchArg a_dl_drop_self_circular;

        /// --dl-dont-ignore-the-frickin-patch-dep
        paludis::args::SwitchArg a_dl_patch_dep;

        /// --dl-drop-circular
        paludis::args::SwitchArg a_dl_drop_circular;

        /// --dl-drop-all
        paludis::args::SwitchArg a_dl_drop_all;

        /// --dl-ignore-installed
        paludis::args::SwitchArg a_dl_ignore_installed;

        /// --dl-recursive-deps
        paludis::args::SwitchArg a_dl_recursive_deps;

        /// --dl-max-stack-depth
        paludis::args::IntegerArg a_dl_max_stack_depth;

        /// }
};

#endif
