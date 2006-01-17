/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

#include <paludis/qualified_package_name.hh>
#include <paludis/version_spec.hh>
#include <paludis/repository.hh>
#include <paludis/smart_record.hh>
#include <ostream>

/** \file
 * Declarations for the PackageDatabaseEntry class.
 *
 * \ingroup Database
 */

namespace paludis
{
    class PackageDatabase;

    /**
     * Keys in a PackageDatabaseEntry.
     */
    enum PackageDatabaseEntryKeys
    {
        pde_package,           ///< Our package
        pde_version,           ///< Our version
        pde_repository,        ///< Our repository
        last_pde               ///< Number of items
    };

    /**
     * Tag for a PackageDatabaseEntry.
     */
    struct PackageDatabaseEntryTag :
        SmartRecordTag<comparison_mode::FullComparisonTag, comparison_method::SmartRecordCompareByAllTag>,
        SmartRecordKeys<PackageDatabaseEntryKeys, last_pde>,
        SmartRecordKey<pde_package, QualifiedPackageName>,
        SmartRecordKey<pde_version, VersionSpec>,
        SmartRecordKey<pde_repository, RepositoryName>
    {
    };

    /**
     * A PackageDatabaseEntry holds a QualifiedPackageName, a VersionSpec and a
     * RepositoryName, and is fully comparable.
     */
    typedef MakeSmartRecord<PackageDatabaseEntryTag>::Type PackageDatabaseEntry;

    /**
     * A PackageDatabaseEntry can be written to a stream.
     */
    std::ostream & operator<< (std::ostream &, const PackageDatabaseEntry &);
}

#endif
