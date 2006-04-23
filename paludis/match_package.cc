/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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
    if (atom->package() != entry->get<pde_name>())
        return false;

    if (atom->version_spec_ptr() && ! (((entry->get<pde_version>()).*
                    (atom->version_operator().as_version_spec_operator()))
                (*atom->version_spec_ptr())))
        return false;

    if (atom->repository_ptr())
        if (*atom->repository_ptr() != entry->get<pde_repository>())
            return false;

    if (atom->slot_ptr() || atom->use_requirements_ptr())
    {
        VersionMetadata::ConstPointer metadata(env->package_database()->fetch_metadata(*entry));

        if (atom->slot_ptr())
            if (*atom->slot_ptr() != SlotName(metadata->get(vmk_slot)))
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

bool
match_package_internals::do_match(
        const Environment * const env,
        const PackageDepAtom * const atom,
        const DepListEntry * const entry)
{
    if (atom->package() != entry->get<dle_name>())
        return false;

    if (atom->version_spec_ptr() && ! (((entry->get<dle_version>()).*
                    (atom->version_operator().as_version_spec_operator()))
                (*atom->version_spec_ptr())))
        return false;

    if (atom->repository_ptr() && (*atom->repository_ptr() != entry->get<dle_repository>()))
        return false;

    if (atom->slot_ptr() && (atom->slot_ptr()->data() != entry->get<dle_metadata>()->get(vmk_slot)))
        return false;

    if (atom->use_requirements_ptr())
    {
        PackageDatabaseEntry e(entry->get<dle_name>(), entry->get<dle_version>(),
                entry->get<dle_repository>());

        for (UseRequirements::Iterator u(atom->use_requirements_ptr()->begin()),
                u_end(atom->use_requirements_ptr()->end()) ; u != u_end ; ++u)
        {
            switch (u->second)
            {
                case use_unspecified:
                    continue;

                case use_enabled:
                    if (! env->query_use(u->first, &e))
                        return false;
                    continue;

                case use_disabled:
                    if (env->query_use(u->first, &e))
                        return false;
                    continue;
            }
            throw InternalError(PALUDIS_HERE, "bad UseFlagState");
        }
    }

    return true;
}

