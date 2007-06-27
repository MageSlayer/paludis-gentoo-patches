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
#include <paludis/util/tr1_functional.hh>
#include <paludis/package_database.hh>
#include <paludis/environment.hh>
#include <paludis/match_package.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <libwrapiter/libwrapiter_output_iterator.hh>
#include <algorithm>
#include <set>

using namespace paludis;

QueryDelegate::QueryDelegate()
{
}

QueryDelegate::~QueryDelegate()
{
}

tr1::shared_ptr<RepositoryNameCollection>
QueryDelegate::repositories(const Environment &) const
{
    return tr1::shared_ptr<RepositoryNameCollection>();
}

tr1::shared_ptr<CategoryNamePartCollection>
QueryDelegate::categories(const Environment &, tr1::shared_ptr<const RepositoryNameCollection>) const
{
    return tr1::shared_ptr<CategoryNamePartCollection>();
}

tr1::shared_ptr<QualifiedPackageNameCollection>
QueryDelegate::packages(const Environment &, tr1::shared_ptr<const RepositoryNameCollection>,
        tr1::shared_ptr<const CategoryNamePartCollection>) const
{
    return tr1::shared_ptr<QualifiedPackageNameCollection>();
}

tr1::shared_ptr<PackageIDSequence>
QueryDelegate::ids(const Environment &, tr1::shared_ptr<const RepositoryNameCollection>,
        tr1::shared_ptr<const QualifiedPackageNameCollection>) const
{
    return tr1::shared_ptr<PackageIDSequence>();
}

Query::Query(tr1::shared_ptr<const QueryDelegate> d) :
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

        std::string
        as_human_readable_string() const
        {
            return "matches '" + stringify(spec) + "'";
        }

        tr1::shared_ptr<RepositoryNameCollection>
        repositories(const Environment & e) const
        {
            if (spec.repository_ptr())
            {
                tr1::shared_ptr<RepositoryNameCollection> result(new RepositoryNameCollection::Concrete);

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

        tr1::shared_ptr<CategoryNamePartCollection>
        categories(const Environment & e,
                tr1::shared_ptr<const RepositoryNameCollection> r) const
        {
            if (spec.package_ptr())
            {
                tr1::shared_ptr<CategoryNamePartCollection> result(new CategoryNamePartCollection::Concrete);
                result->insert(spec.package_ptr()->category);
                return result;
            }
            else if (spec.category_name_part_ptr())
            {
                tr1::shared_ptr<CategoryNamePartCollection> result(new CategoryNamePartCollection::Concrete);
                result->insert(*spec.category_name_part_ptr());
                return result;
            }
            else
                return QueryDelegate::categories(e, r);
        }

        tr1::shared_ptr<QualifiedPackageNameCollection>
        packages(const Environment & e,
            tr1::shared_ptr<const RepositoryNameCollection> repos,
            tr1::shared_ptr<const CategoryNamePartCollection> cats) const
        {
            if (spec.package_ptr())
            {
                tr1::shared_ptr<QualifiedPackageNameCollection> result(
                        new QualifiedPackageNameCollection::Concrete);
                result->insert(*spec.package_ptr());
                return result;
            }
            else if (spec.package_name_part_ptr())
            {
                tr1::shared_ptr<QualifiedPackageNameCollection> result(
                        new QualifiedPackageNameCollection::Concrete);
                for (RepositoryNameCollection::Iterator r(repos->begin()), r_end(repos->end()) ;
                        r != r_end ; ++r)
                    for (CategoryNamePartCollection::Iterator c(cats->begin()), c_end(cats->end()) ;
                            c != c_end ; ++c)
                        if (e.package_database()->fetch_repository(*r)->has_package_named(*c +
                                    *spec.package_name_part_ptr()))
                            result->insert(*c + *spec.package_name_part_ptr());
                return result;
            }
            else
                return QueryDelegate::packages(e, repos, cats);
        }

        tr1::shared_ptr<PackageIDSequence>
        ids(const Environment & e,
                tr1::shared_ptr<const RepositoryNameCollection> repos,
                tr1::shared_ptr<const QualifiedPackageNameCollection> pkgs) const
        {
            tr1::shared_ptr<PackageIDSequence> result(new PackageIDSequence::Concrete);
            for (RepositoryNameCollection::Iterator r(repos->begin()), r_end(repos->end()) ;
                     r != r_end ; ++r)
            {
                tr1::shared_ptr<const Repository> repo(e.package_database()->fetch_repository(*r));

                for (QualifiedPackageNameCollection::Iterator p(pkgs->begin()), p_end(pkgs->end()) ;
                        p != p_end ; ++p)
                {
                    using namespace tr1::placeholders;
                    tr1::shared_ptr<const PackageIDSequence> i(repo->package_ids(*p));
                    for (PackageIDSequence::Iterator v(i->begin()), v_end(i->end()) ;
                            v != v_end ; ++v)
                        if (match_package(e, spec, **v))
                            result->push_back(*v);
                }
            }

            return result;
        }
    };
}

query::Matches::Matches(const PackageDepSpec & a) :
    Query(tr1::shared_ptr<QueryDelegate>(new MatchesDelegate(a)))
{
}

query::Package::Package(const QualifiedPackageName & a) :
    Query(tr1::shared_ptr<QueryDelegate>(new MatchesDelegate(PackageDepSpec(
                        tr1::shared_ptr<QualifiedPackageName>(new QualifiedPackageName(a))))))
{
}

query::Repository::Repository(const RepositoryName & a) :
    Query(tr1::shared_ptr<QueryDelegate>(new MatchesDelegate(PackageDepSpec(
                        tr1::shared_ptr<QualifiedPackageName>(),
                        tr1::shared_ptr<CategoryNamePart>(),
                        tr1::shared_ptr<PackageNamePart>(),
                        tr1::shared_ptr<VersionRequirements>(),
                        vr_and,
                        tr1::shared_ptr<SlotName>(),
                        tr1::shared_ptr<RepositoryName>(new RepositoryName(a))))))
{
}

query::Category::Category(const CategoryNamePart & a) :
    Query(tr1::shared_ptr<QueryDelegate>(new MatchesDelegate(PackageDepSpec(
                        tr1::shared_ptr<QualifiedPackageName>(),
                        tr1::shared_ptr<CategoryNamePart>(new CategoryNamePart(a))))))
{
}

namespace
{
    struct NotMaskedDelegate :
        QueryDelegate
    {
        std::string
        as_human_readable_string() const
        {
            return "not masked";
        }

        tr1::shared_ptr<PackageIDSequence>
        ids(const Environment & e,
                tr1::shared_ptr<const RepositoryNameCollection> repos,
                tr1::shared_ptr<const QualifiedPackageNameCollection> pkgs) const
        {
            tr1::shared_ptr<PackageIDSequence> result(new PackageIDSequence::Concrete);
            for (RepositoryNameCollection::Iterator r(repos->begin()), r_end(repos->end()) ;
                     r != r_end ; ++r)
            {
                tr1::shared_ptr<const Repository> repo(e.package_database()->fetch_repository(*r));

                for (QualifiedPackageNameCollection::Iterator p(pkgs->begin()), p_end(pkgs->end()) ;
                        p != p_end ; ++p)
                {
                    tr1::shared_ptr<const PackageIDSequence> i(repo->package_ids(*p));
                    for (PackageIDSequence::Iterator v(i->begin()), v_end(i->end()) ;
                            v != v_end ; ++v)
                        if (! e.mask_reasons(**v).any())
                            result->push_back(*v);
                }
            }

            return result;
        }
    };
}

query::NotMasked::NotMasked() :
    Query(tr1::shared_ptr<QueryDelegate>(new NotMaskedDelegate))
{
}

namespace
{
    template <typename I_>
    struct DelegateName;

    template <>
    struct DelegateName<RepositoryInstalledInterface>
    {
        static const char * value;
    };

    const char * DelegateName<RepositoryInstalledInterface>::value = "RepositoryInstalledInterface";

    template <>
    struct DelegateName<RepositoryUninstallableInterface>
    {
        static const char * value;
    };

    const char * DelegateName<RepositoryUninstallableInterface>::value = "RepositoryUninstallableInterface";

    template <>
    struct DelegateName<RepositoryInstallableInterface>
    {
        static const char * value;
    };

    const char * DelegateName<RepositoryInstallableInterface>::value = "RepositoryInstallableInterface";

    template <typename I_, I_ * (RepositoryCapabilities::* i_)>
    struct RepositoryHasDelegate :
        QueryDelegate
    {
        tr1::shared_ptr<RepositoryNameCollection>
        repositories(const Environment & e) const
        {
            tr1::shared_ptr<RepositoryNameCollection> result(new RepositoryNameCollection::Concrete);

            for (PackageDatabase::RepositoryIterator i(e.package_database()->begin_repositories()),
                    i_end(e.package_database()->end_repositories()) ; i != i_end ; ++i)
                if ((**i).*i_)
                    result->push_back((*i)->name());

            return result;
        }

        std::string
        as_human_readable_string() const
        {
            return "repository has '" + std::string(DelegateName<I_>::value) + "'";
        }

    };
}

query::RepositoryHasInstalledInterface::RepositoryHasInstalledInterface() :
    Query(tr1::shared_ptr<QueryDelegate>(
                new RepositoryHasDelegate<RepositoryInstalledInterface, &RepositoryCapabilities::installed_interface>))
{
}

query::RepositoryHasInstallableInterface::RepositoryHasInstallableInterface() :
    Query(tr1::shared_ptr<QueryDelegate>(
                new RepositoryHasDelegate<RepositoryInstallableInterface, &RepositoryCapabilities::installable_interface>))
{
}

query::RepositoryHasUninstallableInterface::RepositoryHasUninstallableInterface() :
    Query(tr1::shared_ptr<QueryDelegate>(
                new RepositoryHasDelegate<RepositoryUninstallableInterface, &RepositoryCapabilities::uninstallable_interface>))
{
}

namespace
{
    struct InstalledAtRootDelegate :
        QueryDelegate
    {
        const FSEntry root;

        std::string
        as_human_readable_string() const
        {
            return "installed at root '" + stringify(root) + "'";
        }

        InstalledAtRootDelegate(const FSEntry & r) :
            root(r)
        {
        }

        tr1::shared_ptr<RepositoryNameCollection>
        repositories(const Environment & e) const
        {
            tr1::shared_ptr<RepositoryNameCollection> result(new RepositoryNameCollection::Concrete);

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
    Query(tr1::shared_ptr<QueryDelegate>(
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
        tr1::shared_ptr<const QueryDelegate> q1, q2;

        std::string
        as_human_readable_string() const
        {
            return q1->as_human_readable_string()+ " & " + q2->as_human_readable_string();
        }

        AndQueryDelegate(tr1::shared_ptr<const QueryDelegate> qq1,
                tr1::shared_ptr<const QueryDelegate> qq2) :
            q1(qq1),
            q2(qq2)
        {
        }

        tr1::shared_ptr<RepositoryNameCollection>
        repositories(const Environment & e) const
        {
            tr1::shared_ptr<RepositoryNameCollection> r1(q1->repositories(e)), r2(q2->repositories(e));

            if (r1 && r2)
            {
                std::set<RepositoryName, RepositoryNameComparator> s1(r1->begin(), r1->end()), s2(r2->begin(), r2->end());
                tr1::shared_ptr<RepositoryNameCollection> result(new RepositoryNameCollection::Concrete);
                std::set_intersection(s1.begin(), s1.end(), s2.begin(), s2.end(), result->inserter(),
                        RepositoryNameComparator());
                return result;
            }
            else if (r1)
                return r1;
            else
                return r2;
        }

        tr1::shared_ptr<CategoryNamePartCollection>
        categories(const Environment & e, tr1::shared_ptr<const RepositoryNameCollection> r) const
        {
            tr1::shared_ptr<CategoryNamePartCollection> r1(q1->categories(e, r)), r2(q2->categories(e, r));

            if (r1 && r2)
            {
                tr1::shared_ptr<CategoryNamePartCollection> result(new CategoryNamePartCollection::Concrete);
                std::set_intersection(r1->begin(), r1->end(), r2->begin(), r2->end(), result->inserter());
                return result;
            }
            else if (r1)
                return r1;
            else
                return r2;
        }

        tr1::shared_ptr<QualifiedPackageNameCollection>
        packages(const Environment & e, tr1::shared_ptr<const RepositoryNameCollection> r,
                tr1::shared_ptr<const CategoryNamePartCollection> c) const
        {
            tr1::shared_ptr<QualifiedPackageNameCollection> r1(q1->packages(e, r, c)), r2(q2->packages(e, r, c));

            if (r1 && r2)
            {
                tr1::shared_ptr<QualifiedPackageNameCollection> result(new QualifiedPackageNameCollection::Concrete);
                std::set_intersection(r1->begin(), r1->end(), r2->begin(), r2->end(), result->inserter());
                return result;
            }
            else if (r1)
                return r1;
            else
                return r2;
        }

        tr1::shared_ptr<PackageIDSequence>
        ids(const Environment & e, tr1::shared_ptr<const RepositoryNameCollection> r,
                tr1::shared_ptr<const QualifiedPackageNameCollection> q) const
        {
            tr1::shared_ptr<PackageIDSequence> r1(q1->ids(e, r, q)), r2(q2->ids(e, r, q));

            if (r1 && r2)
            {
                std::set<tr1::shared_ptr<const PackageID>, PackageIDSetComparator>
                    s1(r1->begin(), r1->end()), s2(r2->begin(), r2->end());
                tr1::shared_ptr<PackageIDSequence> result(new PackageIDSequence::Concrete);
                std::set_intersection(s1.begin(), s1.end(), s2.begin(), s2.end(), result->inserter(), PackageIDSetComparator());
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
    return Query(tr1::shared_ptr<QueryDelegate>(new AndQueryDelegate(q1._d, q2._d)));
}

std::ostream &
paludis::operator<< (std::ostream & s, const Query & q)
{
    s << q._d->as_human_readable_string();
    return s;
}

namespace
{
    struct AllDelegate :
        QueryDelegate
    {
        AllDelegate()
        {
        }

        std::string
        as_human_readable_string() const
        {
            return "all";
        }

    };
}

query::All::All() :
    Query(tr1::shared_ptr<QueryDelegate>(
                new AllDelegate))
{
}

