/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include "qualudis_command_line.hh"

QualudisCommandLine::QualudisCommandLine() :
    ArgsHandler(),

    action_args(this, "Actions",
            "Selects which basic action to perform. Up to one action should "
            "be specified. If no action is specified, the directories specifed "
            "on the command line (or, if none, the current directory) are "
            "checked."),
    a_describe(&action_args, "describe",     'd', "Describe checks"),
    a_version(&action_args,  "version",      'V', "Display program version"),
    a_help(&action_args,     "help",         'h', "Display program help"),

    check_options(this, "Options for general checks",
            "Options relevant for the --owner actions."),
    a_qa_checks(&check_options, "qa-check", 'c', "Only perform given check."),
    a_verbose(&check_options, "verbose", 'v', "Be verbose"),
    a_quiet(&check_options, "quiet", 'q', "Be quiet"),
    a_log_level(&check_options, "log-level", 'L', "Specify the log level",
            paludis::args::EnumArg::EnumArgOptions("debug", "Show debug output (noisy)")
            ("qa",      "Show QA messages and warnings only")
            ("warning", "Show warnings only")
            ("silent", "Suppress all log messages"),
            "warning"),

    a_message_level(&check_options, "message-level", 'M', "Specify the message level",
            paludis::args::EnumArg::EnumArgOptions("info", "Show info and upwards")
            ("minor", "Show minor and upwards")
            ("major", "Show major and upwards")
            ("fatal", "Show only fatals"),
            "info"),

    message_level(paludis::qa::qal_info),

    configuration_options(this, "Configuration options",
            "Options that control general configuration."),
    a_write_cache_dir(&configuration_options, "write-cache-dir", '\0',
            "Use a subdirectory named for the repository name under the specified directory for repository write cache")
{
    add_usage_line("[ options ] [ directories ... ]");
    add_environment_variable("QUALUDIS_OPTIONS", "Default command-line options.");
    add_environment_variable("PALUDIS_QA_DATA_DIR", "Where to look for QA data files.");
}

QualudisCommandLine::~QualudisCommandLine()
{
}

std::string
QualudisCommandLine::app_name() const
{
    return "qualudis";
}

std::string
QualudisCommandLine::app_synopsis() const
{
    return "A QA tool for ebuilds";
}

std::string
QualudisCommandLine::app_description() const
{
    return
        "qualudis is a QA assistant for ebuilds and ebuild repositories. It checks for "
        "many common and potential mistakes and displays a summary of its findings. It "
        "can also be used to commit changes to a repository's VCS.";
}
