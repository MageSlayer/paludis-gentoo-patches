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

#include <paludis/hashed_containers.hh>
#include <paludis/name.hh>
#include <paludis/version_spec.hh>
#include <paludis/package_id.hh>
#include <paludis/repository.hh>

using namespace paludis;

#if defined(PALUDIS_HASH_IS_STD_TR1_UNORDERED) || defined(PALUDIS_HASH_IS_GNU_CXX_HASH) || defined(PALUDIS_HASH_IS_STD_HASH)

std::size_t
CRCHash<QualifiedPackageName>::operator() (const QualifiedPackageName & val) const
{
    const std::string & s1(val.category.data()), s2(val.package.data());
    std::size_t h(0);

    for (std::string::size_type t(0) ; t < s1.length() ; ++t)
    {
        std::size_t hh(h & h_mask);
        h <<= 5;
        h ^= (hh >> h_shift);
        h ^= s1[t];
    }

    for (std::string::size_type t(0) ; t < s2.length() ; ++t)
    {
        std::size_t hh(h & h_mask);
        h <<= 5;
        h ^= (hh >> h_shift);
        h ^= s2[t];
    }

    return h;
}

std::size_t
CRCHash<std::string>::operator() (const std::string & val) const
{
    std::size_t h(0);

    for (std::string::size_type t(0) ; t < val.length() ; ++t)
    {
        std::size_t hh(h & h_mask);
        h <<= 5;
        h ^= (hh >> h_shift);
        h ^= val[t];
    }

    return h;
}

std::size_t
CRCHash<std::pair<QualifiedPackageName, VersionSpec> >::operator() (
        const std::pair<QualifiedPackageName, VersionSpec> & val) const
{
    const std::string & s1(val.first.category.data()),
          s2(val.first.package.data());

    std::size_t h(0);

    for (std::string::size_type t(0) ; t < s1.length() ; ++t)
    {
        std::size_t hh(h & h_mask);
        h <<= 5;
        h ^= (hh >> h_shift);
        h ^= s1[t];
    }

    for (std::string::size_type t(0) ; t < s2.length() ; ++t)
    {
        std::size_t hh(h & h_mask);
        h <<= 5;
        h ^= (hh >> h_shift);
        h ^= s2[t];
    }

    h ^= val.second.hash_value();

    return h;
}

std::size_t
CRCHash<PackageID>::operator() (const PackageID & val) const
{
    return
        CRCHash<QualifiedPackageName>()(val.name()) ^
        (val.version().hash_value() << 5) ^
        (CRCHash<RepositoryName>()(val.repository()->name()) << 9) ^
        (val.extra_hash_value() << 13);
}

#if (! defined(PALUDIS_HASH_IS_STD_TR1_UNORDERED)) && (! defined(PALUDIS_HASH_IS_GNU_CXX_HASH))

bool
CRCHash<PackageID, PackageID>::operator() (const PackageID & i1, const PackageID & i2) const
{
    return PackageIDSetComparator()(i1, i2);
}

#endif
#endif

