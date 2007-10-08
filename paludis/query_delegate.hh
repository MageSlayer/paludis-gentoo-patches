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

#ifndef PALUDIS_GUARD_PALUDIS_QUERY_DELEGATE_HH
#define PALUDIS_GUARD_PALUDIS_QUERY_DELEGATE_HH 1

#include <paludis/name-fwd.hh>
#include <paludis/query_delegate-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/query_delegate-fwd.hh>

/** \file
 * Declarations for QueryDelegate.
 *
 * \ingroup g_query
 *
 * \section Examples
 *
 * - \ref example_query_delegate.cc "example_query_delegate.cc"
 * - \ref example_query.cc "example_query.cc"
 * - \ref example_match_package.cc "example_match_package.cc"
 */

namespace paludis
{
    /**
     * A QueryDelegate subclass is used by Query to provide the information
     * needed by PackageDatabase::query.
     *
     * \see Query
     * \ingroup g_query
     */
    class PALUDIS_VISIBLE QueryDelegate
    {
        protected:
            ///\name Basic operations
            ///\{

            QueryDelegate();

        public:
            virtual ~QueryDelegate();

            ///\}

            /**
             * Fetch the names of repositories potentially containing matches.
             * All returned repositories must exist.
             *
             * Default behaviour: return all repositories.
             */
            virtual tr1::shared_ptr<RepositoryNameSequence> repositories(const Environment &) const;

            /**
             * Fetch the names of categories potentially containing matches.
             *
             * Default behaviour: return all categories in the provided
             * repository collection.
             */
            virtual tr1::shared_ptr<CategoryNamePartSet> categories(const Environment &,
                    tr1::shared_ptr<const RepositoryNameSequence>) const;

            /**
             * Fetch the names of packages potentially containing matches.
             *
             * Default behaviour: return all packages in the provided repository
             * in the provided categories.
             *
             * Note that some entries in the categories collection (but not in
             * the repositories collection) may not exist.
             */
            virtual tr1::shared_ptr<QualifiedPackageNameSet> packages(const Environment &,
                    tr1::shared_ptr<const RepositoryNameSequence>,
                    tr1::shared_ptr<const CategoryNamePartSet>) const;

            /**
             * Fetch the IDs of matching packages.
             *
             * Default behaviour: return all IDs in the provided packages.
             *
             * Note that some entries in the qualified package name collection
             * (but not in the repositories collection) may not exist.
             */
            virtual tr1::shared_ptr<PackageIDSequence> ids(const Environment &,
                    tr1::shared_ptr<const RepositoryNameSequence>,
                    tr1::shared_ptr<const QualifiedPackageNameSet>) const;

            /**
             * Fetch a string representation of our query.
             */
            virtual std::string as_human_readable_string() const = 0;
    };
}

#endif
