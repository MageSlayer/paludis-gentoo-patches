/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010 Ciaran McCreesh
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

#include <paludis/package_dep_spec_collection.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/package_id.hh>
#include <paludis/dep_spec.hh>
#include <paludis/match_package.hh>
#include <list>
#include <map>

using namespace paludis;

namespace paludis
{
    template <>
    struct Imp<PackageDepSpecCollection>
    {
        std::multimap<QualifiedPackageName, PackageDepSpec> by_name;
        std::list<PackageDepSpec> unnamed;
    };
}

PackageDepSpecCollection::PackageDepSpecCollection() :
    Pimp<PackageDepSpecCollection>()
{
}

PackageDepSpecCollection::~PackageDepSpecCollection() = default;

void
PackageDepSpecCollection::insert(const PackageDepSpec & spec)
{
    if (spec.package_ptr())
        _imp->by_name.insert(std::make_pair(*spec.package_ptr(), spec));
    else
        _imp->unnamed.push_back(spec);
}

bool
PackageDepSpecCollection::match_any(
        const Environment * const env,
        const std::shared_ptr<const PackageID> & id,
        const MatchPackageOptions & opts) const
{
    auto named(_imp->by_name.equal_range(id->name()));
    for ( ; named.first != named.second ; ++named.first)
        if (match_package(*env, named.first->second, *id, opts))
            return true;

    for (auto u(_imp->unnamed.begin()), u_end(_imp->unnamed.end()) ;
            u != u_end ; ++u)
        if (match_package(*env, *u, *id, opts))
            return true;

    return false;
}

template class Pimp<PackageDepSpecCollection>;

