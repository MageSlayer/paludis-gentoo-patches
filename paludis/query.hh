/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008 Ciaran McCreesh
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

#include <paludis/name-fwd.hh>
#include <paludis/query-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/dep_spec-fwd.hh>
#include <paludis/query_delegate-fwd.hh>
#include <paludis/util/fs_entry-fwd.hh>
#include <iosfwd>

/** \file
 * Declarations for Query and the various query:: classes.
 *
 * \ingroup g_query
 *
 * \section Examples
 *
 * - \ref example_query.cc "example_query.cc"
 * - \ref example_query_delegate.cc "example_query_delegate.cc"
 * - \ref example_match_package.cc "example_match_package.cc"
 */

namespace paludis
{
    /**
     * Parameter for a PackageDatabase query.
     *
     * Holds a QueryDelegate to perform actual operations, so that it can be
     * copied without splicing problems.
     *
     * \see QueryDelegate
     * \see PackageDatabase::query
     * \ingroup g_query
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE Query
    {
        friend Query operator& (const Query &, const Query &);
        friend std::ostream & operator<< (std::ostream &, const Query &);

        private:
            std::tr1::shared_ptr<const QueryDelegate> _d;

        protected:
            ///\name Basic operations
            ///\{

            Query(std::tr1::shared_ptr<const QueryDelegate>);

        public:
            ~Query();

            ///\}

            ///\name Delegate-implemented functions
            ///\{

            std::tr1::shared_ptr<RepositoryNameSequence> repositories(const Environment & e) const;

            std::tr1::shared_ptr<CategoryNamePartSet> categories(const Environment & e,
                    std::tr1::shared_ptr<const RepositoryNameSequence> r) const;

            std::tr1::shared_ptr<QualifiedPackageNameSet> packages(const Environment & e,
                    std::tr1::shared_ptr<const RepositoryNameSequence> r,
                    std::tr1::shared_ptr<const CategoryNamePartSet> c) const;

            std::tr1::shared_ptr<PackageIDSequence> ids(const Environment & e,
                    std::tr1::shared_ptr<const RepositoryNameSequence> r,
                    std::tr1::shared_ptr<const QualifiedPackageNameSet> q) const;

            ///\}
    };

    /**
     * Various Query classes.
     *
     * \see Query
     * \ingroup g_query
     */
    namespace query
    {
        /**
         * Fetch packages matching a given PackageDepSpec.
         *
         * \see Query
         * \see PackageDatabase::query
         * \ingroup g_query
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
         * \see PackageDatabase::query
         * \ingroup g_query
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
         * \see PackageDatabase::query
         * \ingroup g_query
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
         * \see PackageDatabase::query
         * \ingroup g_query
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
         * \see PackageDatabase::query
         * \ingroup g_query
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
         * Fetch packages that support a particular action.
         *
         * \see Query
         * \see PackageDatabase::query
         * \ingroup g_query
         * \nosubgrouping
         */
        template <typename A_>
        class PALUDIS_VISIBLE SupportsAction :
            public Query
        {
            public:
                ///\name Basic operations
                ///\{

                SupportsAction();

                ///\}
        };

        /**
         * Fetch packages that maybe support a particular action.
         *
         * A full SupportsAction<> on an ebuild ID requires a metadata load,
         * since unsupported EAPIs don't support any actions. MaybeSupportsAction,
         * on the other hand, only uses Repository::some_ids_might_support_action,
         * so it does not incur a penalty but may return additional results.
         *
         * \see Query
         * \see PackageDatabase::query
         * \ingroup g_query
         * \nosubgrouping
         */
        template <typename A_>
        class PALUDIS_VISIBLE MaybeSupportsAction :
            public Query
        {
            public:
                ///\name Basic operations
                ///\{

                MaybeSupportsAction();

                ///\}
        };

        /**
         * Fetch packages that are installed at a particular root.
         *
         * \see Query
         * \see PackageDatabase::query
         * \ingroup g_query
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
         * \ingroup g_query
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
}

#endif
