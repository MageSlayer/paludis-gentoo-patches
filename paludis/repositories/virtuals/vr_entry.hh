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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_VIRTUALS_VR_ENTRY_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_VIRTUALS_VR_ENTRY_HH 1

#include <paludis/name.hh>
#include <paludis/version_spec.hh>
#include <paludis/package_database.hh>
#include <paludis/environment.hh>

namespace paludis
{

#include <paludis/repositories/virtuals/vr_entry-sr.hh>

    /**
     * Sort ordering for _imp->entries for an InstalledVirtualsRepository.
     *
     * \ingroup grpvirtualsrepository
     * \nosubgrouping
     */
    struct EntriesComparator
    {
        /// Our PackageDatabase instance.
        std::tr1::shared_ptr<const PackageDatabase> const db;

        ///\name Basic operations
        ///\{

        EntriesComparator(std::tr1::shared_ptr<const PackageDatabase> const d) :
            db(d)
        {
        }

        ///\}

        ///\name Comparison operators
        ///\{

        bool
        operator() (const VREntry & a, const VREntry & b) const
        {
            if (a < b)
                return true;
            if (a > b)
                return false;

            if (a.provided_by_repository != b.provided_by_repository)
            {
                // not a bug
                if (db->more_important_than(a.provided_by_repository, b.provided_by_repository))
                    return true;
                return false;
            }

            return a.provided_by_name > b.provided_by_name; // not a bug either
        }

        ///\}
    };

    /**
     * Comparison on name only for an VREntry.
     *
     * \ingroup grpvirtualsrepository
     * \nosubgrouping
     */
    struct EntriesNameComparator
    {
        ///\name Comparison operators
        ///\{

        bool
        operator() (const VREntry & a, const VREntry & b) const
        {
            return a.virtual_name < b.virtual_name;
        }

        ///\}
    };

    /**
     * Extract only the name for an VREntry.
     *
     * \ingroup grpvirtualsrepository
     * \nosubgrouping
     */
    struct EntriesNameExtractor
    {
        ///\name Extraction function
        ///\{

        QualifiedPackageName
        operator() (const VREntry & a) const
        {
            return a.virtual_name;
        }

        ///\}
    };

    /**
     * Comparison on category name only for an VREntry.
     *
     * \ingroup grpvirtualsrepository
     * \nosubgrouping
     */
    struct EntriesCategoryComparator
    {
        ///\name Comparison operators
        ///\{

        bool
        operator() (const VREntry & a, const VREntry & b) const
        {
            return a.virtual_name.category < b.virtual_name.category;
        }

        ///\}
    };

    /**
     * Extract only the category name for an VREntry.
     *
     * \ingroup grpvirtualsrepository
     * \nosubgrouping
     */
    struct EntriesCategoryExtractor
    {
        ///\name Extraction function
        ///\{

        CategoryNamePart
        operator() (const VREntry & a) const
        {
            return a.virtual_name.category;
        }

        ///\}
    };
}

#endif
