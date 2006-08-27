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
    a_sync(&action_args,      "sync",         's',  "Sync all or specified repositories"),
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
            "configuration variable for a particular package"),
    a_list_repositories(&action_args_internal, "list-repositories", '\0', "List available repositories"),
    a_list_categories(&action_args_internal, "list-categories", '\0', "List available categories"),
    a_list_packages(&action_args_internal, "list-packages", '\0', "List available packages"),
    a_list_sync_protocols(&action_args_internal, "list-sync-protocols", '\0', "List available sync protocols"),
    a_list_repository_formats(&action_args_internal, "list-repository-formats", '\0',
            "List available repository formats"),
    a_list_dep_tag_categories(&action_args_internal, "list-dep-tag-categories", '\0', "List known dep tag categories"),
    a_list_vulnerabilities(&action_args_internal, "list-vulnerabilities", '\0', "List known vulnerabilities"),
    a_update_news(&action_args_internal, "update-news", '\0', "Regenerate news.unread files"),

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

    query_args(this, "Query options",
            "Options which are relevant for --query."),
    a_show_slot(&query_args,        "show-slot",    'S', "Show SLOTs"),
    a_show_deps(&query_args,        "show-deps",    'D', "Show dependencies"),
    a_show_metadata(&query_args,    "show-metadata", 'M', "Show raw metadata"),

    install_args(this, "Install, Uninstall options",
            "Options which are relevant for --install or --uninstall."),
    a_pretend(&install_args, "pretend", 'p', "Pretend only"),
    a_preserve_world(&install_args, "preserve-world", '1', "Don't modify the world file"),
    a_no_config_protection(&install_args, "no-config-protection", '\0', "Disable config file protection (dangerous)"),
    a_fetch(&install_args, "fetch", 'f', "Only fetch sources; don't install anything"),

    dl_args(this, "DepList behaviour",
            "Modify dependency list generation behaviour. Use with caution."),
    a_dl_rdepend_post(&dl_args, "dl-rdepend-post", '\0', "Treat RDEPEND like PDEPEND", 
        paludis::args::EnumArg::EnumArgOptions("always", "Always")
        ("never", "Never")
        ("as-needed", "To resolve circular dependencies"),
        "as-needed"),
    a_dl_drop_self_circular(&dl_args, "dl-drop-self-circular", '\0', "Drop self-circular dependencies"),
    a_dl_drop_circular(&dl_args, "dl-drop-circular", '\0', "Drop circular dependencies"),
    a_dl_drop_all(&dl_args, "dl-drop-all", '0', "Drop all dependencies"),
    a_dl_ignore_installed(&dl_args, "dl-ignore-installed", 'e', "Ignore installed packages"),
    a_dl_no_recursive_deps(&dl_args, "dl-no-recursive-deps", '\0',
            "Don't check runtime dependencies for installed packages"),
    a_dl_max_stack_depth(&dl_args, "dl-max-stack-depth", '\0', "Maximum stack depth (default 100)"),
    a_dl_no_unnecessary_upgrades(&dl_args, "dl-no-unnecessary-upgrades", 'U',
            "Don't upgrade installed packages except where necessary as a dependency of another package"),

    list_args(this, "List options",
            "Options relevant for one or more of the --list actions."),
    a_repository(&list_args, "repository", '\0', "Matches with this repository name only"),
    a_category(&list_args,   "category",   '\0', "Matches with this category name only"),
    a_package(&list_args,    "package",    '\0', "Matches with this package name only"),

    owner_args(this, "Owner options",
            "Options relevant for the --owner actions."),
    a_full_match(&owner_args, "full-match", '\0', "Match whole filename")
{
    a_dl_max_stack_depth.set_argument(100);

    add_usage_line("--query [query options] target ...");
    add_usage_line("--install [install options] target ...");
    add_usage_line("--sync [target (leave blank for all)]");
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
    add_usage_line("--list-sync-protocols");
    add_usage_line("--list-repository-formats");
    add_usage_line("--list-dep-tag-categories");
    add_usage_line("--list-vulnerabilities [--repository repo1 ...]");
    add_usage_line("--update-news");

    add_enviromnent_variable("PALUDIS_HOME", "Overrides the home directory used when searching "
            "for configuration files etc.");
    add_enviromnent_variable("PALUDIS_NO_GLOBAL_HOOKS", "Don't use global hooks. Mostly for "
            "internal and test case use.");
    add_enviromnent_variable("PALUDIS_SKIP_CONFIG", "Don't load configuration. Mostly for "
            "internal and test case use.");
    add_enviromnent_variable("PALUDIS_EBUILD_DIR", "Where to look for ebuild.bash and related "
            "utilities.");
    add_enviromnent_variable("PALUDIS_REPOSITORY_SO_DIR", "Where to look for repository .so "
            "files.");
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

