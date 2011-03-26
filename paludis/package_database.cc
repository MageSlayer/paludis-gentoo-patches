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

#include <paludis/dep_spec.hh>
#include <paludis/match_package.hh>
#include <paludis/package_database.hh>
#include <paludis/package_id.hh>
#include <paludis/environment.hh>
#include <paludis/selection.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/partially_made_package_dep_spec.hh>
#include <paludis/metadata_key.hh>
#include <paludis/util/log.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/join.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/member_iterator-impl.hh>
#include <paludis/util/set-impl.hh>
#include <paludis/util/map.hh>
#include <paludis/util/map-impl.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/member_iterator.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <functional>
#include <algorithm>
#include <list>
#include <map>
#include <set>

using namespace paludis;

namespace paludis
{
    template <>
    struct WrappedForwardIteratorTraits<PackageDatabase::RepositoryConstIteratorTag>
    {
        typedef std::list<std::shared_ptr<Repository> >::const_iterator UnderlyingIterator;
    };
}

namespace paludis
{
    template<>
    struct Imp<PackageDatabase>
    {
        std::list<std::shared_ptr<Repository> > repositories;
        std::multimap<int, std::list<std::shared_ptr<Repository> >::iterator> repository_importances;
        const Environment * environment;
    };
}

PackageDatabase::PackageDatabase(const Environment * const e) :
    _imp()
{
    _imp->environment = e;
}

PackageDatabase::~PackageDatabase()
{
}

void
PackageDatabase::add_repository(int i, const std::shared_ptr<Repository> r)
{
    Context c("When adding a repository named '" + stringify(r->name()) + "':");

    for (IndirectIterator<RepositoryConstIterator> r_c(begin_repositories()), r_end(end_repositories()) ;
            r_c != r_end ; ++r_c)
        if (r_c->name() == r->name())
            throw DuplicateRepositoryError(stringify(r->name()));

    std::list<std::shared_ptr<Repository> >::iterator q(_imp->repositories.end());
    for (std::multimap<int, std::list<std::shared_ptr<Repository> >::iterator>::iterator
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
    typedef std::map<const QualifiedPackageName, std::pair<bool, bool> > QPNIMap;

    struct CategoryRepositoryNamePairComparator
    {
        bool operator() (const std::pair<CategoryNamePart, RepositoryName> & a,
                         const std::pair<CategoryNamePart, RepositoryName> & b) const
        {
            if (a.first != b.first)
                return a.first < b.first;
            return a.second < b.second;
        }
    };

    struct IsInstalled
    {
        const Environment * const _env;

        IsInstalled(const Environment * e) :
            _env(e)
        {
        }

        typedef QualifiedPackageName argument_type;
        typedef bool result_type;

        bool operator() (const QualifiedPackageName & qpn) const
        {
            return ! (*_env)[selection::SomeArbitraryVersion(generator::Package(qpn) |
                    filter::InstalledAtRoot(_env->preferred_root_key()->value()))]->empty();
        }
    };

    struct IsImportant
    {
        typedef QualifiedPackageName argument_type;
        typedef bool result_type;

        const std::shared_ptr<QPNIMap> _map;

        IsImportant(const std::shared_ptr<QPNIMap> & m) :
            _map(m)
        {
        }

        bool operator() (const QualifiedPackageName & qpn) const
        {
            return _map->find(qpn)->second.first;
        }
    };

    struct IsInUnimportantRepo
    {
        typedef QualifiedPackageName argument_type;
        typedef bool result_type;

        const std::shared_ptr<QPNIMap> _map;

        IsInUnimportantRepo(const std::shared_ptr<QPNIMap> & m) :
            _map(m)
        {
        }

        bool operator() (const QualifiedPackageName & qpn) const
        {
            return ! _map->find(qpn)->second.second;
        }
    };
}

QualifiedPackageName
PackageDatabase::fetch_unique_qualified_package_name(const PackageNamePart & p, const Filter & f, const bool disambiguate) const
{
    Context context("When disambiguating package name '" + stringify(p) + "':");

    // Map matching QualifiedPackageNames with a pair of flags specifying
    // respectively that at least one repository containing the package thinks
    // the category is important and that the package is in a repository
    // that reports it is important itself
    std::shared_ptr<QPNIMap> result(new QPNIMap);
    std::set<std::pair<CategoryNamePart, RepositoryName>, CategoryRepositoryNamePairComparator> checked;

    std::shared_ptr<const PackageIDSequence> pkgs((*_imp->environment)[selection::AllVersionsUnsorted(
                generator::Matches(make_package_dep_spec({ }).package_name_part(p), make_null_shared_ptr(), { }) | f)]);

    for (IndirectIterator<PackageIDSequence::ConstIterator> it(pkgs->begin()),
             it_end(pkgs->end()); it_end != it; ++it)
    {
        Context local_context("When checking category '" + stringify(it->name().category()) + "' in repository '" +
                stringify(it->repository_name()) + "':");

        if (! checked.insert(std::make_pair(it->name().category(), it->repository_name())).second)
            continue;

        auto repo(fetch_repository(it->repository_name()));
        std::shared_ptr<const CategoryNamePartSet> unimportant_cats(repo->unimportant_category_names({ }));
        bool is_important(unimportant_cats->end() == unimportant_cats->find(it->name().category()));
        bool is_in_important_repo(! repo->is_unimportant());
        QPNIMap::iterator i(result->insert(std::make_pair(it->name(), std::make_pair(is_important, is_in_important_repo))).first);
        i->second.first = i->second.first || is_important;
        i->second.second = i->second.second || is_in_important_repo;
    }

    if (result->empty())
        throw NoSuchPackageError(stringify(p));
    if (result->size() > 1)
    {
        using namespace std::placeholders;

        std::list<QualifiedPackageName> qpns;

        do
        {
            const IsImportant is_important(result);
            const IsInstalled is_installed(_imp->environment);
            const IsInUnimportantRepo is_in_unimportant_repo(result);

            std::remove_copy_if(first_iterator(result->begin()), first_iterator(result->end()),
                    std::front_inserter(qpns),
                    std::bind(std::logical_and<bool>(),
                        std::bind(std::not1(is_important), _1),
                        std::bind(std::not1(is_installed), _1)));

            if (! qpns.empty() && next(qpns.begin()) == qpns.end())
                break;

            qpns.remove_if(std::bind(is_in_unimportant_repo, _1));

            if (! qpns.empty() && next(qpns.begin()) == qpns.end())
                break;

            qpns.remove_if(std::bind(std::logical_and<bool>(),
                        std::bind(is_important, _1),
                        std::bind(std::not1(is_installed), _1)));

            if (! qpns.empty() && next(qpns.begin()) == qpns.end())
                break;

            qpns.remove_if(std::bind(std::logical_and<bool>(),
                        std::bind(std::not1(is_important), _1),
                        std::bind(is_installed, _1)));

            if (! qpns.empty() && next(qpns.begin()) == qpns.end())
                break;

            auto candidates(std::make_shared<Sequence<std::string> >());
            std::transform(first_iterator(result->begin()), first_iterator(result->end()), candidates->back_inserter(),
                    &stringify<QualifiedPackageName>);
            throw AmbiguousPackageNameError(stringify(p), candidates);
        } while (false);

        if (! disambiguate)
        {
            auto candidates(std::make_shared<Sequence<std::string> >());
            std::transform(first_iterator(result->begin()), first_iterator(result->end()), candidates->back_inserter(),
                    &stringify<QualifiedPackageName>);
            throw AmbiguousPackageNameError(stringify(p), candidates);
        }

        Log::get_instance()->message("package_database.ambiguous_name", ll_warning, lc_context)
            << "Package name '" << p << "' is ambiguous, assuming you meant '" << *qpns.begin()
            << "' (candidates were '"
            << join(first_iterator(result->begin()), first_iterator(result->end()), "', '") << "')";

        return *qpns.begin();
    }
    else
        return result->begin()->first;
}

std::shared_ptr<const Repository>
PackageDatabase::fetch_repository(const RepositoryName & n) const
{
    for (RepositoryConstIterator r(begin_repositories()), r_end(end_repositories()) ;
            r != r_end ; ++r)
        if ((*r)->name() == n)
            return *r;

    throw NoSuchRepositoryError(n);
}

std::shared_ptr<Repository>
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

const Filter &
PackageDatabase::all_filter()
{
    static const Filter result((filter::All()));
    return result;
}

template class WrappedForwardIterator<PackageDatabase::RepositoryConstIteratorTag, const std::shared_ptr<Repository> >;

