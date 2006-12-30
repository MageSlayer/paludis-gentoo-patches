/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_PACKAGE_DATABASE_ENTRY_HH
#define PALUDIS_GUARD_PALUDIS_PACKAGE_DATABASE_ENTRY_HH 1

#include <paludis/name.hh>
#include <paludis/version_spec.hh>

namespace paludis
{

#include <paludis/package_database_entry-sr.hh>

    /**
     * A collection of PackageDatabaseEntry instances.
     *
     * Often this will have been ordered by
     * PackageDatabase::sort_package_database_entry_collection . There is no intrinsic
     * ordering because RepositoryName is not comparable except via a PackageDatabase .
     */
    typedef SequentialCollection<PackageDatabaseEntry> PackageDatabaseEntryCollection;

    /**
     * Comparator for ArbitrarilyOrderedPackageDatabaseEntryCollection.
     */
    struct ArbitrarilyOrderedPackageDatabaseEntryCollectionComparator
    {
        bool operator () (const PackageDatabaseEntry & lhs, const PackageDatabaseEntry & rhs) const;
    };

    /**
     * A collection of PackageDatabaseEntry instances that are ordered, but not
     * by best first.
     */
    typedef SortedCollection<PackageDatabaseEntry, ArbitrarilyOrderedPackageDatabaseEntryCollectionComparator>
        ArbitrarilyOrderedPackageDatabaseEntryCollection;

    /**
     * A PackageDatabaseEntry can be written to a stream.
     */
    std::ostream & operator<< (std::ostream &, const PackageDatabaseEntry &);
}

#endif
