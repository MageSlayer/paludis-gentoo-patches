/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 Alexander Færøy
 * Copyright (c) 2010 Ciaran McCreesh
 *
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

#ifndef PALUDIS_GUARD_SRC_CLIENTS_CAVE_OWNER_COMMON_HH
#define PALUDIS_GUARD_SRC_CLIENTS_CAVE_OWNER_COMMON_HH 1

#include <paludis/package_id-fwd.hh>
#include <paludis/environment-fwd.hh>
#include <string>
#include <tr1/memory>
#include <tr1/functional>

namespace paludis
{
    namespace cave
    {
        int owner_common(
                const std::tr1::shared_ptr<Environment> & env,
                const std::string & match,
                const std::string & query,
                const std::tr1::function<void (const std::tr1::shared_ptr<const PackageID> &)> &);
    }
}

#endif
