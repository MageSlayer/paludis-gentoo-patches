/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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
    a_install(&action_args,   "install",      'i',  "Install one or more packages (default)"),
    a_version(&action_args,   "version",      'V',  "Display program version"),
    a_help(&action_args,      "help",         'h',  "Display program help"),

    general_args(this, "General options",
            "Options which are relevant for most or all actions."),
    a_log_level(&general_args, "log-level",  '\0'),
    a_no_colour(&general_args, "no-colour", '\0', "Do not use colour"),
    a_no_color(&a_no_colour, "no-color"),
    a_environment(&general_args, "environment", 'E', "Environment specification (class:suffix, both parts optional)"),

    source_args(this, "Source options",
            "Options affecting the source image"),
    a_location(&source_args, "location", 'l', "Location of source image (default: current directory)"),

    install_args(this, "Install options",
            "Options which are relevant for --install"),

    dl_args(this)
{
    add_usage_line("[ --install ] [ --location path/ ] category/package [ version ] [ slot ]");

    install_args.a_preserve_world.remove();
    install_args.a_preserve_world.set_specified(true);
    install_args.a_add_to_world_spec.remove();
    install_args.a_debug_build.remove();
    install_args.a_checks.remove();
    install_args.a_fetch.remove();
    install_args.a_no_safe_resume.remove();

    dl_args.dl_reinstall_targets.remove();

    add_environment_variable("IMPORTARE_OPTIONS", "Default command-line options.");
}

std::string
CommandLine::app_name() const
{
    return "importare";
}

std::string
CommandLine::app_synopsis() const
{
    return "The Paludis unpackaged package installer";
}

std::string
CommandLine::app_description() const
{
    return
        "importare can be used to manage packages where no real package file is available. It "
        "treats the contents of a named directory as being the content of the package, and uses "
        "a dummy package name provided on the command line to do the install. Safe merge, unmerge, "
        "upgrade and replace support is provided, as is content tracking for installed files.";
}

CommandLine::~CommandLine()
{
}

