/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh
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
#include <paludis/dep_spec_flattener.hh>
#include <paludis/environment.hh>
#include <paludis/version_requirements.hh>
#include <paludis/use_requirements.hh>
#include <paludis/package_database.hh>
#include <paludis/package_id.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/tr1_functional.hh>
#include <algorithm>

using namespace paludis;

bool
paludis::match_package(
        const Environment & env,
        const PackageDepSpec & spec,
        const PackageID & entry)
{
    if (spec.package_ptr() && *spec.package_ptr() != entry.name())
        return false;

    if (spec.package_name_part_ptr() && *spec.package_name_part_ptr() != entry.name().package)
        return false;

    if (spec.category_name_part_ptr() && *spec.category_name_part_ptr() != entry.name().category)
        return false;

    if (spec.version_requirements_ptr())
        switch (spec.version_requirements_mode())
        {
            case vr_and:
                for (VersionRequirements::ConstIterator r(spec.version_requirements_ptr()->begin()),
                        r_end(spec.version_requirements_ptr()->end()) ; r != r_end ; ++r)
                    if (! r->version_operator.as_version_spec_comparator()(entry.version(), r->version_spec))
                        return false;
                break;

            case vr_or:
                {
                    bool matched(false);
                    for (VersionRequirements::ConstIterator r(spec.version_requirements_ptr()->begin()),
                            r_end(spec.version_requirements_ptr()->end()) ; r != r_end ; ++r)
                        if (r->version_operator.as_version_spec_comparator()(entry.version(), r->version_spec))
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
        if (*spec.repository_ptr() != entry.repository()->name())
            return false;

    if (spec.slot_ptr())
        if (*spec.slot_ptr() != entry.slot())
            return false;

    if (spec.use_requirements_ptr())
    {
        for (UseRequirements::ConstIterator u(spec.use_requirements_ptr()->begin()),
                u_end(spec.use_requirements_ptr()->end()) ; u != u_end ; ++u)
            if (! (*u)->satisfied_by(&env, entry))
                return false;
    }

    return true;
}

bool
paludis::match_package_in_set(
        const Environment & env,
        const SetSpecTree::ConstItem & target,
        const PackageID & entry)
{
    using namespace tr1::placeholders;

    DepSpecFlattener<SetSpecTree, PackageDepSpec> f(&env, entry);
    target.accept(f);
    return indirect_iterator(f.end()) != std::find_if(
            indirect_iterator(f.begin()), indirect_iterator(f.end()),
            tr1::bind(&match_package, tr1::cref(env), _1, tr1::cref(entry)));
}

