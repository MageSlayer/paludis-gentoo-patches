/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 Saleem Abdulrasool
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

#ifndef PALUDIS_GUARD_SRC_CLIENTS_CAVE_COMMAND_COMMAND_LINE_HH
#define PALUDIS_GUARD_SRC_CLIENTS_CAVE_COMMAND_COMMAND_LINE_HH 1

#include <iosfwd>
#include <paludis/args/args.hh>

namespace paludis
{
    namespace cave
    {
        struct CaveCommandCommandLine :
            args::ArgsHandler
        {
            friend std::ostream & operator<< (std::ostream &, const CaveCommandCommandLine &);

            args::ArgsGroup g_global_options;
            args::SwitchArg a_help;

            CaveCommandCommandLine();
        };

        std::ostream &
        operator<< (std::ostream &, const CaveCommandCommandLine &) PALUDIS_VISIBLE;
    }
}

#endif

