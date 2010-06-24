/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_SRC_CLIENTS_CAVE_RESOLVE_COMMON_HH
#define PALUDIS_GUARD_SRC_CLIENTS_CAVE_RESOLVE_COMMON_HH 1

#include <paludis/environment-fwd.hh>
#include <paludis/util/sequence-fwd.hh>
#include <paludis/util/map-fwd.hh>
#include <tr1/memory>
#include "cmd_resolve_cmdline.hh"

namespace paludis
{
    namespace cave
    {
        int resolve_common(
                const std::tr1::shared_ptr<Environment> & env,
                const ResolveCommandLineResolutionOptions & resolution_options,
                const ResolveCommandLineExecutionOptions & execution_options,
                const ResolveCommandLineDisplayOptions & display_options,
                const ResolveCommandLineProgramOptions & program_options,
                const std::tr1::shared_ptr<const Map<std::string, std::string> > & keys_if_import,
                const std::tr1::shared_ptr<const Sequence<std::string> > & targets_if_not_purge,
                const bool purge
                ) PALUDIS_ATTRIBUTE((warn_unused_result));
    }
}

#endif
