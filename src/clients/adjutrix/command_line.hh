/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_SRC_ARCHTOOL_COMMAND_LINE_HH
#define PALUDIS_GUARD_SRC_ARCHTOOL_COMMAND_LINE_HH 1

#include <paludis/args/args.hh>
#include <paludis/util/singleton.hh>
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

        paludis::args::ArgsGroup tree_action_args;
        paludis::args::SwitchArg a_find_stable_candidates;
        paludis::args::SwitchArg a_find_dropped_keywords;
        paludis::args::SwitchArg a_find_insecure_packages;
        paludis::args::SwitchArg a_find_unused_packages;
        paludis::args::SwitchArg a_keywords_graph;
        paludis::args::SwitchArg a_reverse_deps;
        paludis::args::SwitchArg a_what_needs_keywording;

        paludis::args::ArgsGroup profile_action_args;
        paludis::args::SwitchArg a_display_default_system_resolution;

        paludis::args::ArgsGroup downgrade_check_args;
        paludis::args::SwitchArg a_build_downgrade_check_list;
        paludis::args::SwitchArg a_downgrade_check;

        paludis::args::ArgsGroup general_action_args;
        paludis::args::SwitchArg a_version;
        paludis::args::SwitchArg a_help;

        /// \name General arguments
        ///{

        /// General arguments.
        paludis::args::ArgsGroup general_args;

        paludis::args::LogLevelArg a_log_level;
        paludis::args::SwitchArg a_no_colour;
        paludis::args::AliasArg a_no_color;
        paludis::args::SwitchArg a_force_colour;
        paludis::args::AliasArg a_force_color;
        paludis::args::SwitchArg a_no_suggestions;

        paludis::args::StringArg a_repository_directory;

        ///}


        ///\name Tree arguments
        ///\{

        paludis::args::ArgsGroup tree_args;

        paludis::args::StringSetArg a_category;
        paludis::args::StringSetArg a_package;

        ///\}

        ///\name Profile arguments
        ///\{

        paludis::args::ArgsGroup profile_args;

        paludis::args::StringArg a_profile;
        paludis::args::SwitchArg a_unstable;

        ///\}

        ///\name Configuration options
        ///\{

        paludis::args::ArgsGroup configuration_options;

        paludis::args::StringArg a_write_cache_dir;
        paludis::args::StringArg a_master_repository_name;
        paludis::args::StringSequenceArg a_extra_repository_dir;

        ///\}
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
