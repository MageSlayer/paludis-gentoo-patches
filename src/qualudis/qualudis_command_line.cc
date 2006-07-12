/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#include "qualudis_command_line.hh"

QualudisCommandLine::QualudisCommandLine() :
    ArgsHandler(),

    action_args(this, "Actions (specify exactly one)"),
    a_describe(&action_args, "describe",     'd', "Describe checks"),
    a_version(&action_args,  "version",      'V', "Display program version"),
    a_help(&action_args,     "help",         'h', "Display program help"),

    check_options(this, "Options for general checks"),
    a_qa_checks(&check_options, "qa-check", 'c', "Only perform given check."),
    a_verbose(&check_options, "verbose", 'v', "Be verbose"),
    a_log_level(&check_options, "log-level", 'L', "Specify the log level",
            paludis::args::EnumArg::EnumArgOptions("debug", "Show debug output (noisy)")
            ("qa",      "Show QA messages and warnings only")
            ("warning", "Show warnings only")
            ("silent", "Suppress all log messages"),
            "warning"),

    a_message_level(&check_options, "message-level", 'M', "Specify the message level",
            paludis::args::EnumArg::EnumArgOptions("info", "Show info and upwards")
            ("minor", "Show minor and upwards")
            ("major", "Show major and upwards")
            ("fatal", "Show only fatals"),
            "warning"),

    message_level(paludis::qa::qal_info)
{
}

QualudisCommandLine::~QualudisCommandLine()
{
}

