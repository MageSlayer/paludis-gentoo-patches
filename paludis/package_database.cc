/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <paludis/dep_spec.hh>
#include <paludis/match_package.hh>
#include <paludis/package_database.hh>
#include <paludis/package_id.hh>
#include <paludis/environment.hh>
#include <paludis/util/log.hh>
#include <paludis/util/iterator.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/join.hh>
#include <paludis/util/iterator.hh>
#include <paludis/util/set.hh>
#include <paludis/util/map.hh>
#include <paludis/util/map-impl.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/tr1_functional.hh>
#include <paludis/query.hh>

#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <libwrapiter/libwrapiter_output_iterator.hh>

#include <list>
#include <map>
#include <set>

using namespace paludis;

#include "package_database-se.cc"

PackageDatabaseError::PackageDatabaseError(const std::string & our_message) throw () :
    Exception(our_message)
{
}

PackageDatabaseLookupError::PackageDatabaseLookupError(const std::string & our_message) throw () :
    PackageDatabaseError(our_message)
{
}

DuplicateRepositoryError::DuplicateRepositoryError(const std::string & name) throw () :
    PackageDatabaseError("A repository named '" + name + "' already exists")
{
}

NoSuchPackageError::NoSuchPackageError(const std::string & our_name) throw () :
    PackageDatabaseLookupError("Could not find '" + our_name + "'"),
    _name(our_name)
{
}

NonUniqueQueryResultError::NonUniqueQueryResultError(const Query & q,
        const tr1::shared_ptr<const PackageIDSequence> & r) throw () :
    PackageDatabaseLookupError("Query '" + stringify(q) + "' returned " +
            (r->empty() ? "empty result set" : "'" + join(indirect_iterator(r->begin()), indirect_iterator(r->end()), " ")
             + "'") + " but qo_require_exactly_one was specified")
{
}

NoSuchRepositoryError::NoSuchRepositoryError(const std::string & n) throw () :
    PackageDatabaseLookupError("Could not find repository '" + n + "'"),
    _name("UNKNOWN")
{
}

NoSuchRepositoryError::NoSuchRepositoryError(const RepositoryName & n) throw () :
    PackageDatabaseLookupError("Could not find repository '" + stringify(n) + "'"),
    _name(n)
{
}

NoSuchRepositoryError::~NoSuchRepositoryError() throw ()
{
}

RepositoryName
NoSuchRepositoryError::name() const
{
    return _name;
}

struct AmbiguousPackageNameError::NameData
{
    std::string name;
    std::list<std::string> names;
};

template <typename I_>
AmbiguousPackageNameError::AmbiguousPackageNameError(const std::string & our_name,
        I_ begin, const I_ end) throw () :
    PackageDatabaseLookupError("Ambiguous package name '" + our_name + "' (candidates are " +
            join(begin, end, ", ") + ")"),
    _name_data(new NameData)
{
    _name_data->name = our_name;
    std::transform(begin, end, std::back_inserter(_name_data->names),
            &stringify<typename std::iterator_traits<I_>::value_type>);
}

AmbiguousPackageNameError::AmbiguousPackageNameError(const AmbiguousPackageNameError & other) :
    PackageDatabaseLookupError(other),
    _name_data(new NameData)
{
    _name_data->name = other._name_data->name;
    _name_data->names = other._name_data->names;
}

AmbiguousPackageNameError::~AmbiguousPackageNameError() throw ()
{
    delete _name_data;
}

AmbiguousPackageNameError::OptionsConstIterator
AmbiguousPackageNameError::begin_options() const
{
    return OptionsConstIterator(_name_data->names.begin());
}

AmbiguousPackageNameError::OptionsConstIterator
AmbiguousPackageNameError::end_options() const
{
    return OptionsConstIterator(_name_data->names.end());
}

const std::string &
AmbiguousPackageNameError::name() const
{
    return _name_data->name;
}

namespace paludis
{
    template<>
    struct Implementation<PackageDatabase>
    {
        std::list<tr1::shared_ptr<Repository> > repositories;
        std::multimap<int, std::list<tr1::shared_ptr<Repository> >::iterator> repository_importances;
        const Environment * environment;
    };
}

PackageDatabase::PackageDatabase(const Environment * const e) :
    PrivateImplementationPattern<PackageDatabase>(new Implementation<PackageDatabase>)
{
    _imp->environment = e;
}

PackageDatabase::~PackageDatabase()
{
}

void
PackageDatabase::add_repository(int i, const tr1::shared_ptr<Repository> r)
{
    Context c("When adding a repository named '" + stringify(r->name()) + "':");

    for (IndirectIterator<RepositoryConstIterator> r_c(begin_repositories()), r_end(end_repositories()) ;
            r_c != r_end ; ++r_c)
        if (r_c->name() == r->name())
            throw DuplicateRepositoryError(stringify(r->name()));

    std::list<tr1::shared_ptr<Repository> >::iterator q(_imp->repositories.end());
    for (std::multimap<int, std::list<tr1::shared_ptr<Repository> >::iterator>::iterator
            p(_imp->repository_importances.begin()), p_end(_imp->repository_importances.end()) ;
            p != p_end ; ++p)
        if (p->first > i)
        {
            q = p->second;
            break;
        }

    _imp->repository_importances.insert(std::make_pair(i, _imp->repositories.insert(q, r)));
}

namespace
{
    struct IsInstalled
    {
        const FSEntry _root;
        const tr1::shared_ptr<const PackageDatabase> _db;

        IsInstalled(const Environment * e) :
            _root(e->root()),
            _db(e->package_database())
        {
        }

        typedef QualifiedPackageName argument_type;
        typedef bool result_type;

        bool operator() (const QualifiedPackageName & qpn) const
        {
            return (! _db->query(query::Package(qpn) & query::InstalledAtRoot(_root), qo_whatever)->empty());
        }
    };

    struct IsImportant
    {
        typedef QualifiedPackageName argument_type;
        typedef bool result_type;

        typedef Map<const QualifiedPackageName, const tr1::shared_ptr<const CategoryNamePartSet> > QPNCMap;
        const tr1::shared_ptr<QPNCMap> _map;

        IsImportant(const tr1::shared_ptr<QPNCMap> & m) :
            _map(m)
        {
        }

        bool operator() (const QualifiedPackageName & qpn) const
        {
            return (_map->find(qpn)->second->end() == _map->find(qpn)->second->find(qpn.category));
        }
    };
}

QualifiedPackageName
PackageDatabase::fetch_unique_qualified_package_name(
        const PackageNamePart & p) const
{
    Context context("When disambiguating package name '" + stringify(p) + "':");

    // Map matching QualifiedPackageNames with unimportant_category_names sets from their repository.
    typedef Map<const QualifiedPackageName, const tr1::shared_ptr<const CategoryNamePartSet> > QPNCMap;
    tr1::shared_ptr<QPNCMap> result(new QPNCMap);

    for (IndirectIterator<RepositoryConstIterator> r(begin_repositories()), r_end(end_repositories()) ;
            r != r_end ; ++r)
    {
        Context local_context("When looking in repository '" + stringify(r->name()) + "':");

        tr1::shared_ptr<const CategoryNamePartSet> cats(r->category_names_containing_package(p));
        tr1::shared_ptr<const CategoryNamePartSet> unimportant_cats(r->unimportant_category_names());

        for (CategoryNamePartSet::ConstIterator c(cats->begin()), c_end(cats->end()) ;
                c != c_end ; ++c)
            result->insert(*c + p, unimportant_cats);
    }

    if (result->empty())
        throw NoSuchPackageError(stringify(p));
    if (result->size() > 1)
    {
        using namespace tr1::placeholders;

        std::list<QualifiedPackageName> qpns;

        do
        {
            const IsImportant is_important(result);
            const IsInstalled is_installed(_imp->environment);

            std::remove_copy_if(first_iterator(result->begin()), first_iterator(result->end()),
                    std::front_inserter(qpns),
                    tr1::bind(std::logical_and<bool>(),
                        tr1::bind(std::not1(is_important), _1),
                        tr1::bind(std::not1(is_installed), _1)));

            if (! qpns.empty() && next(qpns.begin()) == qpns.end())
                break;

            qpns.remove_if(tr1::bind(std::logical_and<bool>(),
                        tr1::bind(is_important, _1),
                        tr1::bind(std::not1(is_installed), _1)));

            if (! qpns.empty() && next(qpns.begin()) == qpns.end())
                break;

            qpns.remove_if(tr1::bind(std::logical_and<bool>(),
                        tr1::bind(std::not1(is_important), _1),
                        tr1::bind(is_installed, _1)));

            if (! qpns.empty() && next(qpns.begin()) == qpns.end())
                break;

            throw AmbiguousPackageNameError(stringify(p), first_iterator(result->begin()),
                    first_iterator(result->end()));
        } while (false);

        Log::get_instance()->message(ll_warning, lc_context)
            << "Package name '" << p << "' is ambiguous, assuming you meant '" << *qpns.begin()
            << "' (candidates were '"
            << join(first_iterator(result->begin()), first_iterator(result->end()), "', '") << "')";

        return *qpns.begin();
    }
    else
        return result->begin()->first;
}

const tr1::shared_ptr<const PackageIDSequence>
PackageDatabase::query(const Query & q, const QueryOrder query_order) const
{
    using namespace tr1::placeholders;

    tr1::shared_ptr<PackageIDSequence> result(new PackageIDSequence);

    tr1::shared_ptr<RepositoryNameSequence> repos(q.repositories(*_imp->environment));
    if (! repos)
    {
        repos.reset(new RepositoryNameSequence);
        for (RepositoryConstIterator r(begin_repositories()), r_end(end_repositories()) ;
                r != r_end ; ++r)
            repos->push_back((*r)->name());
    }
    if (repos->empty())
        if (qo_require_exactly_one == query_order)
            throw NonUniqueQueryResultError(q, result);
        else
            return result;

    tr1::shared_ptr<CategoryNamePartSet> cats(q.categories(*_imp->environment, repos));
    if (! cats)
    {
        cats.reset(new CategoryNamePartSet);
        for (RepositoryNameSequence::ConstIterator r(repos->begin()), r_end(repos->end()) ;
                r != r_end ; ++r)
        {
            tr1::shared_ptr<const CategoryNamePartSet> local_cats(fetch_repository(*r)->category_names());
            std::copy(local_cats->begin(), local_cats->end(), cats->inserter());
        }
    }
    if (cats->empty())
        if (qo_require_exactly_one == query_order)
            throw NonUniqueQueryResultError(q, result);
        else
            return result;

    tr1::shared_ptr<QualifiedPackageNameSet> pkgs(q.packages(*_imp->environment, repos, cats));
    if (! pkgs)
    {
        pkgs.reset(new QualifiedPackageNameSet);
        for (RepositoryNameSequence::ConstIterator r(repos->begin()), r_end(repos->end()) ;
                r != r_end ; ++r)
            for (CategoryNamePartSet::ConstIterator c(cats->begin()), c_end(cats->end()) ;
                    c != c_end ; ++c)
            {
                tr1::shared_ptr<const QualifiedPackageNameSet> local_pkgs(fetch_repository(*r)->package_names(*c));
                std::copy(local_pkgs->begin(), local_pkgs->end(), pkgs->inserter());
            }
    }
    if (pkgs->empty())
        if (qo_require_exactly_one == query_order)
            throw NonUniqueQueryResultError(q, result);
        else
            return result;

    tr1::shared_ptr<PackageIDSequence> ids(q.ids(*_imp->environment, repos, pkgs));
    if (! ids)
    {
        for (RepositoryNameSequence::ConstIterator r(repos->begin()), r_end(repos->end()) ;
                r != r_end ; ++r)
            for (QualifiedPackageNameSet::ConstIterator p(pkgs->begin()), p_end(pkgs->end()) ;
                    p != p_end ; ++p)
            {
                tr1::shared_ptr<const PackageIDSequence> local_ids(fetch_repository(*r)->package_ids(*p));
                std::copy(local_ids->begin(), local_ids->end(), result->back_inserter());
            }
    }
    else
    {
        if (ids->empty())
            if (qo_require_exactly_one == query_order)
                throw NonUniqueQueryResultError(q, result);
            else
                return result;

        std::copy(ids->begin(), ids->end(), result->back_inserter());
    }

    do
    {
        switch (query_order)
        {
            case qo_order_by_version:
                {
                    PackageIDComparator c(this);
                    result->sort(tr1::cref(c));
                }
                continue;

            case qo_group_by_slot:
                {
                    /* if someone's bored, they can rewrite this to be a lot faster */
                    PackageIDComparator c(this);
                    std::set<tr1::shared_ptr<const PackageID>, tr1::function<bool (tr1::shared_ptr<const PackageID>,
                            tr1::shared_ptr<const PackageID>)> > s(
                                result->begin(), result->end(), tr1::bind(&PackageIDComparator::operator(), tr1::cref(c), _2, _1));
                    result.reset(new PackageIDSequence);

                    while (! s.empty())
                    {
                        result->push_front(*s.begin());
                        s.erase(s.begin());

                        for (std::set<tr1::shared_ptr<const PackageID>, tr1::function<bool (tr1::shared_ptr<const PackageID>,
                                    tr1::shared_ptr<const PackageID>)> >::iterator
                                i(s.begin()) ; i != s.end() ; )
                        {
                            if ((*i)->name() == (*result->begin())->name() && (*i)->slot() == (*result->begin())->slot())
                            {
                                result->push_front(*i);
                                s.erase(i++);
                            }
                            else
                                ++i;
                        }
                    }
                }
                continue;

            case qo_best_version_only:
                {
                    std::map<QualifiedPackageName, tr1::shared_ptr<const PackageID> > best;
                    PackageIDComparator c(this);
                    for (PackageIDSequence::ConstIterator r(result->begin()), r_end(result->end()) ;
                            r != r_end ; ++r)
                    {
                        std::pair<std::map<QualifiedPackageName, tr1::shared_ptr<const PackageID> >::iterator, bool> p(
                                best.insert(std::make_pair((*r)->name(), *r)));
                        if ((! p.second) && c(p.first->second, *r))
                            p.first->second = *r;
                    }

                    result.reset(new PackageIDSequence);
                    std::copy(best.begin(), best.end(), transform_inserter(result->back_inserter(),
                                tr1::mem_fn(&std::pair<const QualifiedPackageName, tr1::shared_ptr<const PackageID> >::second)));
                }
                continue;

            case qo_best_version_in_slot_only:
                {
                    std::map<std::pair<QualifiedPackageName, SlotName>, tr1::shared_ptr<const PackageID> > best;
                    PackageIDComparator c(this);
                    for (PackageIDSequence::ConstIterator r(result->begin()), r_end(result->end()) ;
                            r != r_end ; ++r)
                    {
                        std::pair<std::map<std::pair<QualifiedPackageName, SlotName>, tr1::shared_ptr<const PackageID> >::iterator, bool> p(
                                best.insert(std::make_pair(std::make_pair((*r)->name(), (*r)->slot()), *r)));
                        if ((! p.second) && c(p.first->second, *r))
                            p.first->second = *r;
                    }

                    result.reset(new PackageIDSequence);
                    std::copy(best.begin(), best.end(), transform_inserter(result->back_inserter(),
                                tr1::mem_fn(&std::pair<const std::pair<QualifiedPackageName, SlotName>,
                                    tr1::shared_ptr<const PackageID> >::second)));
                    result->sort(tr1::cref(c));
                }
                continue;

            case qo_require_exactly_one:
                if (result->empty() || (next(result->begin()) != result->end()))
                    throw NonUniqueQueryResultError(q, result);
                continue;

            case qo_whatever:
                continue;

            case last_qo:
                break;
        };

        throw InternalError(PALUDIS_HERE, "Bad query_order");
    }
    while (false);

    return result;

}


tr1::shared_ptr<const Repository>
PackageDatabase::fetch_repository(const RepositoryName & n) const
{
    for (RepositoryConstIterator r(begin_repositories()), r_end(end_repositories()) ;
            r != r_end ; ++r)
        if ((*r)->name() == n)
            return *r;

    throw NoSuchRepositoryError(n);
}

tr1::shared_ptr<Repository>
PackageDatabase::fetch_repository(const RepositoryName & n)
{
    for (RepositoryConstIterator r(begin_repositories()), r_end(end_repositories()) ;
            r != r_end ; ++r)
        if ((*r)->name() == n)
            return *r;

    throw NoSuchRepositoryError(n);
}

bool
PackageDatabase::has_repository_named(const RepositoryName & n) const
{
    for (RepositoryConstIterator r(begin_repositories()), r_end(end_repositories()) ;
            r != r_end ; ++r)
        if ((*r)->name() == n)
            return true;

    return false;
}

RepositoryName
PackageDatabase::favourite_repository() const
{
    for (RepositoryConstIterator r(begin_repositories()), r_end(end_repositories()) ;
            r != r_end ; ++r)
        if ((*r)->can_be_favourite_repository())
            return (*r)->name();

    return RepositoryName("unnamed");
}

bool
PackageDatabase::more_important_than(const RepositoryName & lhs,
        const RepositoryName & rhs) const
{
    std::map<std::string, int> rank;
    int x(0);
    for (PackageDatabase::RepositoryConstIterator r(begin_repositories()), r_end(end_repositories()) ;
            r != r_end ; ++r)
        rank.insert(std::make_pair(stringify((*r)->name()), ++x));

    std::map<std::string, int>::const_iterator l(rank.find(stringify(lhs)));
    if (l == rank.end())
        throw InternalError(PALUDIS_HERE, "lhs.repository '" + stringify(lhs) + "' not in rank");

    std::map<std::string, int>::const_iterator r(rank.find(stringify(rhs)));
    if (r == rank.end())
        throw InternalError(PALUDIS_HERE, "rhs.repository '" + stringify(rhs) + "' not in rank");

    return l->second > r->second;
}

PackageDatabase::RepositoryConstIterator
PackageDatabase::begin_repositories() const
{
    return RepositoryConstIterator(_imp->repositories.begin());
}

PackageDatabase::RepositoryConstIterator
PackageDatabase::end_repositories() const
{
    return RepositoryConstIterator(_imp->repositories.end());
}

