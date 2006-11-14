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

using namespace gtkpaludis;

CommandLine::CommandLine() :
    ArgsHandler(),

    action_args(this, "Actions",
            "Selects which basic action to perform. Up to one action should "
            "be specified."),
    a_version(&action_args,  "version",      'V', "Display program version"),
    a_help(&action_args,     "help",         'h', "Display program help"),

    general_args(this, "General options",
            "Options which are relevant for most or all actions."),
    a_log_level(&general_args, "log-level",  '\0', "Specify the log level",
            paludis::args::EnumArg::EnumArgOptions("debug", "Show debug output (noisy)")
            ("qa",      "Show QA messages and warnings only")
            ("warning", "Show warnings only")
            ("silent",  "Suppress all log messages"),
            "qa"),
    a_config_suffix(&general_args, "config-suffix", 'c', "Config directory suffix")
{
    add_usage_line("[ general options ]");
    add_usage_line("--version");
    add_usage_line("--help");
    add_environment_variable("GTKPALUDIS_OPTIONS", "Default command-line options.");
}

std::string
CommandLine::app_name() const
{
    return "gtkpaludis";
}

std::string
CommandLine::app_synopsis() const
{
    return "A graphical interface for the other package mangler";
}

std::string
CommandLine::app_description() const
{
    return
        "A graphical interface for the paludis package manager.";
}

CommandLine::~CommandLine()
{
}

