/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

template class paludis::InstantiationPolicy<CommandLine, paludis::instantiation_method::SingletonTag>;

CommandLine::CommandLine() :
    ArgsHandler(),

    action_args(this, "Actions",
            "Selects which basic action to perform. Exactly one action should "
            "be specified."),
    a_query(&action_args,     "query",        'q',  "Query for package information"),
    a_install(&action_args,   "install",      'i',  "Install one or more packages"),
    a_uninstall(&action_args, "uninstall",    'u',  "Uninstall one or more packages"),
    a_uninstall_unused(&action_args, "uninstall-unused",    '\0',  "Uninstall unused packages"),
    a_sync(&action_args,      "sync",         's',  "Sync all or specified repositories"),
    a_report(&action_args,    "report",       'r',  "Report the current state of the system"),
    a_contents(&action_args,  "contents",     'k',  "Display contents of a package"),
    a_owner(&action_args,     "owner",        'o',  "Display the owner of a file"),
    a_config(&action_args,    "config",       '\0',  "Run post-install configuration for a package"),
    a_version(&action_args,   "version",      'V',  "Display program version"),
    a_info(&action_args,      "info",         'I',  "Display program version and system information"),
    a_help(&action_args,      "help",         'h',  "Display program help"),

    action_args_internal(this, "More actions",
            "Additional actions, mostly for script and internal use."),
    a_has_version(&action_args_internal, "has-version", '\0', "Check whether the specified spec is installed"),
    a_best_version(&action_args_internal, "best-version", '\0', "Display the best version of the specified spec"),
    a_match(&action_args_internal, "match", '\0', "Display all installed packages matching the supplied argument"),
    a_environment_variable(&action_args_internal, "environment-variable", '\0', "Display the value of an environment "
            "variable for a particular package"),
    a_configuration_variable(&action_args_internal, "configuration-variable", '\0', "Display the value of a "
            "configuration variable for a particular repository"),
    a_list_repositories(&action_args_internal, "list-repositories", '\0', "List available repositories"),
    a_list_categories(&action_args_internal, "list-categories", '\0', "List available categories"),
    a_list_packages(&action_args_internal, "list-packages", '\0', "List available packages"),
    a_list_sets(&action_args_internal, "list-sets", '\0', "List available package sets"),
    a_list_sync_protocols(&action_args_internal, "list-sync-protocols", '\0', "List available sync protocols"),
    a_list_repository_formats(&action_args_internal, "list-repository-formats", '\0',
            "List available repository formats"),
    a_list_dep_tag_categories(&action_args_internal, "list-dep-tag-categories", '\0', "List known dep tag categories"),
    a_regenerate_installed_cache(&action_args_internal, "regenerate-installed-cache", '\0',
            "Regenerate (non-metadata) cache for installed repositories"),
    a_regenerate_installable_cache(&action_args_internal, "regenerate-installable-cache", '\0',
            "Regenerate (non-metadata) cache for installable repositories"),

    general_args(this, "General options",
            "Options which are relevant for most or all actions."),
    a_log_level(&general_args, "log-level",  '\0'),
    a_no_colour(&general_args, "no-colour", '\0', "Do not use colour"),
    a_no_color(&a_no_colour, "no-color"),
    a_environment(&general_args, "environment", 'E', "Environment specification (class:suffix, both parts optional)"),
    a_resume_command_template(&general_args, "resume-command-template", '\0', "Save the resume command to a file. If the filename contains 'XXXXXX', use mkstemp(3) to generate the filename"),

    query_args(this, "Query options",
            "Options which are relevant for --query."),
    a_show_deps(&query_args,        "show-deps",    'D', "Show dependencies"),
    a_show_metadata(&query_args,    "show-metadata", 'M', "Show raw metadata"),

    install_args(this, "Install, Uninstall options",
            "Options which are relevant for --install, --uninstall or --uninstall-unused."),

    uninstall_args(this, "Uninstall options",
            "Options which are relevant for --uninstall."),
    a_with_unused_dependencies(&uninstall_args, "with-unused-dependencies", '\0',
            "Also uninstall any dependencies of the target that are no longer used"),
    a_with_dependencies(&uninstall_args, "with-dependencies", '\0',
            "Also uninstall packages that depend upon the target"),
    a_all_versions(&uninstall_args, "all-versions", '\0',
            "Uninstall all versions of a package"),
    a_permit_unsafe_uninstalls(&uninstall_args, "permit-unsafe-uninstalls", '\0',
            "Allow depended-upon packages to uninstalled"),

    dl_args(this),

    list_args(this, "List options",
            "Options relevant for one or more of the --list actions."),
    a_repository(&list_args, "repository", '\0', "Matches with this repository name only",
            paludis::args::StringSetArg::StringSetArgOptions(), &paludis::RepositoryNameValidator::validate),
    a_repository_format(&list_args, "repository-format", '\0', "Matches with this repository format only"),
    a_category(&list_args,   "category",   '\0', "Matches with this category name only",
            paludis::args::StringSetArg::StringSetArgOptions(), &paludis::CategoryNamePartValidator::validate),
    a_package(&list_args,    "package",    '\0', "Matches with this package name only",
            paludis::args::StringSetArg::StringSetArgOptions(), &paludis::PackageNamePartValidator::validate),
    a_set(&list_args,        "set",        '\0', "Matches with this package set name only",
            paludis::args::StringSetArg::StringSetArgOptions(), &paludis::SetNameValidator::validate),

    owner_args(this, "Owner options",
            "Options relevant for the --owner actions."),
    a_full_match(&owner_args, "full-match", '\0', "Match whole filename"),

    deprecated_args(this, "Deprecated options", "Deprecated options."),
    a_dl_no_unnecessary_upgrades(&deprecated_args, "dl-no-unnecessary-upgrades", 'U',
            "Replaced by --dl-upgrade as-needed"),
    a_dl_drop_all(&deprecated_args, "dl-drop-all", '0',
            "Replaced by --dl-deps-default discard"),
    a_dl_ignore_installed(&deprecated_args, "dl-ignore-installed", 'e',
            "Replaced by --dl-reinstall always"),
    a_show_install_reasons(&deprecated_args, "show-install-reasons",
            '\0', "Replaced by --show-reasons"),
    a_add_to_world_atom(&deprecated_args, "add-to-world-atom", '\0',
            "Replaced by --add-to-world-spec"),
    a_config_suffix(&deprecated_args, "config-suffix", 'c',
            "Replaced by --environment"),
    a_update_news(&deprecated_args, "update-news", '\0',
            "No longer useful, does nothing"),
    a_safe_resume(&deprecated_args, "safe-resume", '\0',
            "Now default behaviour, use --no-safe-resume to disable")
{
    add_usage_line("--query [query options] target ...");
    add_usage_line("--install [install options] target ...");
    add_usage_line("--sync [target (leave blank for all)]");
    add_usage_line("--report");
    add_usage_line("--contents target ...");
    add_usage_line("--owner [owner options] files ...");
    add_usage_line("--version");
    add_usage_line("--info [target ...]");
    add_usage_line("--help");

    add_usage_line("--has-version spec");
    add_usage_line("--best-version spec");
    add_usage_line("--environment-variable spec variable");
    add_usage_line("--configuration-variable repository variable");
    add_usage_line("--list-repositories [--repository repo1 --repository repo2 ...]");
    add_usage_line("--list-categories [--repository repo1 ... --category cat1 --category cat2 ...]");
    add_usage_line("--list-packages [--repository repo1 ... --category cat1 ... --package pkg1 --package pkg2 ...]");
    add_usage_line("--list-sets [--repository repo1 ... --set set1 ...]");
    add_usage_line("--list-sync-protocols");
    add_usage_line("--list-repository-formats");
    add_usage_line("--list-dep-tag-categories");

    add_environment_variable("PALUDIS_OPTIONS", "Default command-line options.");
}

std::string
CommandLine::app_name() const
{
    return "paludis";
}

std::string
CommandLine::app_synopsis() const
{
    return "The other package mangler";
}

std::string
CommandLine::app_description() const
{
    return
        "paludis is the command line interface used to handle packages. It can query and "
        "install packages, update repositories and display information about packages "
        "already installed on a system.";
}

CommandLine::~CommandLine()
{
}

