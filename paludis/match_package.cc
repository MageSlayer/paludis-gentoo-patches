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
paludis::match_package(
        const Environment & env,
        const PackageDepSpec & spec,
        const PackageDatabaseEntry & entry)
{
    if (spec.package_ptr() && *spec.package_ptr() != entry.name)
        return false;

    if (spec.version_requirements_ptr())
        switch (spec.version_requirements_mode())
        {
            case vr_and:
                for (VersionRequirements::Iterator r(spec.version_requirements_ptr()->begin()),
                        r_end(spec.version_requirements_ptr()->end()) ; r != r_end ; ++r)
                    if (! (((entry.version).*(r->version_operator.as_version_spec_operator()))(r->version_spec)))
                        return false;
                break;

            case vr_or:
                {
                    bool matched(false);
                    for (VersionRequirements::Iterator r(spec.version_requirements_ptr()->begin()),
                            r_end(spec.version_requirements_ptr()->end()) ; r != r_end ; ++r)
                        if ((((entry.version).*(r->version_operator.as_version_spec_operator()))(r->version_spec)))
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
        std::tr1::shared_ptr<const VersionMetadata> metadata(env.package_database()->fetch_repository(
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
                        if (! env.query_use(u->first, &entry))
                            return false;
                        continue;

                    case use_disabled:
                        if (env.query_use(u->first, &entry))
                            return false;
                        continue;
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
        DepSpecVisitorTypes::ConstVisitor,
        std::unary_function<PackageDatabaseEntry, bool>
    {
        const Environment & env;
        const DepSpec & target;
        const PackageDatabaseEntry * dbe;
        bool matched;

        IsInHeirarchy(const Environment & e, const DepSpec & t) :
            env(e),
            target(t),
            matched(false)
        {
        }

        bool operator() (const PackageDatabaseEntry & e)
        {
            dbe = &e;
            matched = false;
            target.accept(this);
            return matched;
        }

        void visit(const AllDepSpec * const a)
        {
            if (matched)
                return;

            std::for_each(a->begin(), a->end(), accept_visitor(this));
        }

        void visit(const PackageDepSpec * const a)
        {
            if (matched)
                return;

            if (match_package(env, *a, *dbe))
                matched = true;
        }

        void visit(const UseDepSpec * const u)
        {
            if (matched)
                return;

            std::for_each(u->begin(), u->end(), accept_visitor(this));
        }

        void visit(const AnyDepSpec * const a)
        {
            if (matched)
                return;

            std::for_each(a->begin(), a->end(), accept_visitor(this));
        }

        void visit(const BlockDepSpec * const)
        {
        }

        void visit(const PlainTextDepSpec * const) PALUDIS_ATTRIBUTE((noreturn))
        {
            throw InternalError(PALUDIS_HERE, "Got PlainTextDepSpec?");
        }
    };
}

bool
paludis::match_package_in_heirarchy(
        const Environment & env,
        const DepSpec & spec,
        const PackageDatabaseEntry & entry)
{
    IsInHeirarchy h(env, spec);
    return h(entry);
}

