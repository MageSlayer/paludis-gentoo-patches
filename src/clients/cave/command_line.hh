/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_SRC_CLIENTS_CAVE_COMMAND_LINE_HH
#define PALUDIS_GUARD_SRC_CLIENTS_CAVE_COMMAND_LINE_HH 1

#include <paludis/args/args_handler.hh>
#include <paludis/args/log_level_arg.hh>

namespace paludis
{
    namespace cave
    {
        struct CaveCommandLine :
            args::ArgsHandler
        {
            std::string app_name() const override
            {
                return "cave";
            }

            std::string app_synopsis() const override
            {
                return "A commandline client for the other package mangler.";
            }

            std::string app_description() const override
            {
                return "The front-end to a number of commands.";
            }

            args::ArgsGroup g_global_options;
            args::StringArg a_environment;
            args::LogLevelArg a_log_level;
            args::EnumArg a_colour;
            args::AliasArg a_color;
            args::SwitchArg a_help;
            args::SwitchArg a_version;

            CaveCommandLine();
        };
    }
}

#endif
