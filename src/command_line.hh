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

#ifndef PALUDIS_GUARD_SRC_COMMAND_LINE_HH
#define PALUDIS_GUARD_SRC_COMMAND_LINE_HH 1

#include <paludis/args/args.hh>
#include <paludis/util/instantiation_policy.hh>

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

        /// --uninstall
        paludis::args::SwitchArg a_uninstall;

        /// --sync
        paludis::args::SwitchArg a_sync;

        /// --list-repositories
        paludis::args::SwitchArg a_list_repositories;

        /// --list-categories
        paludis::args::SwitchArg a_list_categories;

        /// --list-packages
        paludis::args::SwitchArg a_list_packages;

        /// --contents
        paludis::args::SwitchArg a_contents;

        /// --owner
        paludis::args::SwitchArg a_owner;

        /// --version
        paludis::args::SwitchArg a_version;

        /// --help
        paludis::args::SwitchArg a_help;

        /// Action arguments (internal).
        paludis::args::ArgsGroup action_args_internal;

        /// --has-version
        paludis::args::SwitchArg a_has_version;

        /// --best-version
        paludis::args::SwitchArg a_best_version;

        /// --environment-variable
        paludis::args::SwitchArg a_environment_variable;

        /// --list-sync-protocols
        paludis::args::SwitchArg a_list_sync_protocols;

        /// --list-repository-formats
        paludis::args::SwitchArg a_list_repository_formats;

        /// --list-dep-tag-categories
        paludis::args::SwitchArg a_list_dep_tag_categories;

        /// --list-vulnerabilities
        paludis::args::SwitchArg a_list_vulnerabilities;

        /// --update-news
        paludis::args::SwitchArg a_update_news;

        ///}

        /// \name General arguments
        ///{

        /// General arguments.
        paludis::args::ArgsGroup general_args;

        /// --log-level
        paludis::args::EnumArg a_log_level;

        /// --no-colour
        paludis::args::SwitchArg a_no_colour;

        /// --no-color
        paludis::args::AliasArg a_no_color;

        /// --config-suffix
        paludis::args::StringArg a_config_suffix;

        ///}

        /// \name Query arguments
        ///{

        /// Query arguments.
        paludis::args::ArgsGroup query_args;

        /// --show-slot
        paludis::args::SwitchArg a_show_slot;

        /// --show-deps
        paludis::args::SwitchArg a_show_deps;

        /// --show-metadata
        paludis::args::SwitchArg a_show_metadata;

        /// }

        /// \name (Un)Install arguments
        /// {

        /// Install arguments.
        paludis::args::ArgsGroup install_args;

        /// --pretend
        paludis::args::SwitchArg a_pretend;

        /// --preserve-world
        paludis::args::SwitchArg a_preserve_world;

        /// --no-config-protection
        paludis::args::SwitchArg a_no_config_protection;

        /// --fetch
        paludis::args::SwitchArg a_fetch;

        /// }

        /// \name DepList behaviour arguments
        /// {

        /// DepList behaviour arguments.
        paludis::args::ArgsGroup dl_args;

        /// --dl-rdepend-post
        paludis::args::EnumArg a_dl_rdepend_post;

        /// --dl-drop-self-circular
        paludis::args::SwitchArg a_dl_drop_self_circular;

        /// --dl-drop-circular
        paludis::args::SwitchArg a_dl_drop_circular;

        /// --dl-drop-all
        paludis::args::SwitchArg a_dl_drop_all;

        /// --dl-ignore-installed
        paludis::args::SwitchArg a_dl_ignore_installed;

        /// --dl-no-recursive-deps
        paludis::args::SwitchArg a_dl_no_recursive_deps;

        /// --dl-max-stack-depth
        paludis::args::IntegerArg a_dl_max_stack_depth;

        /// --dl-no-unnecessary-upgrades
        paludis::args::SwitchArg a_dl_no_unnecessary_upgrades;

        /// }

        /// \name List arguments
        /// {

        /// List arguments.
        paludis::args::ArgsGroup list_args;

        /// --repository
        paludis::args::StringSetArg a_repository;

        /// --category
        paludis::args::StringSetArg a_category;

        /// --package
        paludis::args::StringSetArg a_package;

        /// }

        /// \name Owner arguments
        /// {

        /// Owner arguments.
        paludis::args::ArgsGroup owner_args;

        /// --full-match
        paludis::args::SwitchArg a_full_match;

        /// }
};

/**
 * Show the help message.
 */
struct DoHelp
{
    const std::string message;

    DoHelp(const std::string & m = "") :
        message(m)
    {
    }
};

#endif
