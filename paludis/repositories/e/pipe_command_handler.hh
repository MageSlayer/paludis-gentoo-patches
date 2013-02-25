/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_PIPE_COMMAND_HANDLER_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_PIPE_COMMAND_HANDLER_HH 1

#include <paludis/repositories/e/permitted_directories-fwd.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/output_manager-fwd.hh>
#include <functional>
#include <string>

namespace paludis
{
    namespace erepository
    {
        class ERepositoryID;

        std::string pipe_command_handler(const Environment * const,
                const std::shared_ptr<const ERepositoryID> &,
                const std::shared_ptr<PermittedDirectories> &,
                bool in_metadata_generation,
                const std::string & s,
                const std::shared_ptr<OutputManager> & maybe_output_manager);
    }
}

#endif
