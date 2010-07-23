/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010 Ciaran McCreesh
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
#include <paludis/package_database.hh>
#include <paludis/util/log.hh>
#include <paludis/util/save.hh>
#include <paludis/util/set.hh>
#include <paludis/util/system.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/member_iterator-impl.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/hook.hh>
#include <paludis/distribution.hh>
#include <paludis/selection.hh>
#include <paludis/selection_cache.hh>
#include <algorithm>
#include <map>
#include <list>
#include "config.h"

using namespace paludis;

namespace
{
    typedef std::function<void (const SetName &) > CombiningFunction;

    struct CombineSets
    {
        std::shared_ptr<AllDepSpec> root;
        std::shared_ptr<SetSpecTree> tree;

        void add(const SetName & s)
        {
            tree->root()->append(std::make_shared<NamedSetDepSpec>(s));
        }

        CombineSets() :
            root(std::make_shared<AllDepSpec>()),
            tree(std::make_shared<SetSpecTree>(root))
        {
        }

        CombineSets(const CombineSets & other) :
            root(other.root),
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
    struct Imp<EnvironmentImplementation>
    {
        std::map<unsigned, NotifierCallbackFunction> notifier_callbacks;
        std::list<std::shared_ptr<const SelectionCache> > selection_caches;

        mutable Mutex sets_mutex;
        mutable bool loaded_sets;
        std::shared_ptr<SetNameSet> set_names;
        SetsStore sets;

        Imp() :
            loaded_sets(false)
        {
        }
    };
}

EnvironmentImplementation::EnvironmentImplementation() :
    Pimp<EnvironmentImplementation>(),
    _imp(Pimp<EnvironmentImplementation>::_imp)
{
}

EnvironmentImplementation::~EnvironmentImplementation()
{
}


std::shared_ptr<const FSEntrySequence>
EnvironmentImplementation::bashrc_files() const
{
    return std::make_shared<FSEntrySequence>();
}

std::shared_ptr<const FSEntrySequence>
EnvironmentImplementation::syncers_dirs() const
{
    std::shared_ptr<FSEntrySequence> result(std::make_shared<FSEntrySequence>());
    result->push_back(FSEntry(DATADIR "/paludis/syncers"));
    result->push_back(FSEntry(LIBEXECDIR "/paludis/syncers"));
    return result;
}

std::shared_ptr<const FSEntrySequence>
EnvironmentImplementation::fetchers_dirs() const
{
    std::shared_ptr<FSEntrySequence> result(std::make_shared<FSEntrySequence>());
    std::string fetchers_dir(getenv_with_default("PALUDIS_FETCHERS_DIR", ""));
    if (fetchers_dir.empty())
    {
        result->push_back(FSEntry(DATADIR "/paludis/fetchers"));
        result->push_back(FSEntry(LIBEXECDIR "/paludis/fetchers"));
    }
    else
        result->push_back(FSEntry(fetchers_dir));
    return result;
}

std::shared_ptr<const DestinationsSet>
EnvironmentImplementation::default_destinations() const
{
    std::shared_ptr<DestinationsSet> result(std::make_shared<DestinationsSet>());

    for (PackageDatabase::RepositoryConstIterator r(package_database()->begin_repositories()),
            r_end(package_database()->end_repositories()) ;
            r != r_end ; ++r)
        if ((**r).destination_interface())
            if ((**r).destination_interface()->is_default_destination())
                result->insert(*r);

    return result;
}

std::string
EnvironmentImplementation::distribution() const
{
    static const std::string result(getenv_with_default("PALUDIS_DISTRIBUTION", DEFAULT_DISTRIBUTION));
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
    if (_imp->selection_caches.empty())
        return selection.perform_select(this);
    else
        return _imp->selection_caches.back()->perform_select(this, selection);
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
EnvironmentImplementation::add_selection_cache(const std::shared_ptr<const SelectionCache> & c)
{
    _imp->selection_caches.push_back(c);
}

void
EnvironmentImplementation::remove_selection_cache(const std::shared_ptr<const SelectionCache> & c)
{
    _imp->selection_caches.remove(c);
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

    for (PackageDatabase::RepositoryConstIterator r(package_database()->begin_repositories()),
            r_end(package_database()->end_repositories()) ;
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
        result->root()->append(std::make_shared<NamedSetDepSpec>(SetName("installed-packages")));
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

DuplicateSetError::DuplicateSetError(const SetName & s) throw () :
    Exception("A set named '" + stringify(s) + "' already exists")
{
}

