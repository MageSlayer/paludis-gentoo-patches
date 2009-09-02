/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009 Ciaran McCreesh
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
#include <paludis/util/instantiation_policy-impl.hh>

template class paludis::InstantiationPolicy<QualudisCommandLine, paludis::instantiation_method::SingletonTag>;

QualudisCommandLine::QualudisCommandLine() :
    ArgsHandler(),

    action_args(main_options_section(), "Actions",
            "Selects which basic action to perform. Up to one action should "
            "be specified. If no action is specified, the directories specifed "
            "on the command line (or, if none, the current directory) are "
            "checked."),
    a_version(&action_args,  "version",      'V', "Display program version", false),
    a_help(&action_args,     "help",         'h', "Display program help", false),

    check_options(main_options_section(), "Options for general checks",
            "Options relevant for the --owner actions."),
    a_log_level(&check_options, "log-level", 'L'),

    a_message_level(&check_options, "message-level", 'M', "Specify the message level",
            paludis::args::EnumArg::EnumArgOptions
            ("debug",  "Show debug and upwards")
            ("maybe",  "Show maybe and upwards")
            ("minor",  "Show minor and upwards")
            ("normal", "Show normal and upwards")
            ("severe", "Show severe and upwards"),
            "maybe"),
    message_level(paludis::qaml_maybe),

    a_show_associated_keys(&check_options, "show-associated-keys", '\0', "Show the contents of relevant metadata keys",
            paludis::args::EnumArg::EnumArgOptions
            ("never",  "Never show metadata keys")
            ("once",   "Show each metadata key at most once")
            ("always", "Always show metadata keys"),
            "once"),

    a_repository_directory(&check_options, "repository-dir", 'D',
            "Where to find the repository (default: detected from ./ or ../ or ../..)"),

    configuration_options(main_options_section(), "Configuration options",
            "Options that control general configuration."),
    a_write_cache_dir(&configuration_options, "write-cache-dir", '\0',
            "Use a subdirectory named for the repository name under the specified directory for repository write cache"),
    a_master_repository_name(&configuration_options, "master-repository-name", '\0',
            "Use the specified name for the master repository. Specify the location using --extra-repository-dir. "
            "Only for repositories with no metadata/layout.conf."),
    a_extra_repository_dir(&configuration_options, "extra-repository-dir", '\0',
            "Also include the repository at this location. May be specified multiple times, in creation order."),
    a_use_repository_cache(&configuration_options, "use-repository-cache", '\0',
            "Use the repository's metadata cache, if available (faster, but may miss certain errors)", true)
{
    add_usage_line("[ options ] [ directories ... ]");

    add_description_line("qualudis is configured purely from the command line. It does not use any user "
            "configuration files.");

    add_environment_variable("QUALUDIS_OPTIONS", "Default command-line options.");
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
        "many common and potential mistakes and displays a summary of its findings.";
}
