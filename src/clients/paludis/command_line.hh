/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2010 Ciaran McCreesh
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
#include <paludis/util/singleton.hh>
#include <paludis/args/dep_list_args_group.hh>
#include <paludis/args/install_args_group.hh>
#include <paludis/args/log_level_arg.hh>

/** \file
 * Declarations for the CommandLine class.
 */

/**
 * Our command line.
 */
class CommandLine :
    public paludis::args::ArgsHandler,
    public paludis::Singleton<CommandLine>
{
    friend class paludis::Singleton<CommandLine>;

    private:
        /// Constructor.
        CommandLine();

        /// Destructor.
        ~CommandLine();

    public:
        ///\name Program information
        ///\{

        virtual std::string app_name() const;
        virtual std::string app_synopsis() const;
        virtual std::string app_description() const;

        ///\}

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

        /// --uninstall-unused
        paludis::args::SwitchArg a_uninstall_unused;

        /// --sync
        paludis::args::SwitchArg a_sync;

        /// --report
        paludis::args::SwitchArg a_report;

        /// --contents
        paludis::args::SwitchArg a_contents;

        /// --executables
        paludis::args::SwitchArg a_executables;

        /// --owner
        paludis::args::SwitchArg a_owner;

        /// --config
        paludis::args::SwitchArg a_config;

        /// --version
        paludis::args::SwitchArg a_version;

        /// --info
        paludis::args::SwitchArg a_info;

        /// --help
        paludis::args::SwitchArg a_help;

        /// Action arguments (internal).
        paludis::args::ArgsGroup action_args_internal;

        /// --has-version
        paludis::args::SwitchArg a_has_version;

        /// --best-version
        paludis::args::SwitchArg a_best_version;

        /// --match
        paludis::args::SwitchArg a_match;

        /// --environment-variable
        paludis::args::SwitchArg a_environment_variable;

        /// --configuration-variable
        paludis::args::SwitchArg a_configuration_variable;

        /// --list-repositories
        paludis::args::SwitchArg a_list_repositories;

        /// --list-categories
        paludis::args::SwitchArg a_list_categories;

        /// --list-packages
        paludis::args::SwitchArg a_list_packages;

        /// --list-sets
        paludis::args::SwitchArg a_list_sets;

        /// --list-sync-protocols
        paludis::args::SwitchArg a_list_sync_protocols;

        /// --list-repository-formats
        paludis::args::SwitchArg a_list_repository_formats;

        /// --regenerate-installed-cache
        paludis::args::SwitchArg a_regenerate_installed_cache;

        /// --regenerate-installable-cache
        paludis::args::SwitchArg a_regenerate_installable_cache;

        ///}

        /// \name General arguments
        ///{

        /// General arguments.
        paludis::args::ArgsGroup general_args;

        /// --log-level
        paludis::args::LogLevelArg a_log_level;

        /// --no-colour
        paludis::args::SwitchArg a_no_colour;

        /// --no-color
        paludis::args::AliasArg a_no_color;

        /// --force-colour
        paludis::args::SwitchArg a_force_colour;

        /// --force-color
        paludis::args::AliasArg a_force_color;

        /// --no-suggestions
        paludis::args::SwitchArg a_no_suggestions;

        /// --environment
        paludis::args::StringArg a_environment;

        /// --resume-command-template
        paludis::args::StringArg a_resume_command_template;

        /// --multitask
        paludis::args::SwitchArg a_multitask;

        /// --compact
        paludis::args::SwitchArg a_compact;

        ///}

        /// \name Query arguments
        ///{

        /// Query arguments.
        paludis::args::ArgsGroup query_args;

        /// --show-deps
        paludis::args::SwitchArg a_show_deps;

        /// --show-authors
        paludis::args::SwitchArg a_show_authors;

        /// --show-metadata
        paludis::args::SwitchArg a_show_metadata;

        /// }

        /// \name (Un)Install arguments
        paludis::args::InstallArgsGroup install_args;

        paludis::args::StringArg a_serialised;

        /// \name Uninstall arguments
        ///\{

        paludis::args::ArgsGroup uninstall_args;

        /// --with-unused-dependencies
        paludis::args::SwitchArg a_with_unused_dependencies;

        /// --with-dependencies
        paludis::args::SwitchArg a_with_dependencies;

        /// --all-versions
        paludis::args::SwitchArg a_all_versions;

        /// --permit-unsafe-uninstalls
        paludis::args::SwitchArg a_permit_unsafe_uninstalls;

        ///\}

        /// DepList behaviour arguments.
        paludis::args::DepListArgsGroup dl_args;

        /// \name List arguments
        /// {

        /// List arguments.
        paludis::args::ArgsGroup list_args;

        /// --repository
        paludis::args::StringSetArg a_repository;

        /// --repository-format
        paludis::args::StringSetArg a_repository_format;

        /// --category
        paludis::args::StringSetArg a_category;

        /// --package
        paludis::args::StringSetArg a_package;

        /// --set
        paludis::args::StringSetArg a_set;

        /// }

        /// \name Owner arguments
        /// {

        /// Owner arguments.
        paludis::args::ArgsGroup owner_args;

        /// --full-match
        paludis::args::SwitchArg a_full_match;

        /// }
};

#endif
