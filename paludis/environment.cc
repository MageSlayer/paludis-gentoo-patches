/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

#include "package_database.hh"
#include "environment.hh"
#include "internal_error.hh"

using namespace paludis;

Environment::Environment(PackageDatabase::Pointer d, PackageDatabase::Pointer i) :
    _package_db(d),
    _installed_db(i)
{
}

Environment::~Environment()
{
}

MaskReasons
Environment::mask_reasons(const PackageDatabaseEntry & e) const
{
    MaskReasons result;
    VersionMetadata::ConstPointer metadata(package_db()->fetch_metadata(e));

    if (metadata->get(vmk_eapi) != "0" && metadata->get(vmk_eapi) != "")
        result.set(mr_eapi);
    else
    {

        result.set(mr_keyword);
        for (VersionMetadata::KeywordIterator k(metadata->begin_keywords()),
                k_end(metadata->end_keywords()) ; k != k_end ; ++k)
            if (accept_keyword(*k, &e))
            {
                result.reset(mr_keyword);
                break;
            }

        if (! query_user_unmasks(e))
        {
            if (query_user_masks(e))
                result.set(mr_user_mask);

            if (package_db()->fetch_repository(e.get<pde_repository>())->query_profile_masks(e.get<pde_package>(),
                        e.get<pde_version>()))
                result.set(mr_profile_mask);

            if (package_db()->fetch_repository(e.get<pde_repository>())->query_repository_masks(e.get<pde_package>(),
                        e.get<pde_version>()))
                result.set(mr_repository_mask);
        }
    }

    return result;
}

