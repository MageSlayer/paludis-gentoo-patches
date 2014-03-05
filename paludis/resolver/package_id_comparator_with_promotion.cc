/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
 * Copyright (c) 2014 Dimitry Ishenko
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

#include <paludis/resolver/package_id_comparator_with_promotion.hh>
#include <paludis/resolver/get_sameness.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/name.hh>
#include <paludis/version_spec.hh>
#include <paludis/repository.hh>
#include <paludis/environment.hh>

#include <paludis/util/pimp-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/hashes.hh>

#include <memory>
#include <unordered_map>

using namespace paludis;
using namespace paludis::resolver;

namespace paludis
{
    template <>
    struct Imp<PackageIDComparatorWithPromotion>
    {
        std::unordered_map<RepositoryName, unsigned, Hash<RepositoryName> > m;
    };
}

PackageIDComparatorWithPromotion::PackageIDComparatorWithPromotion(const Environment * const e) :
    _imp()
{
    unsigned c(0);
    for (auto r(e->begin_repositories()), r_end(e->end_repositories()) ; r != r_end ; ++r)
        _imp->m.insert(std::make_pair((*r)->name(), ++c));
}

PackageIDComparatorWithPromotion::PackageIDComparatorWithPromotion(const PackageIDComparatorWithPromotion & other) :
    _imp()
{
    _imp->m = other._imp->m;
}

PackageIDComparatorWithPromotion::~PackageIDComparatorWithPromotion()
{
}

bool
PackageIDComparatorWithPromotion::operator() (const std::shared_ptr<const PackageID> & a,
        const std::shared_ptr<const PackageID> & b) const
{
    if (a->name() < b->name())
        return true;

    if (a->name() > b->name())
        return false;

    if (a->version() < b->version())
        return true;

    if (a->version() > b->version())
        return false;

    bool a_bin(false), b_bin(false);
    if (a->behaviours_key()) a_bin = a->behaviours_key()->parse_value()->count("binary");
    if (b->behaviours_key()) b_bin = b->behaviours_key()->parse_value()->count("binary");

    if ( (a_bin != b_bin) && get_sameness(a, b)[epia_is_same] )
        return b_bin;

    auto ma(_imp->m.find(a->repository_name())), mb(_imp->m.find(b->repository_name()));

    if (ma == _imp->m.end() || mb == _imp->m.end())
        throw InternalError(PALUDIS_HERE, "Repository not in database");

    if (ma->second < mb->second)
        return true;
    if (ma->second > mb->second)
        return false;

    return a->arbitrary_less_than_comparison(*b);
}
