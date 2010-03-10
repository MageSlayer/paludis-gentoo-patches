/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009 Kim Højgaard-Hansen
 * Copyright (c) 2007, 2008, 2009 Ciaran McCreesh
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
#include <paludis/util/instantiation_policy-impl.hh>

using namespace paludis;

template class InstantiationPolicy<CommandLine, instantiation_method::SingletonTag>;

CommandLine::CommandLine() :
    ArgsHandler(),

    action_args(main_options_section(), "Actions",
            "Selects which basic action to perform. Exactly one action should "
            "be specified."),
    a_manifest(&action_args,     "manifest",        'm',  "Create manifest file", false),
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
            "Write report to the specified file, rather than stdout"),
    a_override(&general_args, "override",  '\0', "Override Manifest if distfile's checksum does not match with it", false),
    tree_args(main_options_section(), "Tree action options",
                "Options which are relevant for tree actions."),
    a_category(&tree_args,   "category",   'C',
            "Matches with this category name only (may be specified multiple times)",
            args::StringSetArg::StringSetArgOptions(), &CategoryNamePartValidator::validate),
    a_package(&tree_args,    "package",    'P',
            "Matches with this package name only (may be specified multiple times)",
            args::StringSetArg::StringSetArgOptions(), &PackageNamePartValidator::validate)
{
    add_usage_line("--manifest");

    add_description_line("appareo is configured purely from the command line. It does not use any user "
            "configuration files.");
}

std::string
CommandLine::app_name() const
{
    return "appareo";
}

std::string
CommandLine::app_synopsis() const
{
    return "Manifest client for Paludis";
}

std::string
CommandLine::app_description() const
{
    return
        "Appareo is a manifest creation client for Paludis. It fetches the distfile(s) for the specified package(s), "
        "category or categories or repository, creates manifest files for these and produces a report of any failures.";
}

CommandLine::~CommandLine()
{
}


