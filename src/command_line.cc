/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

#include "src/command_line.hh"

CommandLine::CommandLine() :
    ArgsHandler(),

    action_args(this, "Actions (specify exactly one)"),
    a_query(&action_args,    "query",        'q', "Query for package information"),
    a_install(&action_args,  "install",      'i', "Install one or more packages"),
    a_list_repositories(&action_args, "list-repositories", '\0', "List available repositories"),
    a_list_categories(&action_args, "list-categories", '\0', "List available categories"),
    a_list_packages(&action_args, "list-packages", '\0', "List available packages"),
    a_version(&action_args,  "version",      'V', "Display program version"),
    a_help(&action_args,     "help",         'h', "Display program help"),

    general_args(this, "General options"),
    a_no_colour(&general_args, "no-colour", 'c', "Do not use colour"),
    a_no_color(&a_no_colour, "no-color"),

    query_args(this, "Query options"),
    a_show_slot(&query_args,        "show-slot",    'S', "Show SLOTs"),
    a_show_license(&query_args,     "show-license", 'L', "Show licenses"),
    a_show_licence(&a_show_license, "show-licence"),
    a_show_deps(&query_args,        "show-deps",    'd', "Show dependencies"),
    a_show_metadata(&query_args,    "show-metadata", 'M', "Show raw metadata"),

    install_args(this, "Install options"),
    a_pretend(&install_args, "pretend", 'p', "Pretend only"),

    dl_args(this, "DepList behaviour (use with caution)"),
    a_dl_rdepend_post(&dl_args, "dl-rdepend-post", '\0', "Treat RDEPEND like PDEPEND", 
        paludis::args::EnumArg::EnumArgOptions("always", "Always")
        ("never", "Never")
        ("as-needed", "To resolve circular dependencies"),
        "as-needed"),
    a_dl_drop_self_circular(&dl_args, "dl-drop-self-circular", '\0', "Drop self-circular dependencies"),
    a_dl_patch_dep(&dl_args, "dl-dont-ignore-the-frickin-patch-dep", '\0', "Don't ignore the stupid patch depend on itself"),
    a_dl_drop_circular(&dl_args, "dl-drop-circular", '\0', "Drop circular dependencies"),
    a_dl_drop_all(&dl_args, "dl-drop-all", '0', "Drop all dependencies"),
    a_dl_ignore_installed(&dl_args, "dl-ignore-installed", 'e', "Ignore installed packages"),
    a_dl_recursive_deps(&dl_args, "dl-recursive-deps", 'D', "Check dependencies for installed packages"),
    a_dl_max_stack_depth(&dl_args, "dl-max-stack-depth", '\0', "Maximum stack depth (default 100)")
{
    a_dl_max_stack_depth.set_argument(100);
}

CommandLine::~CommandLine()
{
}

