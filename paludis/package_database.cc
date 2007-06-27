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
#include <paludis/util/iterator.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/join.hh>
#include <paludis/util/collection_concrete.hh>
#include <paludis/query.hh>

#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <libwrapiter/libwrapiter_output_iterator.hh>

#include <list>
#include <map>
#include <set>

/** \file
 * Implementation of PackageDatabase.
 */

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

/**
 * Name data for an AmbiguousPackageNameError.
 *
 * \ingroup grpexceptions
 */
struct AmbiguousPackageNameError::NameData
{
    /// Our query name
    std::string name;

    /// Our match names
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

AmbiguousPackageNameError::OptionsIterator
AmbiguousPackageNameError::begin_options() const
{
    return OptionsIterator(_name_data->names.begin());
}

AmbiguousPackageNameError::OptionsIterator
AmbiguousPackageNameError::end_options() const
{
    return OptionsIterator(_name_data->names.end());
}

const std::string &
AmbiguousPackageNameError::name() const
{
    return _name_data->name;
}

namespace paludis
{
    /**
     * Implementation data for a PackageDatabase.
     *
     * \ingroup grppackagedatabase
     */
    template<>
    struct Implementation<PackageDatabase>
    {
        /**
         * Our Repository instances.
         */
        std::list<tr1::shared_ptr<Repository> > repositories;

        /**
         * Repository importances.
         */
        std::multimap<int, std::list<tr1::shared_ptr<Repository> >::iterator> repository_importances;

        /// Our environment.
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

    for (IndirectIterator<RepositoryIterator> r_c(begin_repositories()), r_end(end_repositories()) ;
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

QualifiedPackageName
PackageDatabase::fetch_unique_qualified_package_name(
        const PackageNamePart & p) const
{
    Context context("When disambiguating package name '" + stringify(p) + "':");

    tr1::shared_ptr<QualifiedPackageNameCollection> result(new QualifiedPackageNameCollection::Concrete);

    for (IndirectIterator<RepositoryIterator> r(begin_repositories()), r_end(end_repositories()) ;
            r != r_end ; ++r)
    {
        Context local_context("When looking in repository '" + stringify(r->name()) + "':");

        tr1::shared_ptr<const CategoryNamePartCollection> cats(r->category_names_containing_package(p));
        for (CategoryNamePartCollection::Iterator c(cats->begin()), c_end(cats->end()) ;
                c != c_end ; ++c)
            result->insert(*c + p);
    }

    if (result->empty())
        throw NoSuchPackageError(stringify(p));
    if (result->size() > 1)
        throw AmbiguousPackageNameError(stringify(p), result->begin(), result->end());

    return *(result->begin());
}

namespace
{
    bool compare_name(const tr1::shared_ptr<const PackageID> & a,
            const tr1::shared_ptr<const PackageID> & b)
    {
        return a->name() == b->name();
    }

    bool compare_name_slot(const tr1::shared_ptr<const PackageID> & a,
            const tr1::shared_ptr<const PackageID> & b)
    {
        if (a->name() != b->name())
            return false;

        return a->slot() == b->slot();
    }

    struct PDEComparator
    {
        const PackageDatabase * const pde;
        std::map<std::string, int> rank;

        PDEComparator(const PackageDatabase * const p) :
            pde(p)
        {
            int x(0);
            for (PackageDatabase::RepositoryIterator r(pde->begin_repositories()), r_end(pde->end_repositories()) ;
                    r != r_end ; ++r)
                rank.insert(std::make_pair(stringify((*r)->name()), ++x));
        }

        bool operator() (const tr1::shared_ptr<const PackageID> & lhs,
                const tr1::shared_ptr<const PackageID> & rhs) const
        {
            if (lhs->name() < rhs->name())
                return true;
            if (lhs->name() > rhs->name())
                return false;

            if (lhs->version() < rhs->version())
                return true;
            if (lhs->version() > rhs->version())
                return false;

            std::map<std::string, int>::const_iterator l(rank.find(stringify(lhs->repository()->name())));
            if (l == rank.end())
                throw InternalError(PALUDIS_HERE, "lhs.repository '" + stringify(lhs->repository()->name()) + "' not in rank");

            std::map<std::string, int>::const_iterator r(rank.find(stringify(rhs->repository()->name())));
            if (r == rank.end())
                throw InternalError(PALUDIS_HERE, "rhs.repository '" + stringify(rhs->repository()->name()) + "' not in rank");

            if (l->second < r->second)
                return true;

            return false;
        }
    };

    void sort_package_database_entry_collection(const PackageDatabase * const t,
            PackageIDSequence::Concrete & p)
    {
        if (! p.empty())
            p.sort(PDEComparator(t));
    }

    void
    group_package_database_entry_collection(PackageIDSequence::Concrete & p)
    {
        if (p.empty())
            return;

        for (std::list<tr1::shared_ptr<const PackageID> >::reverse_iterator r(p.list.rbegin()) ;
                r != p.list.rend() ; ++r)
        {
            for (std::list<tr1::shared_ptr<const PackageID> >::reverse_iterator rr(next(r)) ;
                    rr != p.list.rend() ; ++rr)
            {
                if ((*r)->name() != (*rr)->name())
                    break;

                if ((*r)->slot() != (*rr)->slot())
                    continue;

                p.list.splice(previous(r.base()), p.list, previous(rr.base()));
                if (p.list.rend() == ((rr = ++r)))
                    return;
            }
        }
    }
}

const tr1::shared_ptr<const PackageIDSequence>
PackageDatabase::query(const Query & q, const QueryOrder query_order) const
{
    tr1::shared_ptr<PackageIDSequence::Concrete> result(new PackageIDSequence::Concrete);

    tr1::shared_ptr<RepositoryNameCollection> repos(q.repositories(*_imp->environment));
    if (! repos)
    {
        repos.reset(new RepositoryNameCollection::Concrete);
        for (RepositoryIterator r(begin_repositories()), r_end(end_repositories()) ;
                r != r_end ; ++r)
            repos->push_back((*r)->name());
    }
    if (repos->empty())
        if (qo_require_exactly_one == query_order)
            throw NonUniqueQueryResultError(q, result);
        else
            return result;

    tr1::shared_ptr<CategoryNamePartCollection> cats(q.categories(*_imp->environment, repos));
    if (! cats)
    {
        cats.reset(new CategoryNamePartCollection::Concrete);
        for (RepositoryNameCollection::Iterator r(repos->begin()), r_end(repos->end()) ;
                r != r_end ; ++r)
        {
            tr1::shared_ptr<const CategoryNamePartCollection> local_cats(fetch_repository(*r)->category_names());
            std::copy(local_cats->begin(), local_cats->end(), cats->inserter());
        }
    }
    if (cats->empty())
        if (qo_require_exactly_one == query_order)
            throw NonUniqueQueryResultError(q, result);
        else
            return result;

    tr1::shared_ptr<QualifiedPackageNameCollection> pkgs(q.packages(*_imp->environment, repos, cats));
    if (! pkgs)
    {
        pkgs.reset(new QualifiedPackageNameCollection::Concrete);
        for (RepositoryNameCollection::Iterator r(repos->begin()), r_end(repos->end()) ;
                r != r_end ; ++r)
            for (CategoryNamePartCollection::Iterator c(cats->begin()), c_end(cats->end()) ;
                    c != c_end ; ++c)
            {
                tr1::shared_ptr<const QualifiedPackageNameCollection> local_pkgs(fetch_repository(*r)->package_names(*c));
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
        for (RepositoryNameCollection::Iterator r(repos->begin()), r_end(repos->end()) ;
                r != r_end ; ++r)
            for (QualifiedPackageNameCollection::Iterator p(pkgs->begin()), p_end(pkgs->end()) ;
                    p != p_end ; ++p)
            {
                tr1::shared_ptr<const PackageIDSequence> local_ids(fetch_repository(*r)->package_ids(*p));
                std::copy(local_ids->begin(), local_ids->end(), result->inserter());
            }
    }
    else
    {
        if (ids->empty())
            if (qo_require_exactly_one == query_order)
                throw NonUniqueQueryResultError(q, result);
            else
                return result;

        std::copy(ids->begin(), ids->end(), result->inserter());
    }

    do
    {
        switch (query_order)
        {
            case qo_order_by_version:
                sort_package_database_entry_collection(this, *result);
                continue;

            case qo_group_by_slot:
                sort_package_database_entry_collection(this, *result);
                group_package_database_entry_collection(*result);
                continue;

            case qo_best_version_only:
                {
                    sort_package_database_entry_collection(this, *result);
                    std::list<tr1::shared_ptr<const PackageID> > l;
                    std::unique_copy(result->list.rbegin(), result->list.rend(),
                            std::front_inserter(l), &compare_name);
                    result->list.swap(l);
                }
                continue;

            case qo_best_version_in_slot_only:
                {
                    sort_package_database_entry_collection(this, *result);
                    group_package_database_entry_collection(*result);
                    std::list<tr1::shared_ptr<const PackageID> > l;
                    std::unique_copy(result->list.rbegin(), result->list.rend(),
                            std::front_inserter(l), &compare_name_slot);
                    result->list.swap(l);
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
    for (RepositoryIterator r(begin_repositories()), r_end(end_repositories()) ;
            r != r_end ; ++r)
        if ((*r)->name() == n)
            return *r;

    throw NoSuchRepositoryError(n);
}

tr1::shared_ptr<Repository>
PackageDatabase::fetch_repository(const RepositoryName & n)
{
    for (RepositoryIterator r(begin_repositories()), r_end(end_repositories()) ;
            r != r_end ; ++r)
        if ((*r)->name() == n)
            return *r;

    throw NoSuchRepositoryError(n);
}

bool
PackageDatabase::has_repository_named(const RepositoryName & n) const
{
    for (RepositoryIterator r(begin_repositories()), r_end(end_repositories()) ;
            r != r_end ; ++r)
        if ((*r)->name() == n)
            return true;

    return false;
}

RepositoryName
PackageDatabase::favourite_repository() const
{
    for (RepositoryIterator r(begin_repositories()), r_end(end_repositories()) ;
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
    for (PackageDatabase::RepositoryIterator r(begin_repositories()), r_end(end_repositories()) ;
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

PackageDatabase::RepositoryIterator
PackageDatabase::begin_repositories() const
{
    return RepositoryIterator(_imp->repositories.begin());
}

PackageDatabase::RepositoryIterator
PackageDatabase::end_repositories() const
{
    return RepositoryIterator(_imp->repositories.end());
}

std::ostream &
paludis::operator<< (std::ostream & o, const InstallState & s)
{
    do
    {
        switch (s)
        {
            case is_installed_only:
                o << "installed_only";
                continue;

            case is_installable_only:
                o << "installable_only";
                continue;

            case is_any:
                o << "any";
                continue;

            case last_is:
                ;
        }

        throw InternalError(PALUDIS_HERE, "Bad InstallState");
    } while (false);

    return o;
}

