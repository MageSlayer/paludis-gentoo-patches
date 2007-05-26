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
#include <paludis/dep_spec.hh>
#include <paludis/environment.hh>
#include <paludis/version_metadata.hh>
#include <paludis/package_database.hh>
#include <paludis/util/visitor-impl.hh>
#include <algorithm>

using namespace paludis;

bool
paludis::match_package(
        const Environment & env,
        const PackageDepSpec & spec,
        const PackageDatabaseEntry & entry)
{
    if (spec.package_ptr() && *spec.package_ptr() != entry.name)
        return false;

    if (spec.package_name_part_ptr() && *spec.package_name_part_ptr() != entry.name.package)
        return false;

    if (spec.category_name_part_ptr() && *spec.category_name_part_ptr() != entry.name.category)
        return false;

    if (spec.version_requirements_ptr())
        switch (spec.version_requirements_mode())
        {
            case vr_and:
                for (VersionRequirements::Iterator r(spec.version_requirements_ptr()->begin()),
                        r_end(spec.version_requirements_ptr()->end()) ; r != r_end ; ++r)
                    if (! r->version_operator.as_version_spec_comparator()(entry.version, r->version_spec))
                        return false;
                break;

            case vr_or:
                {
                    bool matched(false);
                    for (VersionRequirements::Iterator r(spec.version_requirements_ptr()->begin()),
                            r_end(spec.version_requirements_ptr()->end()) ; r != r_end ; ++r)
                        if (r->version_operator.as_version_spec_comparator()(entry.version, r->version_spec))
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

    if (spec.repository_ptr())
        if (*spec.repository_ptr() != entry.repository)
            return false;

    if (spec.slot_ptr() || spec.use_requirements_ptr())
    {
        tr1::shared_ptr<const VersionMetadata> metadata(env.package_database()->fetch_repository(
                    entry.repository)->version_metadata(
                    entry.name, entry.version));

        if (spec.slot_ptr())
            if (*spec.slot_ptr() != SlotName(metadata->slot))
                return false;

        if (spec.use_requirements_ptr())
        {
            for (UseRequirements::Iterator u(spec.use_requirements_ptr()->begin()),
                    u_end(spec.use_requirements_ptr()->end()) ; u != u_end ; ++u)
            {
                switch (u->second)
                {
                    case use_unspecified:
                        continue;

                    case use_enabled:
                        if (! env.query_use(u->first, entry))
                            return false;
                        continue;

                    case use_disabled:
                        if (env.query_use(u->first, entry))
                            return false;
                        continue;

                    case last_use:
                        ;
                }
                throw InternalError(PALUDIS_HERE, "bad UseFlagState");
            }
        }
    }

    return true;
}

namespace
{
    struct IsInHeirarchy :
        ConstVisitor<SetSpecTree>,
        std::unary_function<PackageDatabaseEntry, bool>
    {
        const Environment & env;
        const SetSpecTree::ConstItem & target;
        const PackageDatabaseEntry * dbe;
        bool matched;

        IsInHeirarchy(const Environment & e, const SetSpecTree::ConstItem & t) :
            env(e),
            target(t),
            matched(false)
        {
        }

        bool operator() (const PackageDatabaseEntry & e)
        {
            dbe = &e;
            matched = false;
            target.accept(*this);
            return matched;
        }

        void visit_sequence(const AllDepSpec &,
                SetSpecTree::ConstSequenceIterator begin,
                SetSpecTree::ConstSequenceIterator end)
        {
            if (matched)
                return;

            std::for_each(begin, end, accept_visitor(*this));
        }

        void visit_leaf(const PackageDepSpec & a)
        {
            if (matched)
                return;

            if (match_package(env, a, *dbe))
                matched = true;
        }
    };
}

bool
paludis::match_package_in_set(
        const Environment & env,
        const SetSpecTree::ConstItem & target,
        const PackageDatabaseEntry & entry)
{
    IsInHeirarchy h(env, target);
    return h(entry);
}

