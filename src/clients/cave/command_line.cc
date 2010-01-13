/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010 Ciaran McCreesh
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

using namespace paludis;
using namespace cave;

CaveCommandLine::CaveCommandLine() :
    g_global_options(main_options_section(), "Global Options", "Global options, used by every subcommand."),
    a_environment(&g_global_options, "environment", 'E',
            "Environment specification (class:suffix, both parts optional)"),
    a_log_level(&g_global_options, "log-level", '\0')
{
    add_usage_line("[ --environment class:suffix ] [ --log-level level ] COMMAND [ARGS...]");
    add_description_line("For the COMMAND argument, see 'cave help' for a list of common commands, "
            "or 'cave help --all' for all commands. To see documentation for a command named "
            "'foo', use 'man cave-foo'.");
    add_description_line("Arguments specified after the COMMAND are handled by the individual "
            "commands. Arguments before the COMMAND are global arguments shared by all commands.");
}

