/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_SRC_CLIENTS_CAVE_CMD_SEARCH_CMDLINE_HH
#define PALUDIS_GUARD_SRC_CLIENTS_CAVE_CMD_SEARCH_CMDLINE_HH 1

#include "command_command_line.hh"
#include <paludis/environment-fwd.hh>
#include <memory>

namespace paludis
{
    namespace cave
    {
        struct SearchCommandLineCandidateOptions :
            args::ArgsSection
        {
            SearchCommandLineCandidateOptions(args::ArgsHandler * const);

            args::ArgsGroup g_candidate_options;
            args::SwitchArg a_all_versions;
            args::SwitchArg a_visible;
            args::StringSetArg a_matching;
        };

        struct SearchCommandLineMatchOptions :
            args::ArgsSection
        {
            SearchCommandLineMatchOptions(args::ArgsHandler * const);

            args::ArgsGroup g_pattern_options;
            args::EnumArg a_type;
            args::SwitchArg a_and;
            args::SwitchArg a_not;

            args::ArgsGroup g_search_key_options;
            args::StringSetArg a_key;
            args::SwitchArg a_name;
            args::SwitchArg a_description;
        };
    }
}

#endif
