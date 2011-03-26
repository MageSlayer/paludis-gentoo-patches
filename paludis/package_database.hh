/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_PACKAGE_DATABASE_HH
#define PALUDIS_GUARD_PALUDIS_PACKAGE_DATABASE_HH 1

#include <paludis/package_database-fwd.hh>
#include <paludis/dep_spec.hh>
#include <paludis/name.hh>
#include <paludis/repository.hh>
#include <paludis/selection-fwd.hh>
#include <paludis/filter-fwd.hh>
#include <paludis/version_spec.hh>
#include <paludis/contents.hh>
#include <paludis/environment.hh>

#include <paludis/util/exception.hh>
#include <paludis/util/join.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/wrapped_forward_iterator.hh>

#include <iosfwd>

/** \file
 * Declarations for PackageDatabase.
 *
 * \ingroup g_package_database
 *
 * \section Examples
 *
 * - \ref example_package_database.cc "example_package_database.cc"
 */

namespace paludis
{
    /**
     * A PackageDatabase, which is owned by an Environment, contains a number of
     * Repository instances and supports various querying methods.
     *
     * \ingroup g_package_database
     */
    class PALUDIS_VISIBLE PackageDatabase
    {
        friend class EnvironmentImplementation;
        friend class WrappedForwardIteratorTraits<Environment::RepositoryConstIteratorTag>;

        private:
            Pimp<PackageDatabase> _imp;

            static const Filter & all_filter() PALUDIS_ATTRIBUTE((warn_unused_result));

        public:
            ~PackageDatabase();

        protected:
            ///\name Basic operations
            ///\{

            explicit PackageDatabase(const Environment * const);

            PackageDatabase(const PackageDatabase &) = delete;
            PackageDatabase & operator= (const PackageDatabase &) = delete;

            ///\}

            /**
             * Add a repository.
             *
             * \exception DuplicateRepositoryError if a Repository with the
             * same name as the new Repository already exists in our
             * collection.
             */
            void add_repository(int importance, const std::shared_ptr<Repository>);

            /**
             * Fetch a named repository.
             */
            std::shared_ptr<const Repository> fetch_repository(const RepositoryName &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Fetch a named repository.
             */
            std::shared_ptr<Repository> fetch_repository(const RepositoryName &)
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Do we have a named repository?
             */
            bool has_repository_named(const RepositoryName &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Disambiguate a package name.  If a filter is specified,
             * limit the potential results to packages that match.
             *
             * \throw AmbiguousPackageNameError if there is no unambiguous
             * disambiguation. If disambiguate is set to false, the
             * exception will be always thrown in presence of ambiguity.
             * \since 0.56 takes the disambiguate flag.
             */
            QualifiedPackageName fetch_unique_qualified_package_name(
                    const PackageNamePart &, const Filter & = all_filter(), const bool disambiguate = true) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Return true if the first repository is more important than the second.
             */
            bool more_important_than(const RepositoryName &, const RepositoryName &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            ///\name Iterate over our repositories
            ///\{

            struct RepositoryConstIteratorTag;
            typedef WrappedForwardIterator<RepositoryConstIteratorTag, const std::shared_ptr<Repository> > RepositoryConstIterator;

            RepositoryConstIterator begin_repositories() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            RepositoryConstIterator end_repositories() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            ///\}
    };

    extern template class WrappedForwardIterator<PackageDatabase::RepositoryConstIteratorTag, const std::shared_ptr<Repository> >;
}

#endif
