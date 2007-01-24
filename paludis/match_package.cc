/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <paludis/match_package.hh>

/** \file
 * Implementation for match_package.hh.
 *
 * \ingroup grpmatchpackage
 */

using namespace paludis;

bool
match_package_internals::do_match(
        const Environment * const env,
        const PackageDepAtom * const atom,
        const PackageDatabaseEntry * const entry)
{
    if (atom->package() != entry->name)
        return false;

    if (atom->version_requirements_ptr())
        switch (atom->version_requirements_mode())
        {
            case vr_and:
                for (VersionRequirements::Iterator r(atom->version_requirements_ptr()->begin()),
                        r_end(atom->version_requirements_ptr()->end()) ; r != r_end ; ++r)
                    if (! (((entry->version).*(r->version_operator.as_version_spec_operator()))(r->version_spec)))
                        return false;
                break;

            case vr_or:
                {
                    bool matched(false);
                    for (VersionRequirements::Iterator r(atom->version_requirements_ptr()->begin()),
                            r_end(atom->version_requirements_ptr()->end()) ; r != r_end ; ++r)
                        if ((((entry->version).*(r->version_operator.as_version_spec_operator()))(r->version_spec)))
                        {
                            matched = true;
                            break;
                        }

                    if (! matched)
                        return false;
                }
                break;

            case last_vr:
                ;
        }

    if (atom->repository_ptr())
        if (*atom->repository_ptr() != entry->repository)
            return false;

    if (atom->slot_ptr() || atom->use_requirements_ptr())
    {
        VersionMetadata::ConstPointer metadata(env->package_database()->fetch_repository(
                    entry->repository)->version_metadata(
                    entry->name, entry->version));

        if (atom->slot_ptr())
            if (*atom->slot_ptr() != SlotName(metadata->slot))
                return false;

        if (atom->use_requirements_ptr())
        {
            for (UseRequirements::Iterator u(atom->use_requirements_ptr()->begin()),
                    u_end(atom->use_requirements_ptr()->end()) ; u != u_end ; ++u)
            {
                switch (u->second)
                {
                    case use_unspecified:
                        continue;

                    case use_enabled:
                        if (! env->query_use(u->first, entry))
                            return false;
                        continue;

                    case use_disabled:
                        if (env->query_use(u->first, entry))
                            return false;
                        continue;
                }
                throw InternalError(PALUDIS_HERE, "bad UseFlagState");
            }
        }
    }

    return true;
}

