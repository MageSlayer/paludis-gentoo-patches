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

#include "query.hh"
#include <paludis/util/collection_concrete.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/package_database.hh>
#include <paludis/environment.hh>
#include <paludis/match_package.hh>
#include <algorithm>
#include <set>

using namespace paludis;

QueryDelegate::QueryDelegate()
{
}

QueryDelegate::~QueryDelegate()
{
}

std::tr1::shared_ptr<RepositoryNameCollection>
QueryDelegate::repositories(const Environment &) const
{
    return std::tr1::shared_ptr<RepositoryNameCollection>();
}

std::tr1::shared_ptr<CategoryNamePartCollection>
QueryDelegate::categories(const Environment &, std::tr1::shared_ptr<const RepositoryNameCollection>) const
{
    return std::tr1::shared_ptr<CategoryNamePartCollection>();
}

std::tr1::shared_ptr<QualifiedPackageNameCollection>
QueryDelegate::packages(const Environment &, std::tr1::shared_ptr<const RepositoryNameCollection>,
        std::tr1::shared_ptr<const CategoryNamePartCollection>) const
{
    return std::tr1::shared_ptr<QualifiedPackageNameCollection>();
}

std::tr1::shared_ptr<PackageDatabaseEntryCollection>
QueryDelegate::versions(const Environment &, std::tr1::shared_ptr<const RepositoryNameCollection>,
        std::tr1::shared_ptr<const QualifiedPackageNameCollection>) const
{
    return std::tr1::shared_ptr<PackageDatabaseEntryCollection>();
}

Query::Query(std::tr1::shared_ptr<const QueryDelegate> d) :
    _d(d)
{
}

Query::~Query()
{
}

namespace
{
    struct MatchesDelegate :
        QueryDelegate
    {
        const PackageDepSpec spec;

        MatchesDelegate(const PackageDepSpec & a) :
            spec(a)
        {
        }

        std::tr1::shared_ptr<RepositoryNameCollection>
        repositories(const Environment & e) const
        {
            if (spec.repository_ptr())
            {
                std::tr1::shared_ptr<RepositoryNameCollection> result(new RepositoryNameCollection::Concrete);

                for (PackageDatabase::RepositoryIterator i(e.package_database()->begin_repositories()),
                        i_end(e.package_database()->end_repositories()) ; i != i_end ; ++i)
                    if ((*i)->name() == *spec.repository_ptr())
                    {
                        result->push_back((*i)->name());
                        break;
                    }

                return result;
            }

            return QueryDelegate::repositories(e);
        }

        std::tr1::shared_ptr<CategoryNamePartCollection>
        categories(const Environment &,
                std::tr1::shared_ptr<const RepositoryNameCollection>) const
        {
            std::tr1::shared_ptr<CategoryNamePartCollection> result(new CategoryNamePartCollection::Concrete);
            result->insert(spec.package().category);
            return result;
        }

        std::tr1::shared_ptr<QualifiedPackageNameCollection>
        packages(const Environment &,
            std::tr1::shared_ptr<const RepositoryNameCollection>,
            std::tr1::shared_ptr<const CategoryNamePartCollection>) const
        {
            std::tr1::shared_ptr<QualifiedPackageNameCollection> result(new QualifiedPackageNameCollection::Concrete);
            result->insert(spec.package());
            return result;
        }

        std::tr1::shared_ptr<PackageDatabaseEntryCollection>
        versions(const Environment & e,
                std::tr1::shared_ptr<const RepositoryNameCollection> repos,
                std::tr1::shared_ptr<const QualifiedPackageNameCollection> pkgs) const
        {
            std::tr1::shared_ptr<PackageDatabaseEntryCollection> result(new PackageDatabaseEntryCollection::Concrete);
            for (RepositoryNameCollection::Iterator r(repos->begin()), r_end(repos->end()) ;
                     r != r_end ; ++r)
            {
                std::tr1::shared_ptr<const Repository> repo(e.package_database()->fetch_repository(*r));

                for (QualifiedPackageNameCollection::Iterator p(pkgs->begin()), p_end(pkgs->end()) ;
                        p != p_end ; ++p)
                {
                    std::tr1::shared_ptr<const VersionSpecCollection> vers(repo->version_specs(*p));
                    for (VersionSpecCollection::Iterator v(vers->begin()), v_end(vers->end()) ;
                            v != v_end ; ++v)
                    {
                        PackageDatabaseEntry dbe(*p, *v, *r);
                        if (match_package(e, spec, dbe))
                            result->push_back(dbe);
                    }
                }
            }

            return result;
        }
    };
}

query::Matches::Matches(const PackageDepSpec & a) :
    Query(std::tr1::shared_ptr<QueryDelegate>(new MatchesDelegate(a)))
{
}

query::Package::Package(const QualifiedPackageName & a) :
    Query(std::tr1::shared_ptr<QueryDelegate>(new MatchesDelegate(PackageDepSpec(a))))
{
}

namespace
{
    struct NotMaskedDelegate :
        QueryDelegate
    {
        std::tr1::shared_ptr<PackageDatabaseEntryCollection>
        versions(const Environment & e,
                std::tr1::shared_ptr<const RepositoryNameCollection> repos,
                std::tr1::shared_ptr<const QualifiedPackageNameCollection> pkgs) const
        {
            std::tr1::shared_ptr<PackageDatabaseEntryCollection> result(new PackageDatabaseEntryCollection::Concrete);
            for (RepositoryNameCollection::Iterator r(repos->begin()), r_end(repos->end()) ;
                     r != r_end ; ++r)
            {
                std::tr1::shared_ptr<const Repository> repo(e.package_database()->fetch_repository(*r));

                for (QualifiedPackageNameCollection::Iterator p(pkgs->begin()), p_end(pkgs->end()) ;
                        p != p_end ; ++p)
                {
                    std::tr1::shared_ptr<const VersionSpecCollection> vers(repo->version_specs(*p));
                    for (VersionSpecCollection::Iterator v(vers->begin()), v_end(vers->end()) ;
                            v != v_end ; ++v)
                    {
                        PackageDatabaseEntry dbe(*p, *v, *r);
                        if (! e.mask_reasons(dbe).any())
                            result->push_back(dbe);
                    }
                }
            }

            return result;
        }
    };
}

query::NotMasked::NotMasked() :
    Query(std::tr1::shared_ptr<QueryDelegate>(new NotMaskedDelegate))
{
}

namespace
{
    template <typename I_, I_ * (RepositoryCapabilities::* i_)>
    struct RepositoryHasDelegate :
        QueryDelegate
    {
        std::tr1::shared_ptr<RepositoryNameCollection>
        repositories(const Environment & e) const
        {
            std::tr1::shared_ptr<RepositoryNameCollection> result(new RepositoryNameCollection::Concrete);

            for (PackageDatabase::RepositoryIterator i(e.package_database()->begin_repositories()),
                    i_end(e.package_database()->end_repositories()) ; i != i_end ; ++i)
                if ((**i).*i_)
                    result->push_back((*i)->name());

            return result;
        }
    };
}

query::RepositoryHasInstalledInterface::RepositoryHasInstalledInterface() :
    Query(std::tr1::shared_ptr<QueryDelegate>(
                new RepositoryHasDelegate<RepositoryInstalledInterface, &RepositoryCapabilities::installed_interface>))
{
}

query::RepositoryHasInstallableInterface::RepositoryHasInstallableInterface() :
    Query(std::tr1::shared_ptr<QueryDelegate>(
                new RepositoryHasDelegate<RepositoryInstallableInterface, &RepositoryCapabilities::installable_interface>))
{
}

query::RepositoryHasUninstallableInterface::RepositoryHasUninstallableInterface() :
    Query(std::tr1::shared_ptr<QueryDelegate>(
                new RepositoryHasDelegate<RepositoryUninstallableInterface, &RepositoryCapabilities::uninstallable_interface>))
{
}

namespace
{
    struct InstalledAtRootDelegate :
        QueryDelegate
    {
        const FSEntry root;

        InstalledAtRootDelegate(const FSEntry & r) :
            root(r)
        {
        }

        std::tr1::shared_ptr<RepositoryNameCollection>
        repositories(const Environment & e) const
        {
            std::tr1::shared_ptr<RepositoryNameCollection> result(new RepositoryNameCollection::Concrete);

            for (PackageDatabase::RepositoryIterator i(e.package_database()->begin_repositories()),
                    i_end(e.package_database()->end_repositories()) ; i != i_end ; ++i)
                if ((*i)->installed_interface)
                    if (root == (*i)->installed_interface->root())
                        result->push_back((*i)->name());

            return result;
        }
    };
}

query::InstalledAtRoot::InstalledAtRoot(const FSEntry & r) :
    Query(std::tr1::shared_ptr<QueryDelegate>(
                new InstalledAtRootDelegate(r)))
{
}

namespace
{
    struct RepositoryNameComparator
    {
        bool operator() (const RepositoryName & l, const RepositoryName & r) const
        {
            return stringify(l) < stringify(r);
        }
    };

    struct AndQueryDelegate :
        QueryDelegate
    {
        std::tr1::shared_ptr<const QueryDelegate> q1, q2;

        AndQueryDelegate(std::tr1::shared_ptr<const QueryDelegate> qq1,
                std::tr1::shared_ptr<const QueryDelegate> qq2) :
            q1(qq1),
            q2(qq2)
        {
        }

        std::tr1::shared_ptr<RepositoryNameCollection>
        repositories(const Environment & e) const
        {
            std::tr1::shared_ptr<RepositoryNameCollection> r1(q1->repositories(e)), r2(q2->repositories(e));

            if (r1 && r2)
            {
                std::set<RepositoryName, RepositoryNameComparator> s1(r1->begin(), r1->end()), s2(r2->begin(), r2->end());
                std::tr1::shared_ptr<RepositoryNameCollection> result(new RepositoryNameCollection::Concrete);
                std::set_intersection(s1.begin(), s1.end(), s2.begin(), s2.end(), result->inserter(),
                        RepositoryNameComparator());
                return result;
            }
            else if (r1)
                return r1;
            else
                return r2;
        }

        std::tr1::shared_ptr<CategoryNamePartCollection>
        categories(const Environment & e, std::tr1::shared_ptr<const RepositoryNameCollection> r) const
        {
            std::tr1::shared_ptr<CategoryNamePartCollection> r1(q1->categories(e, r)), r2(q2->categories(e, r));

            if (r1 && r2)
            {
                std::tr1::shared_ptr<CategoryNamePartCollection> result(new CategoryNamePartCollection::Concrete);
                std::set_intersection(r1->begin(), r1->end(), r2->begin(), r2->end(), result->inserter());
                return result;
            }
            else if (r1)
                return r1;
            else
                return r2;
        }

        std::tr1::shared_ptr<QualifiedPackageNameCollection>
        packages(const Environment & e, std::tr1::shared_ptr<const RepositoryNameCollection> r,
                std::tr1::shared_ptr<const CategoryNamePartCollection> c) const
        {
            std::tr1::shared_ptr<QualifiedPackageNameCollection> r1(q1->packages(e, r, c)), r2(q2->packages(e, r, c));

            if (r1 && r2)
            {
                std::tr1::shared_ptr<QualifiedPackageNameCollection> result(new QualifiedPackageNameCollection::Concrete);
                std::set_intersection(r1->begin(), r1->end(), r2->begin(), r2->end(), result->inserter());
                return result;
            }
            else if (r1)
                return r1;
            else
                return r2;
        }

        std::tr1::shared_ptr<PackageDatabaseEntryCollection>
        versions(const Environment & e, std::tr1::shared_ptr<const RepositoryNameCollection> r,
                std::tr1::shared_ptr<const QualifiedPackageNameCollection> q) const
        {
            std::tr1::shared_ptr<PackageDatabaseEntryCollection> r1(q1->versions(e, r, q)), r2(q2->versions(e, r, q));

            if (r1 && r2)
            {
                std::set<PackageDatabaseEntry, ArbitrarilyOrderedPackageDatabaseEntryCollectionComparator>
                    s1(r1->begin(), r1->end()), s2(r2->begin(), r2->end());
                std::tr1::shared_ptr<PackageDatabaseEntryCollection> result(new PackageDatabaseEntryCollection::Concrete);
                std::set_intersection(s1.begin(), s1.end(), s2.begin(), s2.end(), result->inserter(),
                        ArbitrarilyOrderedPackageDatabaseEntryCollectionComparator());
                return result;
            }
            else if (r1)
                return r1;
            else
                return r2;
        }
    };
}

Query
paludis::operator& (const Query & q1, const Query & q2)
{
    return Query(std::tr1::shared_ptr<QueryDelegate>(new AndQueryDelegate(q1._d, q2._d)));
}

