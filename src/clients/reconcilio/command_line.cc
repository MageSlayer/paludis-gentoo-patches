/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 David Leverton
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

template class Singleton<CommandLine>;

CommandLine::CommandLine() :
    ArgsHandler(),

    action_args(main_options_section(), "Actions",
            "Selects which basic action to perform. At most one action should "
            "be specified."),
    a_fix_linkage(&action_args, "fix-linkage", '\0', "Search for and rebuild packages linked against non-existant libraries (default)", false),
    a_version(&action_args,     "version",     'V',  "Display program version", false),
    a_help(&action_args,        "help",        'h',  "Display program help", false),

    general_args(main_options_section(), "General options",
            "Options which are relevant for most or all actions."),
    a_log_level(&general_args,      "log-level",      '\0'),
    a_no_colour(&general_args, "no-colour", '\0', "Do not use colour", false),
    a_no_color(&a_no_colour,   "no-color"),
    a_force_colour(&general_args, "force-colour", '\0', "Force the use of colour", false),
    a_force_color(&a_force_colour, "force-color"),
    a_environment(&general_args,    "environment",    'E',  "Environment specification (class:suffix, both parts optional)"),
    a_exact(&general_args,     "exact",               '\0', "Rebuild the same package version that is currently installed", true),
    a_resume_command_template(&general_args, "resume-command-template", '\0',
            "Save the resume command to a file. If the filename contains 'XXXXXX', use mkstemp(3) to generate the filename"),
    a_compact(&general_args, "compact", '\0', "Display output using one line per entry", true),

    fix_linkage_args(main_options_section(), "Fix Linkage options",
            "Options which are relevant for --fix-linkage."),
    a_library(&fix_linkage_args, "library", '\0', "Only rebuild packages linked against this library, even if it exists"),

    install_args(main_options_section(), "Install options",
            "Options which are relevant for the install process."),
    dl_args(main_options_section())
{
    add_usage_line("[ --fix-linkage ] [fix linkage options]");
    add_usage_line("--help");

    // XXX destinations support
    install_args.a_destinations.remove();
    install_args.a_preserve_world.remove();
    install_args.a_preserve_world.set_specified(true);
    install_args.a_add_to_world_spec.remove();

    dl_args.dl_reinstall_targets.remove();
    dl_args.dl_upgrade.set_default_arg("as-needed");
    dl_args.dl_new_slots.set_default_arg("as-needed");

    add_environment_variable("RECONCILIO_OPTIONS", "Default command-line options.");

    add_example(
            "reconcilio --pretend",
            "Find and display any packages that appear to have broken linkage. (Requires read access to "
            "files to be checked, so best run as root.)");
    add_example(
            "reconcilio --pretend --exact",
            "The same, but try to reinstall exact versions, even if an upgrade is available.");
    add_example(
            "reconcilio --pretend --library 'libXi.so.6",
            "Find and display any packages that need a named library.");

    add_note("Reconcilio is deprecated. Use 'cave fix-linkage' instead.");
}

std::string
CommandLine::app_name() const
{
    return "reconcilio";
}

std::string
CommandLine::app_synopsis() const
{
    return "A deprecated client for rebuilding packages with broken linkage.";
}

std::string
CommandLine::app_description() const
{
    return
        "Reconcilio is a deprecated linkage fixing client for Paludis. 'cave fix-linkage' should "
        "be used instead.";
}

CommandLine::~CommandLine()
{
}


