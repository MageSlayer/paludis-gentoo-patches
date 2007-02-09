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

#include <paludis/util/collection.hh>
#include <paludis/name.hh>
#include <paludis/package_database_entry.hh>

namespace paludis
{
    class Environment;
    class PackageDepAtom;

    class QueryDelegate
    {
        protected:
            QueryDelegate();

        public:
            virtual ~QueryDelegate();

            virtual std::tr1::shared_ptr<RepositoryNameCollection> repositories(const Environment &) const;

            virtual std::tr1::shared_ptr<CategoryNamePartCollection> categories(const Environment &,
                    std::tr1::shared_ptr<const RepositoryNameCollection>) const;

            virtual std::tr1::shared_ptr<QualifiedPackageNameCollection> packages(const Environment &,
                    std::tr1::shared_ptr<const RepositoryNameCollection>,
                    std::tr1::shared_ptr<const CategoryNamePartCollection>) const;

            virtual std::tr1::shared_ptr<PackageDatabaseEntryCollection> versions(const Environment &,
                    std::tr1::shared_ptr<const RepositoryNameCollection>,
                    std::tr1::shared_ptr<const QualifiedPackageNameCollection>) const;
    };

    class Query
    {
        friend Query operator& (const Query &, const Query &);

        private:
            std::tr1::shared_ptr<const QueryDelegate> _d;

        protected:
            Query(std::tr1::shared_ptr<const QueryDelegate>);

        public:
            ~Query();

            std::tr1::shared_ptr<RepositoryNameCollection> repositories(const Environment & e) const
            {
                return _d->repositories(e);
            }

            std::tr1::shared_ptr<CategoryNamePartCollection> categories(const Environment & e,
                    std::tr1::shared_ptr<const RepositoryNameCollection> r) const
            {
                return _d->categories(e, r);
            }

            std::tr1::shared_ptr<QualifiedPackageNameCollection> packages(const Environment & e,
                    std::tr1::shared_ptr<const RepositoryNameCollection> r,
                    std::tr1::shared_ptr<const CategoryNamePartCollection> c) const
            {
                return _d->packages(e, r, c);
            }

            std::tr1::shared_ptr<PackageDatabaseEntryCollection> versions(const Environment & e,
                    std::tr1::shared_ptr<const RepositoryNameCollection> r,
                    std::tr1::shared_ptr<const QualifiedPackageNameCollection> q) const
            {
                return _d->versions(e, r, q);
            }
    };

    namespace query
    {
        class Matches :
            public Query
        {
            public:
                Matches(const PackageDepAtom &);
        };

        class Package :
            public Query
        {
            public:
                Package(const QualifiedPackageName &);
        };

        class NotMasked :
            public Query
        {
            public:
                NotMasked();
        };

        class RepositoryHasInstalledInterface :
            public Query
        {
            public:
                RepositoryHasInstalledInterface();
        };

        class RepositoryHasInstallableInterface :
            public Query
        {
            public:
                RepositoryHasInstallableInterface();
        };

        class RepositoryHasUninstallableInterface :
            public Query
        {
            public:
                RepositoryHasUninstallableInterface();
        };
    }

    Query operator& (const Query &, const Query &);
}

#endif
