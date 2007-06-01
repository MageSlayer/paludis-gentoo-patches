/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Danny van Dyk <kugelfang@gentoo.org>
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

template class paludis::InstantiationPolicy<CommandLine, paludis::instantiation_method::SingletonTag>;

CommandLine::CommandLine() :
    ArgsHandler(),

    action_args(this, "Actions",
            "Selects which basic action to perform. Up to one action should "
            "be specified. If no action is specified, contrarius tries to create"
            "a cross toolchain."),
    a_version(&action_args,  "version",      'V', "Display program version"),
    a_help(&action_args,     "help",         'h', "Display program help"),

    build_args(this, "Building",
            "Tweak toolchain creation."),
    a_fetch(&build_args,     "fetch",        'f', "Only fetch sources; don't install anything"),
    a_pretend(&build_args,   "pretend",      'p', "Pretend only"),
    a_environment(&build_args, "environment", 'E', "Environment specification (class:suffix, both parts optional)"),
    a_show_reasons(&build_args, "show-reasons", '\0', "Show why packages are being installed",
            paludis::args::EnumArg::EnumArgOptions
            ("none",    "Don't show any information")
            ("summary", "Show a summary")
            ("full",    "Show full output (can be very verbose)"),
            "none"),
    a_stage(&build_args,     "stage",        's', "Build specified toolchain stage.",
            paludis::args::EnumArg::EnumArgOptions("binutils","Build binutils only")
            ("minimal", "Build a minimal gcc additionaly")
            ("headers", "Build kernel headers additionaly")
            ("libc", "Build the C Standard Library additionally")
            ("full", "Build a full cross toolchain."),
            "binutils"),
    a_target(&build_args,    "target",       't', "Build for specified CTARGET."),
    a_headers(&build_args,   "headers",      'H',
            "Add additional stage to install kernel- and libc-headers before gcc."),
    a_always_rebuild(&build_args,   "always-rebuild",   'r',    "Always rebuild already built stages."),

    a_debug_build(&build_args, "debug-build", '\0', "What to do with debug information",
            paludis::args::EnumArg::EnumArgOptions
            ("none",     "Discard debug information")
            ("split",    "Split debug information")
            ("internal", "Keep debug information with binaries"),
            "none"),

    output_options(this, "Options for output verbosity",
            ""),
    a_verbose(&output_options,   "verbose", 'v', "Be verbose"),
    a_log_level(&output_options, "log-level", 'L'),
    a_no_colour(&output_options, "no-colour", 'C', "Do not use colour"),
    a_no_color(&a_no_colour,     "no-color"),
    a_resume_command_template(&output_options, "resume-command-template", '\0', "Save the resume command to a file made using mkstemp(3)")
{
}

CommandLine::~CommandLine()
{
}

std::string
CommandLine::app_name() const
{
    return "contrarius";
}

std::string
CommandLine::app_synopsis() const
{
    return "A tool to create cross-toolchains";
}

std::string
CommandLine::app_description() const
{
    return
        "contrarius.";
}
