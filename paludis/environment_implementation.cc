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

#include <paludis/environment_implementation.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/hook.hh>
#include <paludis/distribution.hh>
#include <paludis/selection.hh>
#include <paludis/repository.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/partially_made_package_dep_spec.hh>

#include <paludis/util/log.hh>
#include <paludis/util/save.hh>
#include <paludis/util/set.hh>
#include <paludis/util/system.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/member_iterator-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/util/env_var_names.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/join.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/set-impl.hh>

#include <algorithm>
#include <map>
#include <list>
#include <set>

#include "config.h"

using namespace paludis;

namespace
{
    typedef std::function<void (const SetName &) > CombiningFunction;

    struct CombineSets
    {
        std::shared_ptr<AllDepSpec> top;
        std::shared_ptr<SetSpecTree> tree;

        void add(const SetName & s)
        {
            tree->top()->append(std::make_shared<NamedSetDepSpec>(s));
        }

        CombineSets() :
            top(std::make_shared<AllDepSpec>()),
            tree(std::make_shared<SetSpecTree>(top))
        {
        }

        CombineSets(const CombineSets & other) :
            top(other.top),
            tree(other.tree)
        {
        }

        std::shared_ptr<const SetSpecTree> result() const
        {
            return tree;
        }
    };

    typedef std::map<SetName, std::pair<std::function<std::shared_ptr<const SetSpecTree> ()>, CombiningFunction> > SetsStore;

    template <typename F_>
    struct Cache
    {
        F_ func;
        std::shared_ptr<typename std::remove_reference<typename F_::result_type>::type> result;

        Cache(const F_ & f) :
            func(f)
        {
        }

        typename F_::result_type operator() ()
        {
            if (! result)
                result = std::make_shared<typename std::remove_reference<typename F_::result_type>::type>(func());
            return *result;
        }
    };

    template <typename F_>
    F_ cache(const F_ & f)
    {
        return Cache<F_>(f);
    }
}

namespace paludis
{
    template <>
    struct WrappedForwardIteratorTraits<Environment::RepositoryConstIteratorTag>
    {
        typedef std::list<std::shared_ptr<Repository> >::const_iterator UnderlyingIterator;
    };

    template <>
    struct Imp<EnvironmentImplementation>
    {
        std::map<unsigned, NotifierCallbackFunction> notifier_callbacks;

        std::list<std::shared_ptr<Repository> > repositories;
        std::multimap<int, std::list<std::shared_ptr<Repository> >::iterator> repository_importances;

        mutable Mutex sets_mutex;
        mutable bool loaded_sets;
        mutable std::shared_ptr<SetNameSet> set_names;
        mutable SetsStore sets;

        Imp() :
            loaded_sets(false)
        {
        }
    };
}

EnvironmentImplementation::EnvironmentImplementation() :
    _imp()
{
}

EnvironmentImplementation::~EnvironmentImplementation()
{
}


std::shared_ptr<const FSPathSequence>
EnvironmentImplementation::bashrc_files() const
{
    return std::make_shared<FSPathSequence>();
}

std::shared_ptr<const FSPathSequence>
EnvironmentImplementation::syncers_dirs() const
{
    std::shared_ptr<FSPathSequence> result(std::make_shared<FSPathSequence>());
    result->push_back(FSPath(DATADIR "/paludis/syncers"));
    result->push_back(FSPath(LIBEXECDIR "/paludis/syncers"));
    return result;
}

std::shared_ptr<const FSPathSequence>
EnvironmentImplementation::fetchers_dirs() const
{
    std::shared_ptr<FSPathSequence> result(std::make_shared<FSPathSequence>());
    std::string fetchers_dir(getenv_with_default(env_vars::fetchers_dir, ""));
    if (fetchers_dir.empty())
    {
        result->push_back(FSPath(DATADIR "/paludis/fetchers"));
        result->push_back(FSPath(LIBEXECDIR "/paludis/fetchers"));
    }
    else
        result->push_back(FSPath(fetchers_dir));
    return result;
}

std::string
EnvironmentImplementation::distribution() const
{
    static const std::string result(getenv_with_default(env_vars::distribution, DEFAULT_DISTRIBUTION));
    return result;
}

bool
EnvironmentImplementation::is_paludis_package(const QualifiedPackageName & n) const
{
    return stringify(n) == (*DistributionData::get_instance()->distribution_from_string(distribution())).paludis_package();
}

std::shared_ptr<PackageIDSequence>
EnvironmentImplementation::operator[] (const Selection & selection) const
{
    return selection.perform_select(this);
}

NotifierCallbackID
EnvironmentImplementation::add_notifier_callback(const NotifierCallbackFunction & f)
{
    unsigned idx(0);
    if (! _imp->notifier_callbacks.empty())
        idx = _imp->notifier_callbacks.rbegin()->first + 1;

    _imp->notifier_callbacks.insert(std::make_pair(idx, f));
    return idx;
}

void
EnvironmentImplementation::remove_notifier_callback(const NotifierCallbackID i)
{
    _imp->notifier_callbacks.erase(i);
}

void
EnvironmentImplementation::trigger_notifier_callback(const NotifierCallbackEvent & e) const
{
    for (std::map<unsigned, NotifierCallbackFunction>::const_iterator i(_imp->notifier_callbacks.begin()),
            i_end(_imp->notifier_callbacks.end()) ;
            i != i_end ; ++i)
        (i->second)(e);
}

void
EnvironmentImplementation::add_set(
        const SetName & name,
        const SetName & combined_name,
        const std::function<std::shared_ptr<const SetSpecTree> ()> & func,
        const bool combine) const
{
    Lock lock(_imp->sets_mutex);
    Context context("When adding set named '" + stringify(name) + ":");

    if (combine)
    {
        if (! _imp->sets.insert(std::make_pair(combined_name, std::make_pair(cache(func), CombiningFunction()))).second)
            throw DuplicateSetError(combined_name);

        std::shared_ptr<CombineSets> c_s(std::make_shared<CombineSets>());
        CombiningFunction c_func(_imp->sets.insert(std::make_pair(name, std::make_pair(
                            std::bind(&CombineSets::result, c_s),
                            std::bind(&CombineSets::add, c_s, std::placeholders::_1)
                            ))).first->second.second);
        if (! c_func)
            throw DuplicateSetError(name);
        c_func(combined_name);
    }
    else
    {
        if (! _imp->sets.insert(std::make_pair(name, std::make_pair(cache(func), CombiningFunction()))).second)
            throw DuplicateSetError(name);
    }
}

std::shared_ptr<const SetNameSet>
EnvironmentImplementation::set_names() const
{
    Lock lock(_imp->sets_mutex);
    _need_sets();

    return _imp->set_names;
}

const std::shared_ptr<const SetSpecTree>
EnvironmentImplementation::set(const SetName & s) const
{
    Lock lock(_imp->sets_mutex);
    _need_sets();

    SetsStore::const_iterator i(_imp->sets.find(s));
    if (_imp->sets.end() != i)
        return i->second.first();
    else
        return make_null_shared_ptr();
}

void
EnvironmentImplementation::_need_sets() const
{
    if (_imp->loaded_sets)
        return;

    for (auto r(begin_repositories()), r_end(end_repositories()) ;
            r != r_end ; ++r)
        (*r)->populate_sets();

    populate_sets();
    populate_standard_sets();

    _imp->set_names = std::make_shared<SetNameSet>();
    std::copy(first_iterator(_imp->sets.begin()), first_iterator(_imp->sets.end()), _imp->set_names->inserter());

    _imp->loaded_sets = true;
}

namespace
{
    std::shared_ptr<const SetSpecTree> make_everything_set()
    {
        Log::get_instance()->message("environment_implementation.everything_deprecated", ll_warning, lc_context)
            << "The 'everything' set is deprecated. Use either 'installed-packages' or 'installed-slots' instead";

        std::shared_ptr<SetSpecTree> result(std::make_shared<SetSpecTree>(std::make_shared<AllDepSpec>()));
        result->top()->append(std::make_shared<NamedSetDepSpec>(SetName("installed-packages")));
        return result;
    }

    std::shared_ptr<const SetSpecTree> make_nothing_set()
    {
        std::shared_ptr<SetSpecTree> result(std::make_shared<SetSpecTree>(std::make_shared<AllDepSpec>()));
        return result;
    }
}

void
EnvironmentImplementation::populate_standard_sets() const
{
    set_always_exists(SetName("system"));
    set_always_exists(SetName("world"));

    set_always_exists(SetName("installed-packages"));
    set_always_exists(SetName("installed-slots"));

    SetsStore::iterator i(_imp->sets.find(SetName("world")));
    /* some test cases build world through evil haxx. don't inject system in
     * then. */
    if (i->second.second)
        i->second.second(SetName("system"));

    /* nothing should define 'everything' any more */
    if (_imp->sets.end() != _imp->sets.find(SetName("everything")))
        throw InternalError(PALUDIS_HERE, "something's still defining the 'everything' set");
    add_set(SetName("everything"), SetName("everything"), make_everything_set, false);

    add_set(SetName("nothing"), SetName("nothing"), make_nothing_set, false);
}

namespace
{
    std::shared_ptr<const SetSpecTree> make_empty_set()
    {
        return std::make_shared<SetSpecTree>(std::make_shared<AllDepSpec>());
    }
}

void
EnvironmentImplementation::set_always_exists(const SetName & s) const
{
    SetsStore::const_iterator i(_imp->sets.find(s));
    if (_imp->sets.end() == i)
        add_set(s, SetName(stringify(s) + "::default"), make_empty_set, true);
}

void
EnvironmentImplementation::add_repository(int importance, const std::shared_ptr<Repository> & repository)
{
    Context c("When adding a repository named '" + stringify(repository->name()) + "':");

    for (IndirectIterator<RepositoryConstIterator> r_c(begin_repositories()), r_end(end_repositories()) ;
            r_c != r_end ; ++r_c)
        if (r_c->name() == repository->name())
            throw DuplicateRepositoryError(stringify(repository->name()));

    std::list<std::shared_ptr<Repository> >::iterator q(_imp->repositories.end());
    for (std::multimap<int, std::list<std::shared_ptr<Repository> >::iterator>::iterator
            p(_imp->repository_importances.begin()), p_end(_imp->repository_importances.end()) ;
            p != p_end ; ++p)
        if (p->first > importance)
        {
            q = p->second;
            break;
        }

    _imp->repository_importances.insert(std::make_pair(importance, _imp->repositories.insert(q, repository)));
}

const std::shared_ptr<const Repository>
EnvironmentImplementation::fetch_repository(const RepositoryName & name) const
{
    for (RepositoryConstIterator r(begin_repositories()), r_end(end_repositories()) ;
            r != r_end ; ++r)
        if ((*r)->name() == name)
            return *r;

    throw NoSuchRepositoryError(name);
}

const std::shared_ptr<Repository>
EnvironmentImplementation::fetch_repository(const RepositoryName & name)
{
    for (RepositoryConstIterator r(begin_repositories()), r_end(end_repositories()) ;
            r != r_end ; ++r)
        if ((*r)->name() == name)
            return *r;

    throw NoSuchRepositoryError(name);
}

bool
EnvironmentImplementation::has_repository_named(const RepositoryName & name) const
{
    for (RepositoryConstIterator r(begin_repositories()), r_end(end_repositories()) ;
            r != r_end ; ++r)
        if ((*r)->name() == name)
            return true;

    return false;
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
                    filter::InstalledAtRoot(_env->preferred_root_key()->parse_value()))]->empty();
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
EnvironmentImplementation::fetch_unique_qualified_package_name(const PackageNamePart & p,
        const Filter & f, const bool disambiguate) const
{
    Context context("When disambiguating package name '" + stringify(p) + "':");

    // Map matching QualifiedPackageNames with a pair of flags specifying
    // respectively that at least one repository containing the package thinks
    // the category is important and that the package is in a repository
    // that reports it is important itself
    std::shared_ptr<QPNIMap> result(new QPNIMap);
    std::set<std::pair<CategoryNamePart, RepositoryName>, CategoryRepositoryNamePairComparator> checked;

    std::shared_ptr<const PackageIDSequence> pkgs((*this)[selection::AllVersionsUnsorted(
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
            const IsInstalled is_installed(this);
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

        Log::get_instance()->message("environment.ambiguous_name", ll_warning, lc_context)
            << "Package name '" << p << "' is ambiguous, assuming you meant '" << *qpns.begin()
            << "' (candidates were '"
            << join(first_iterator(result->begin()), first_iterator(result->end()), "', '") << "')";

        return *qpns.begin();
    }
    else
        return result->begin()->first;
}

bool
EnvironmentImplementation::more_important_than(const RepositoryName & lhs, const RepositoryName & rhs) const
{
    std::map<std::string, int> rank;
    int x(0);
    for (auto r(begin_repositories()), r_end(end_repositories()) ;
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

EnvironmentImplementation::RepositoryConstIterator
EnvironmentImplementation::begin_repositories() const
{
    return RepositoryConstIterator(_imp->repositories.begin());
}

EnvironmentImplementation::RepositoryConstIterator
EnvironmentImplementation::end_repositories() const
{
    return RepositoryConstIterator(_imp->repositories.end());
}

const std::shared_ptr<const Set<std::string> >
EnvironmentImplementation::expand_licence(
        const std::string & s) const
{
    auto result(std::make_shared<Set<std::string> >());

    std::set<std::string> todo;
    todo.insert(s);

    while (! todo.empty())
    {
        std::string t(*todo.begin());
        todo.erase(todo.begin());

        result->insert(t);

        for (auto r(begin_repositories()), r_end(end_repositories()) ;
                r != r_end ; ++r)
        {
            auto l((*r)->maybe_expand_licence_nonrecursively(t));
            if (l)
                for (auto i(l->begin()), i_end(l->end()) ;
                        i != i_end ; ++i)
                    if (result->end() == result->find(*i))
                        todo.insert(*i);
        }
    }

    return result;
}

DuplicateSetError::DuplicateSetError(const SetName & s) throw () :
    Exception("A set named '" + stringify(s) + "' already exists")
{
}

namespace paludis
{
    template class WrappedForwardIterator<Environment::RepositoryConstIteratorTag, const std::shared_ptr<Repository> >;
}
