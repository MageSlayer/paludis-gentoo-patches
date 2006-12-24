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

#include "command_line.hh"

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
    a_version(&action_args,   "version",      'V',  "Display program version"),
    a_info(&action_args,      "info",         'I',  "Display program version and system information"),
    a_help(&action_args,      "help",         'h',  "Display program help"),

    action_args_internal(this, "More actions",
            "Additional actions, mostly for script and internal use."),
    a_has_version(&action_args_internal, "has-version", '\0', "Check whether the specified atom is installed"),
    a_best_version(&action_args_internal, "best-version", '\0', "Display the best version of the specified atom"),
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
    a_update_news(&action_args_internal, "update-news", '\0', "Regenerate news.unread files"),
    a_regenerate_installed_cache(&action_args_internal, "regenerate-installed-cache", '\0',
            "Regenerate (non-metadata) cache for installed repositories"),
    a_regenerate_installable_cache(&action_args_internal, "regenerate-installable-cache", '\0',
            "Regenerate (non-metadata) cache for installable repositories"),

    general_args(this, "General options",
            "Options which are relevant for most or all actions."),
    a_log_level(&general_args, "log-level",  '\0', "Specify the log level",
            paludis::args::EnumArg::EnumArgOptions("debug", "Show debug output (noisy)")
            ("qa",      "Show QA messages and warnings only")
            ("warning", "Show warnings only")
            ("silent",  "Suppress all log messages"),
            "qa"),
    a_no_colour(&general_args, "no-colour", '\0', "Do not use colour"),
    a_no_color(&a_no_colour, "no-color"),
    a_config_suffix(&general_args, "config-suffix", 'c', "Config directory suffix"),
    a_resume_command_template(&general_args, "resume-command-template", '\0', "Save the resume command to a file made using mkstemp(3)"),

    query_args(this, "Query options",
            "Options which are relevant for --query."),
    a_show_deps(&query_args,        "show-deps",    'D', "Show dependencies"),
    a_show_metadata(&query_args,    "show-metadata", 'M', "Show raw metadata"),

    install_args(this, "Install, Uninstall options",
            "Options which are relevant for --install, --uninstall or --uninstall-unused."),
    a_pretend(&install_args, "pretend", 'p', "Pretend only"),
    a_preserve_world(&install_args, "preserve-world", '1', "Don't modify the world file"),
    a_no_config_protection(&install_args, "no-config-protection", '\0', "Disable config file protection (dangerous)"),
    a_debug_build(&install_args, "debug-build", '\0', "What to do with debug information",
            paludis::args::EnumArg::EnumArgOptions
            ("none",     "Discard debug information")
            ("split",    "Split debug information")
            ("internal", "Keep debug information with binaries"),
            "none"),
    a_fetch(&install_args, "fetch", 'f', "Only fetch sources; don't install anything"),
    a_show_install_reasons(&install_args, "show-install-reasons", '\0', "Show why packages are being installed",
            paludis::args::EnumArg::EnumArgOptions
            ("none",    "Don't show any information")
            ("summary", "Show a summary")
            ("full",    "Show full output (can be very verbose)"),
            "none"),
    a_show_use_descriptions(&install_args, "show-use-descriptions", '\0', "Show descriptions of USE flags",
            paludis::args::EnumArg::EnumArgOptions
            ("none",       "Don't show any descriptions")
            ("new",        "Show for new use flags")
            ("changed",    "Show for new and changed flags")
            ("all",        "Show for all flags"),
            "none"),
    a_with_unused_dependencies(&install_args, "with-unused-dependencies", '\0',
            "Also uninstall any dependencies of the target that are no longer used"),
    a_with_dependencies(&install_args, "with-dependencies", '\0',
            "Also uninstall packages that depend upon the target"),

    dl_args(this, "DepList behaviour",
            "Modify dependency list generation behaviour. Use with caution."),

    dl_reinstall(&dl_args, "dl-reinstall", '\0', "When to reinstall packages",
            paludis::args::EnumArg::EnumArgOptions
            ("never",          "Never")
            ("always",         "Always")
            ("if-use-changed", "If USE flags have changed"),
            "never"),
    dl_reinstall_scm(&dl_args, "dl-reinstall-scm", '\0', "When to reinstall scm packages",
            paludis::args::EnumArg::EnumArgOptions
            ("never",          "Never")
            ("always",         "Always")
            ("daily",          "If they are over a day old")
            ("weekly",         "If they are over a week old"),
            "never"),
    dl_upgrade(&dl_args, "dl-upgrade", '\0', "When to upgrade packages",
            paludis::args::EnumArg::EnumArgOptions
            ("always",        "Always")
            ("as-needed",     "As needed"),
            "always"),

    dl_installed_deps_pre(&dl_args, "dl-installed-deps-pre", '\0', "How to handle pre dependencies for installed packages",
            paludis::args::EnumArg::EnumArgOptions
            ("pre",           "As pre dependencies")
            ("pre-or-post",   "As pre dependencies, or post depenencies where needed")
            ("post",          "As post dependencies")
            ("try-post",      "As post dependencies, with no error for failures")
            ("discard",       "Discard"),
            "discard"),
    dl_installed_deps_runtime(&dl_args, "dl-installed-deps-runtime", '\0', "How to handle runtime dependencies for installed packages",
            paludis::args::EnumArg::EnumArgOptions
            ("pre",           "As pre dependencies")
            ("pre-or-post",   "As pre dependencies, or post depenencies where needed")
            ("post",          "As post dependencies")
            ("try-post",      "As post dependencies, with no error for failures")
            ("discard",       "Discard"),
            "try-post"),
    dl_installed_deps_post(&dl_args, "dl-installed-deps-post", '\0', "How to handle post dependencies for installed packages",
            paludis::args::EnumArg::EnumArgOptions
            ("pre",           "As pre dependencies")
            ("pre-or-post",   "As pre dependencies, or post depenencies where needed")
            ("post",          "As post dependencies")
            ("try-post",      "As post dependencies, with no error for failures")
            ("discard",       "Discard"),
            "try-post"),

    dl_uninstalled_deps_pre(&dl_args, "dl-uninstalled-deps-pre", '\0', "How to handle pre dependencies for uninstalled packages",
            paludis::args::EnumArg::EnumArgOptions
            ("pre",           "As pre dependencies")
            ("pre-or-post",   "As pre dependencies, or post depenencies where needed")
            ("post",          "As post dependencies")
            ("try-post",      "As post dependencies, with no error for failures")
            ("discard",       "Discard"),
            "pre"),
    dl_uninstalled_deps_runtime(&dl_args, "dl-uninstalled-deps-runtime", '\0', "How to handle runtime dependencies for uninstalled packages",
            paludis::args::EnumArg::EnumArgOptions
            ("pre",           "As pre dependencies")
            ("pre-or-post",   "As pre dependencies, or post depenencies where needed")
            ("post",          "As post dependencies")
            ("try-post",      "As post dependencies, with no error for failures")
            ("discard",       "Discard"),
            "pre-or-post"),
    dl_uninstalled_deps_post(&dl_args, "dl-uninstalled-deps-post", '\0', "How to handle post dependencies for uninstalled packages",
            paludis::args::EnumArg::EnumArgOptions
            ("pre",           "As pre dependencies")
            ("pre-or-post",   "As pre dependencies, or post depenencies where needed")
            ("post",          "As post dependencies")
            ("try-post",      "As post dependencies, with no error for failures")
            ("discard",       "Discard"),
            "post"),

    dl_circular(&dl_args, "dl-circular", '\0', "How to handle circular dependencies",
            paludis::args::EnumArg::EnumArgOptions
            ("error",         "Raise an error")
            ("discard",       "Discard"),
            "error"),

    dl_fall_back(&dl_args, "dl-fall-back", '\0', "When to fall back to installed packages",
            paludis::args::EnumArg::EnumArgOptions
            ("as-needed-except-targets", "Where necessary, but not for target packages")
            ("as-needed",                "Where necessary, including for target packages")
            ("never",                    "Never"),
            "as-needed-except-targets"),

    list_args(this, "List options",
            "Options relevant for one or more of the --list actions."),
    a_repository(&list_args, "repository", '\0', "Matches with this repository name only"),
    a_category(&list_args,   "category",   '\0', "Matches with this category name only"),
    a_package(&list_args,    "package",    '\0', "Matches with this package name only"),
    a_set(&list_args,        "set",        '\0', "Matches with this package set name only"),

    owner_args(this, "Owner options",
            "Options relevant for the --owner actions."),
    a_full_match(&owner_args, "full-match", '\0', "Match whole filename"),

    deprecated_args(this, "Deprecated options", "Deprecated options."),
    a_dl_no_unnecessary_upgrades(&deprecated_args, "dl-no-unnecessary-upgrades", 'U',
            "Replaced by --dl-upgrade as-needed"),
    a_dl_drop_all(&deprecated_args, "dl-drop-all", '0',
            "Drop all dependencies"),
    a_dl_ignore_installed(&deprecated_args, "dl-ignore-installed", 'e',
            "Replaced by --dl-reinstall always")
{
    add_usage_line("--query [query options] target ...");
    add_usage_line("--install [install options] target ...");
    add_usage_line("--sync [target (leave blank for all)]");
    add_usage_line("--report");
    add_usage_line("--contents target ...");
    add_usage_line("--owner [owner options] files ...");
    add_usage_line("--version");
    add_usage_line("--info");
    add_usage_line("--help");

    add_usage_line("--has-version atom");
    add_usage_line("--best-version atom");
    add_usage_line("--environment-variable atom variable");
    add_usage_line("--configuration-variable repository variable");
    add_usage_line("--list-repositories [--repository repo1 --repository repo2 ...]");
    add_usage_line("--list-categories [--repository repo1 ... --category cat1 --category cat2 ...]");
    add_usage_line("--list-packages [--repository repo1 ... --category cat1 ... --package pkg1 --package pkg2 ...]");
    add_usage_line("--list-sets [--repository repo1 ... --set set1 ...]");
    add_usage_line("--list-sync-protocols");
    add_usage_line("--list-repository-formats");
    add_usage_line("--list-dep-tag-categories");
    add_usage_line("--update-news");

    add_environment_variable("PALUDIS_HOME", "Overrides the home directory used when searching "
            "for configuration files etc.");
    add_environment_variable("PALUDIS_NO_GLOBAL_HOOKS", "Don't use global hooks. Mostly for "
            "internal and test case use.");
    add_environment_variable("PALUDIS_SKIP_CONFIG", "Don't load configuration. Mostly for "
            "internal and test case use.");
    add_environment_variable("PALUDIS_EBUILD_DIR", "Where to look for ebuild.bash and related "
            "utilities.");
    add_environment_variable("PALUDIS_REPOSITORY_SO_DIR", "Where to look for repository .so "
            "files.");
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

