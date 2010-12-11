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

#include "command_line.hh"
#include <paludis/util/singleton-impl.hh>

using namespace paludis;

template class paludis::Singleton<CommandLine>;

CommandLine::CommandLine() :
    ArgsHandler(),

    action_args(main_options_section(), "Actions",
            "Selects which basic action to perform. Exactly one action should "
            "be specified."),
    a_query(&action_args,     "query",        'q',  "Query for package information", false),
    a_install(&action_args,   "install",      'i',  "Install one or more packages", false),
    a_uninstall(&action_args, "uninstall",    'u',  "Uninstall one or more packages", false),
    a_uninstall_unused(&action_args, "uninstall-unused",    '\0',  "Uninstall unused packages", false),
    a_sync(&action_args,      "sync",         's',  "Sync all or specified repositories", false),
    a_report(&action_args,    "report",       'r',  "Report the current state of the system", false),
    a_contents(&action_args,  "contents",     'k',  "Display contents of a package", false),
    a_executables(&action_args,"executables", '\0',  "Display executable contents of a package", false),
    a_owner(&action_args,     "owner",        'o',  "Display the owner of a file", false),
    a_config(&action_args,    "config",       '\0',  "Run post-install configuration for a package", false),
    a_version(&action_args,   "version",      'V',  "Display program version", false),
    a_info(&action_args,      "info",         'I',  "Display program version and system information", false),
    a_help(&action_args,      "help",         'h',  "Display program help", false),

    action_args_internal(main_options_section(), "More actions",
            "Additional actions, mostly for script and internal use."),
    a_has_version(&action_args_internal, "has-version", '\0', "Check whether the specified spec is installed", false),
    a_best_version(&action_args_internal, "best-version", '\0', "Display the best version of the specified spec", false),
    a_match(&action_args_internal, "match", '\0', "Display all installed packages matching the supplied argument", false),
    a_environment_variable(&action_args_internal, "environment-variable", '\0', "Display the value of an environment "
            "variable for a particular package", false),
    a_configuration_variable(&action_args_internal, "configuration-variable", '\0', "Display the value of a "
            "configuration variable for a particular repository", false),
    a_list_repositories(&action_args_internal, "list-repositories", '\0', "List available repositories", false),
    a_list_categories(&action_args_internal, "list-categories", '\0', "List available categories", false),
    a_list_packages(&action_args_internal, "list-packages", '\0', "List available packages", false),
    a_list_sets(&action_args_internal, "list-sets", '\0', "List available package sets", false),
    a_list_sync_protocols(&action_args_internal, "list-sync-protocols", '\0', "List available sync protocols", false),
    a_list_repository_formats(&action_args_internal, "list-repository-formats", '\0',
            "List available repository formats", false),
    a_regenerate_installed_cache(&action_args_internal, "regenerate-installed-cache", '\0',
            "Regenerate (non-metadata) cache for installed repositories", false),
    a_regenerate_installable_cache(&action_args_internal, "regenerate-installable-cache", '\0',
            "Regenerate (non-metadata) cache for installable repositories", false),

    general_args(main_options_section(), "General options",
            "Options which are relevant for most or all actions."),
    a_log_level(&general_args, "log-level",  '\0'),
    a_no_colour(&general_args, "no-colour", '\0', "Do not use colour", false),
    a_no_color(&a_no_colour, "no-color"),
    a_force_colour(&general_args, "force-colour", '\0', "Force the use of colour", false),
    a_force_color(&a_force_colour, "force-color"),
    a_no_suggestions(&general_args, "no-suggestions", '\0', "Do not give suggestions if an unknown name is specified", false),
    a_environment(&general_args, "environment", 'E', "Environment specification (class:suffix, both parts optional)"),
    a_resume_command_template(&general_args, "resume-command-template", '\0',
            "Save the resume command to a file. If the filename contains 'XXXXXX', use mkstemp(3) to generate the filename"),
    a_multitask(&general_args, "multitask", '\0', "Perform tasks in parallel, where supported (currently --sync only)", true),
    a_compact(&general_args, "compact", '\0', "Display output using one line per entry (--install, --query)", true),

    query_args(main_options_section(), "Query options",
            "Options which are relevant for --query."),
    a_show_deps(&query_args,        "show-deps",    'D', "Show dependencies", true),
    a_show_authors(&query_args, "show-authors", 'A', "Show author information", true),
    a_show_metadata(&query_args,    "show-metadata", 'M', "Show raw metadata", true),

    install_args(main_options_section(), "Install, Uninstall options",
            "Options which are relevant for --install, --uninstall or --uninstall-unused."),

    a_serialised(&install_args, "serialised", '\0',
            "Rather than being a collection of atoms, treat the positional parameters as a serialised dependency "
            "list. The parameter to this option specifies the format version. Used by resume commands and Paludis "
            "exec()ing itself upon an upgrade; not to be used manually"),

    uninstall_args(main_options_section(), "Uninstall options",
            "Options which are relevant for --uninstall."),
    a_with_unused_dependencies(&uninstall_args, "with-unused-dependencies", '\0',
            "Also uninstall any dependencies of the target that are no longer used", true),
    a_with_dependencies(&uninstall_args, "with-dependencies", '\0',
            "Also uninstall packages that depend upon the target", true),
    a_all_versions(&uninstall_args, "all-versions", '\0',
            "Uninstall all versions of a package", true),
    a_permit_unsafe_uninstalls(&uninstall_args, "permit-unsafe-uninstalls", '\0',
            "Allow depended-upon packages to uninstalled", true),

    dl_args(main_options_section()),

    list_args(main_options_section(), "List options",
            "Options relevant for one or more of the --list actions."),
    a_repository(&list_args, "repository", '\0', "Matches with this repository name only"),
    a_repository_format(&list_args, "repository-format", '\0', "Matches with this repository format only"),
    a_category(&list_args,   "category",   '\0', "Matches with this category name only"),
    a_package(&list_args,    "package",    '\0', "Matches with this package name only"),
    a_set(&list_args,        "set",        '\0', "Matches with this package set name only"),

    owner_args(main_options_section(), "Owner options",
            "Options relevant for the --owner actions."),
    a_full_match(&owner_args, "full-match", '\0', "Match whole filename", true)
{
    add_usage_line("--query [query options] target ...");
    add_usage_line("--install [install options] target ...");
    add_usage_line("--uninstall [uninstall options] target ...");
    add_usage_line("--uninstall-unused");
    add_usage_line("--sync [target (leave blank for all)]");
    add_usage_line("--report");
    add_usage_line("--contents target ...");
    add_usage_line("--executables target ...");
    add_usage_line("--owner [owner options] files ...");
    add_usage_line("--config target ...");
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
    add_usage_line("--regenerate-installed-cache [target (leave blank for all)]");
    add_usage_line("--regenerate-installable-cache [target (leave blank for all)]");

    add_environment_variable("PALUDIS_OPTIONS", "Default command-line options.");

    add_example(
            "paludis --sync",
            "Sync all syncable repositories, and perform any necessary cache updates.");
    add_example(
            "paludis --install --pretend world",
            "Show available updates for packages in the 'world' set (which contains all packages that "
            "have been explicitly installed as targets, along with the 'system' set), along with runtime "
            "dependencies of packages therein recursively.");
    add_example(
            "paludis --install world",
            "...and, having checked the output above, perform the install.");
    add_example(
            "paludis --install --continue-on-failure if-satisfied world",
            "...and continue as far as possible even after errors are encountered.");
    add_example(
            "paludis --install --pretend --dl-reinstall if-use-changed world",
            "...also reinstall packages whose use settings have changed.");
    add_example(
            "paludis --install --pretend --dl-reinstall-scm weekly world",
            "...also reinstall any scm (cvs, svn, ...) package that was installed over a week ago.");
    add_example(
            "paludis --install --pretend x11-wm/fluxbox",
            "Show what would be done to install a single package, along with all its dependencies, "
            "whilst recursively updating runtime dependencies.");
    add_example(
            "paludis --install --pretend fluxbox",
            "...as above, if the package name is unambiguous.");
    add_example(
            "paludis --install fluxbox",
            "...and perform the install, and add the package to the 'world' set when done.");
    add_example(
            "paludis --install --preserve-world fluxbox",
            "...or don't add the package to the 'world' set.");
    add_example(
            "paludis --install --pretend --dl-upgrade as-needed fluxbox",
            "...only update dependencies where required.");
    add_example(
            "paludis --install --pretend =x11-wm/fluxbox-1.0.0",
            "...specifying an exact version (which also prevents the package from being added to 'world').");
    add_example(
            "paludis --uninstall app-editors/emacs",
            "Uninstall a package.");
    add_example(
            "paludis --uninstall --pretend --with-dependencies app-editors/emacs",
            "Uninstall a package, along with any packages depending upon it.");
    add_example(
            "paludis --uninstall --pretend --with-dependencies --with-unused-dependencies app-editors/emacs",
            "...and also any packages that are only installed to satisfy that package's dependencies.");
    add_example(
            "paludis --uninstall --pretend --permit-unsafe-uninstalls app-editors/emacs",
            "...uninstall, even if the package is required by another installed package.");
    add_example(
            "paludis --uninstall-unused --pretend",
            "Uninstall all unused packages.");

    add_note("paludis is deprecated. Use 'cave' instead.");
}

std::string
CommandLine::app_name() const
{
    return "paludis";
}

std::string
CommandLine::app_synopsis() const
{
    return "The deprecated other package mangler client";
}

std::string
CommandLine::app_description() const
{
    return "paludis is deprecated; use 'cave' instead.";
}

CommandLine::~CommandLine()
{
}

