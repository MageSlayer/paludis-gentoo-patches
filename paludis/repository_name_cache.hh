/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORY_NAME_CACHE_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORY_NAME_CACHE_HH 1

#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/name.hh>
#include <memory>

/** \file
 * Declarations for RepositoryNameCache, which is used by some Repository
 * subclasses to implement a names class.
 *
 * \ingroup g_repository
 *
 * \section Examples
 *
 * - None at this time.
 */

namespace paludis
{
    class FSEntry;
    class Repository;

    /**
     * Used by various Repository subclasses to implement a names cache.
     *
     * \see Repository
     * \ingroup g_repository
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE RepositoryNameCache :
        private PrivateImplementationPattern<RepositoryNameCache>
    {
        public:
            ///\name Basic operations
            ///\{

            RepositoryNameCache(
                    const FSEntry & location,
                    const Repository * const repo);

            virtual ~RepositoryNameCache();


            ///\}

            ///\name Cache helper functions
            ///\{

            /**
             * Implement category_names_containing_package.
             *
             * May return a zero pointer, in which case the repository should
             * fall back to Repository::do_category_names_containing_package or
             * its own implementation.
             */
            std::shared_ptr<const CategoryNamePartSet> category_names_containing_package(
                    const PackageNamePart & p) const;

            /**
             * Whether or not our cache is usable.
             *
             * Initially this will be true. After the first query the value may
             * change to false (the query will return a zero pointer too).
             */
            bool usable() const PALUDIS_ATTRIBUTE((nothrow));

            /**
             * Implement cache regeneration.
             */
            void regenerate_cache() const;

            /**
             * Add a new package to the cache.
             */
            void add(const QualifiedPackageName &);

            /**
             * Remove a package from the cache.
             */
            void remove(const QualifiedPackageName &);

            ///\}
    };
}

#endif
