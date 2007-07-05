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

#ifndef PALUDIS_GUARD_PALUDIS_QUERY_HH
#define PALUDIS_GUARD_PALUDIS_QUERY_HH 1

#include <paludis/name.hh>
#include <paludis/package_id.hh>
#include <iosfwd>

/** \file
 * Query and related classes.
 *
 * \ingroup grpquery
 */

namespace paludis
{
    class Environment;
    class PackageDepSpec;
    class FSEntry;

    /**
     * A QueryDelegate subclass is used by Query to provide the information
     * needed by PackageDatabase::query.
     *
     * \see Query
     * \ingroup grpquery
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

    /**
     * Parameter for a PackageDatabase query.
     *
     * Holds a QueryDelegate to perform actual operations, so that it can be
     * copied without splicing problems.
     *
     * \see QueryDelegate
     * \see PackageDatabase::query
     * \ingroup grpquery
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE Query
    {
        friend Query operator& (const Query &, const Query &);
        friend std::ostream & operator<< (std::ostream &, const Query &);

        private:
            tr1::shared_ptr<const QueryDelegate> _d;

        protected:
            ///\name Basic operations
            ///\{

            Query(tr1::shared_ptr<const QueryDelegate>);

        public:
            ~Query();

            ///\}

            ///\name Delegate-implemented functions
            ///\{

            tr1::shared_ptr<RepositoryNameSequence> repositories(const Environment & e) const
            {
                return _d->repositories(e);
            }

            tr1::shared_ptr<CategoryNamePartSet> categories(const Environment & e,
                    tr1::shared_ptr<const RepositoryNameSequence> r) const
            {
                return _d->categories(e, r);
            }

            tr1::shared_ptr<QualifiedPackageNameSet> packages(const Environment & e,
                    tr1::shared_ptr<const RepositoryNameSequence> r,
                    tr1::shared_ptr<const CategoryNamePartSet> c) const
            {
                return _d->packages(e, r, c);
            }

            tr1::shared_ptr<PackageIDSequence> ids(const Environment & e,
                    tr1::shared_ptr<const RepositoryNameSequence> r,
                    tr1::shared_ptr<const QualifiedPackageNameSet> q) const
            {
                return _d->ids(e, r, q);
            }

            ///\}
    };

    /**
     * Various Query classes.
     *
     * \see Query
     * \ingroup grpquery
     */
    namespace query
    {
        /**
         * Fetch packages matching a given PackageDepSpec.
         *
         * \see Query
         * \see PackageDatabase::query
         * \ingroup grpquery
         * \nosubgrouping
         */
        class PALUDIS_VISIBLE Matches :
            public Query
        {
            public:
                ///\name Basic operations
                ///\{

                Matches(const PackageDepSpec &);

                ///\}
        };

        /**
         * Fetch packages with a given package name.
         *
         * \see Query
         * \see PackageData
         * \ingroup grpquerybase::query
         * \nosubgrouping
         */
        class PALUDIS_VISIBLE Package :
            public Query
        {
            public:
                ///\name Basic operations
                ///\{

                Package(const QualifiedPackageName &);

                ///\}
        };

        /**
         * Fetch packages in a given repository.
         *
         * \see Query
         * \see PackageData
         * \ingroup grpquerybase::query
         * \nosubgrouping
         */
        class PALUDIS_VISIBLE Repository :
            public Query
        {
            public:
                ///\name Basic operations
                ///\{

                Repository(const RepositoryName &);

                ///\}
        };

        /**
         * Fetch packages in a given category.
         *
         * \see Query
         * \see PackageData
         * \ingroup grpquerybase::query
         * \nosubgrouping
         */
        class PALUDIS_VISIBLE Category :
            public Query
        {
            public:
                ///\name Basic operations
                ///\{

                Category(const CategoryNamePart &);

                ///\}
        };

        /**
         * Fetch packages that are not masked.
         *
         * \see Query
         * \see PackageData
         * \ingroup grpquerybase::query
         * \nosubgrouping
         */
        class PALUDIS_VISIBLE NotMasked :
            public Query
        {
            public:
                ///\name Basic operations
                ///\{

                NotMasked();

                ///\}
        };

        /**
         * Fetch packages from a repository that has
         * RepositoryInstalledInterface.
         *
         * \see Query
         * \see PackageData
         * \ingroup grpquerybase::query
         * \nosubgrouping
         */
        class PALUDIS_VISIBLE RepositoryHasInstalledInterface :
            public Query
        {
            public:
                ///\name Basic operations
                ///\{

                RepositoryHasInstalledInterface();

                ///\}
        };

        /**
         * Fetch packages from a repository that has
         * RepositoryInstallableInterface.
         *
         * \see PackageDatabase::query
         * \see Query
         * \ingroup grpquery
         * \nosubgrouping
         */
        class PALUDIS_VISIBLE RepositoryHasInstallableInterface :
            public Query
        {
            public:
                ///\name Basic operations
                ///\{

                RepositoryHasInstallableInterface();

                ///\}
        };

        /**
         * Fetch packages from a repository that has
         * RepositoryUninstallableInterface.
         *
         * \see PackageDatabase::query
         * \see Query
         * \ingroup grpquery
         * \nosubgrouping
         */
        class PALUDIS_VISIBLE RepositoryHasUninstallableInterface :
            public Query
        {
            public:
                ///\name Basic operations
                ///\{

                RepositoryHasUninstallableInterface();

                ///\}
        };

        /**
         * Fetch packages that are installed at a particular root.
         *
         * \see Query
         * \see PackageDatabase::query
         * \ingroup grpquery
         * \nosubgrouping
         */
        class PALUDIS_VISIBLE InstalledAtRoot :
            public Query
        {
            public:
                ///\name Basic operations
                ///\{

                InstalledAtRoot(const FSEntry &);

                ///}
        };

        /**
         * Fetch all packages.
         *
         * \see Query
         * \see PackageDatabase::query
         * \ingroup grpquery
         * \nosubgrouping
         */
        class PALUDIS_VISIBLE All :
            public Query
        {
            public:
                ///\name Basic operations
                ///\{

                All();

                ///}
        };
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
