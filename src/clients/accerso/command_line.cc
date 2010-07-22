/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010 Ciaran McCreesh
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

#include "command_line.hh"
#include <paludis/util/singleton-impl.hh>

using namespace paludis;

template class Singleton<CommandLine>;

CommandLine::CommandLine() :
    ArgsHandler(),

    action_args(main_options_section(), "Actions",
            "Selects which basic action to perform. Exactly one action should "
            "be specified."),
    a_fetch(&action_args,     "fetch",        'f',  "Fetch necessary distfiles", false),
    a_version(&action_args,   "version",      'V',  "Display program version", false),
    a_help(&action_args,      "help",         'h',  "Display program help", false),

    general_args(main_options_section(), "General options",
            "Options which are relevant for most or all actions."),
    a_log_level(&general_args, "log-level",  '\0'),
    a_no_colour(&general_args, "no-colour", '\0', "Do not use colour", false),
    a_no_color(&a_no_colour, "no-color"),
    a_force_colour(&general_args, "force-colour", '\0', "Force the use of colour", false),
    a_force_color(&a_force_colour, "force-color"),
    a_repository_directory(&general_args, "repository-dir", 'D',
            "Where to find the repository (default: current directory)"),
    a_download_directory(&general_args, "download-dir", 'd',
            "Where to place downloaded files"),
    a_master_repository_name(&general_args, "master-repository-name", '\0',
            "Use the specified name for the master repository. Specify the location using --extra-repository-dir. "
            "Only for repositories with no metadata/layout.conf."),
    a_extra_repository_dir(&general_args, "extra-repository-dir", '\0',
            "Also include the repository at this location. May be specified multiple times, in creation order."),
    a_write_cache_dir(&general_args, "write-cache-dir", '\0',
            "Use a subdirectory named for the repository name under the specified directory for repository write cache"),
    a_report_file(&general_args, "report-file", 'r',
            "Write report to the specified file, rather than stdout")
{
    add_usage_line("--fetch");

    add_description_line("accerso is configured purely from the command line. It does not use any user "
            "configuration files.");

    add_environment_variable("ACCERSO_OPTIONS", "Default command-line options.");
}

std::string
CommandLine::app_name() const
{
    return "accerso";
}

std::string
CommandLine::app_synopsis() const
{
    return "Mirror client for Paludis";
}

std::string
CommandLine::app_description() const
{
    return
        "accerso is a mirror client for Paludis. It fetches every distfile for every package in the "
        "given repository and produces a report of any failures.";
}

CommandLine::~CommandLine()
{
}


