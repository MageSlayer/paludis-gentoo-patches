/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh
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
#include <paludis/name.hh>
#include <paludis/util/instantiation_policy-impl.hh>

using namespace paludis;

template class paludis::InstantiationPolicy<CommandLine, paludis::instantiation_method::SingletonTag>;

CommandLine::CommandLine() :
    ArgsHandler(),

    action_args(this, "Actions",
            "Selects which basic action to perform. Exactly one action should "
            "be specified."),
    a_search(&action_args,     "search",        's',  "Search for a package"),
    a_version(&action_args,   "version",      'V',  "Display program version"),
    a_help(&action_args,      "help",         'h',  "Display program help"),

    general_args(this, "General options",
            "Options that are relevant for most or all actions."),
    a_log_level(&general_args, "log-level",  '\0'),
    a_no_colour(&general_args, "no-colour", '\0', "Do not use colour"),
    a_no_color(&a_no_colour, "no-color"),
    a_environment(&general_args, "environment", 'E', "Environment specification (class:suffix, both parts optional)"),

    search_args(this, "Search options",
            "Options that are relevant for the search action."),
    a_matcher(&search_args, "matcher", 'm', "Which match algorithm to use",
            paludis::args::EnumArg::EnumArgOptions
            ("text",        "Simple text match")
            ("pcre",        "Regular expression match using pcre"),
            "text"),
    a_extractors(&search_args, "extractors", 'e', "Which extractors to use",
            paludis::args::StringSetArg::StringSetArgOptions
            ("description",   "Match against description (default)")
            ("name",          "Match against package name")
            ("homepage",      "Match against homepage")),

    a_repository(&search_args, "repository", '\0', "Matches with this repository name only",
            paludis::args::StringSetArg::StringSetArgOptions(), &paludis::RepositoryNameValidator::validate),
    a_repository_format(&search_args, "repository-format", '\0', "Matches with this repository format only"),
    a_category(&search_args,   "category",   '\0', "Matches with this category name only",
            paludis::args::StringSetArg::StringSetArgOptions(), &paludis::CategoryNamePartValidator::validate),
    a_package(&search_args,    "package",    '\0', "Matches with this package name only",
            paludis::args::StringSetArg::StringSetArgOptions(), &paludis::PackageNamePartValidator::validate),

    deprecated_args(this, "Deprecated options", "Deprecated options."),
    a_config_suffix(&deprecated_args, "config-suffix", 'c',
            "Replaced by --environment")
{
    add_usage_line("[ --search ] [search options] pattern ...");
    add_usage_line("--help");

    add_environment_variable("INQUISITIO_OPTIONS", "Default command-line options.");
}

std::string
CommandLine::app_name() const
{
    return "inquisitio";
}

std::string
CommandLine::app_synopsis() const
{
    return "A search client for Paludis, the other package mangler";
}

std::string
CommandLine::app_description() const
{
    return
        "inquisitio is a search client for Paludis. It can find packages based upon a "
        "number of different criteria including package name, description and homepage, "
        "and using a number of different match techniques including simple text match, "
        "approximate match and regular expression.";
}

CommandLine::~CommandLine()
{
}


