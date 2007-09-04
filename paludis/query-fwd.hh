/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_QUERY_FWD_HH
#define PALUDIS_GUARD_PALUDIS_QUERY_FWD_HH 1

#include <iosfwd>
#include <paludis/util/attributes.hh>

namespace paludis
{
    class QueryDelegate;
    class Query;

    namespace query
    {
        class Matches;
        class Package;
        class Repository;
        class Category;
        class NotMasked;
        template <typename A_> class SupportsAction;
        class InstalledAtRoot;
        class All;
    }

    /**
     * Create a Query that returns packages for which both Query parameters
     * hold.
     *
     * \see Query
     * \see PackageDatabase::query
     * \ingroup grpquery
     */
    Query operator& (const Query &, const Query &) PALUDIS_VISIBLE;

    /**
     * Output a human-readable description of a Query.
     *
     * \see Query
     * \ingroup grpquery
     */
    std::ostream & operator<< (std::ostream &, const Query &) PALUDIS_VISIBLE;
}

#endif
