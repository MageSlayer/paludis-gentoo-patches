/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_MAKE_ARCHIVE_STRINGS_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_MAKE_ARCHIVE_STRINGS_HH 1

#include <paludis/repositories/e/e_repository_id.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/environment-fwd.hh>

namespace paludis
{
    namespace erepository
    {
        std::pair<std::string, std::string> make_archives_strings(
                const Environment * const,
                const std::shared_ptr<const ERepositoryID> &);
    }
}

#endif
