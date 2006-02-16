/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

#include "hashed_containers.hh"

using namespace paludis;

#if PALUDIS_HAVE_TR1_HASHES || PALUDIS_HAVE_EXT_HASHES || PALUDIS_HAVE_STD_HASHES

std::size_t
CRCHash<QualifiedPackageName>::operator() (const QualifiedPackageName & val) const
{
    const std::string & s1(val.get<qpn_category>().data()), s2(val.get<qpn_package>().data());
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
CRCHash<std::pair<QualifiedPackageName, VersionSpec> >::operator() (
        const std::pair<QualifiedPackageName, VersionSpec> & val) const
{
    const std::string & s1(val.first.get<qpn_category>().data()),
          s2(val.first.get<qpn_package>().data());

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

#endif

