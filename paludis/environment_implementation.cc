/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009 Ciaran McCreesh
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
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/system.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/hook.hh>
#include <paludis/distribution.hh>
#include <paludis/selection.hh>
#include <algorithm>
#include <map>
#include "config.h"

using namespace paludis;

namespace paludis
{
    template <>
    struct Implementation<EnvironmentImplementation>
    {
        std::map<unsigned, NotifierCallbackFunction> notifier_callbacks;
    };
}

EnvironmentImplementation::EnvironmentImplementation() :
    PrivateImplementationPattern<EnvironmentImplementation>(new Implementation<EnvironmentImplementation>),
    _imp(PrivateImplementationPattern<EnvironmentImplementation>::_imp)
{
}

EnvironmentImplementation::~EnvironmentImplementation()
{
}


std::tr1::shared_ptr<const FSEntrySequence>
EnvironmentImplementation::bashrc_files() const
{
    return make_shared_ptr(new FSEntrySequence);
}

std::tr1::shared_ptr<const FSEntrySequence>
EnvironmentImplementation::syncers_dirs() const
{
    std::tr1::shared_ptr<FSEntrySequence> result(new FSEntrySequence);
    result->push_back(FSEntry(DATADIR "/paludis/syncers"));
    result->push_back(FSEntry(LIBEXECDIR "/paludis/syncers"));
    return result;
}

std::tr1::shared_ptr<const FSEntrySequence>
EnvironmentImplementation::fetchers_dirs() const
{
    std::tr1::shared_ptr<FSEntrySequence> result(new FSEntrySequence);
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

std::tr1::shared_ptr<const DestinationsSet>
EnvironmentImplementation::default_destinations() const
{
    std::tr1::shared_ptr<DestinationsSet> result(new DestinationsSet);

    for (PackageDatabase::RepositoryConstIterator r(package_database()->begin_repositories()),
            r_end(package_database()->end_repositories()) ;
            r != r_end ; ++r)
        if ((**r).destination_interface())
            if ((**r).destination_interface()->is_default_destination())
                result->insert(*r);

    return result;
}

const std::tr1::shared_ptr<const SetSpecTree>
EnvironmentImplementation::set(const SetName & s) const
{
    {
        const std::tr1::shared_ptr<const SetSpecTree> l(local_set(s));
        if (l)
        {
            Log::get_instance()->message("environment_implementation.local_set", ll_debug, lc_context) << "Set '" << s << "' is a local set";
            return l;
        }
    }

    std::tr1::shared_ptr<SetSpecTree> result;;

    /* these sets always exist, even if empty */
    if (s.data() == "everything" || s.data() == "system" || s.data() == "world" || s.data() == "security")
    {
        Log::get_instance()->message("environment_implementation.standard_set", ll_debug, lc_context) << "Set '" << s << "' is a standard set";
        result.reset(new SetSpecTree(make_shared_ptr(new AllDepSpec)));
    }

    for (PackageDatabase::RepositoryConstIterator r(package_database()->begin_repositories()),
            r_end(package_database()->end_repositories()) ;
            r != r_end ; ++r)
    {
        if (! (**r).sets_interface())
            continue;

        std::tr1::shared_ptr<const SetSpecTree> add((**r).sets_interface()->package_set(s));
        if (add)
        {
            Log::get_instance()->message("environment_implementation.set_found_in_repository", ll_debug, lc_context)
                << "Set '" << s << "' found in '" << (*r)->name() << "'";
            if (! result)
                result.reset(new SetSpecTree(make_shared_ptr(new AllDepSpec)));
            result->root()->append_node(add->root());
        }
    }

    if ("everything" == s.data() || "world" == s.data())
        result->root()->append(make_shared_ptr(new NamedSetDepSpec(SetName("system"))));

    if ("world" == s.data())
    {
        std::tr1::shared_ptr<const SetSpecTree> w(world_set());
        if (w)
            result->root()->append_node(w->root());
    }

    if (! result)
        Log::get_instance()->message("environment_implementation.no_match_for_set", ll_debug, lc_context) << "No match for set '" << s << "'";
    return result;
}

std::string
EnvironmentImplementation::distribution() const
{
    static const std::string result(getenv_with_default("PALUDIS_DISTRIBUTION", DEFAULT_DISTRIBUTION));
    return result;
}

std::tr1::shared_ptr<const SetNameSet>
EnvironmentImplementation::set_names() const
{
    return make_shared_ptr(new SetNameSet);
}

bool
EnvironmentImplementation::is_paludis_package(const QualifiedPackageName & n) const
{
    return stringify(n) == (*DistributionData::get_instance()->distribution_from_string(distribution())).paludis_package();
}

std::tr1::shared_ptr<PackageIDSequence>
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

