/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010, 2011, 2013 Ciaran McCreesh
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

#include "licenses_conf.hh"
#include <paludis/environment.hh>
#include <paludis/name.hh>
#include <paludis/dep_spec.hh>
#include <paludis/spec_tree.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/match_package.hh>
#include <paludis/util/config_file.hh>
#include <paludis/package_id.hh>
#include <paludis/util/options.hh>
#include <paludis/environments/paludis/paludis_environment.hh>
#include <paludis/environments/paludis/bashable_conf.hh>
#include <paludis/util/log.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/set.hh>
#include <list>
#include <map>
#include <mutex>
#include <unordered_map>
#include <vector>

using namespace paludis;
using namespace paludis::paludis_environment;

typedef std::list<std::string> LicensesList;
typedef std::map<std::shared_ptr<const PackageDepSpec>, LicensesList> PDSToLicensesList;
typedef std::pair<std::shared_ptr<const SetSpecTree>, LicensesList> SetNameEntry;

typedef std::unordered_map<QualifiedPackageName, PDSToLicensesList, Hash<QualifiedPackageName> > SpecificMap;
typedef PDSToLicensesList UnspecificMap;
typedef std::unordered_map<SetName, SetNameEntry, Hash<SetName> > NamedSetMap;

namespace paludis
{
    template<>
    struct Imp<LicensesConf>
    {
        const PaludisEnvironment * const env;

        mutable SpecificMap qualified;
        mutable UnspecificMap unqualified;
        mutable NamedSetMap set;
        mutable std::mutex set_mutex;
        mutable std::mutex expanded_mutex;
        mutable bool expanded;

        Imp(const PaludisEnvironment * const e) :
            env(e),
            expanded(false)
        {
        }
    };
}

LicensesConf::LicensesConf(const PaludisEnvironment * const e) :
    _imp(e)
{
}

LicensesConf::~LicensesConf() = default;

void
LicensesConf::add(const FSPath & filename)
{
    Context context("When adding source '" + stringify(filename) + "' as a licenses file:");

    std::shared_ptr<LineConfigFile> f(make_bashable_conf(filename, { }));
    if (! f)
        return;

    for (const auto & line : *f)
    {
        std::vector<std::string> tokens;
        tokenise_whitespace_quoted(line, std::back_inserter(tokens));

        if (tokens.size() < 2)
            continue;

        try
        {
            std::shared_ptr<PackageDepSpec> d(std::make_shared<PackageDepSpec>(parse_user_package_dep_spec(
                            tokens.at(0), _imp->env,
                            { updso_allow_wildcards, updso_no_disambiguation, updso_throw_if_set })));
            if (d->package_ptr())
            {
                LicensesList & k(_imp->qualified[*d->package_ptr()][d]);
                for (std::vector<std::string>::const_iterator t(next(tokens.begin())), t_end(tokens.end()) ;
                        t != t_end ; ++t)
                    k.push_back(*t);
            }
            else
            {
                LicensesList & k(_imp->unqualified[d]);
                for (std::vector<std::string>::const_iterator t(next(tokens.begin())), t_end(tokens.end()) ;
                        t != t_end ; ++t)
                    k.push_back(*t);
            }
        }
        catch (const GotASetNotAPackageDepSpec &)
        {
            NamedSetMap::iterator i(_imp->set.insert(std::make_pair(SetName(tokens.at(0)), std::make_pair(
                                nullptr, LicensesList()))).first);

            for (std::vector<std::string>::const_iterator t(next(tokens.begin())), t_end(tokens.end()) ;
                    t != t_end ; ++t)
                i->second.second.push_back(*t);
        }
    }
}

namespace
{
    void expand(const Environment * const env, LicensesList & list)
    {
        LicensesList extras;
        for (auto & i : list)
        {
            std::string s(i);
            if (0 == s.compare(0, 1, "-", 0, 1))
            {
                auto l(env->expand_licence(i.substr(1)));
                for (const auto & v : *l)
                    extras.push_back("-" + v);
            }
            else
            {
                auto l(env->expand_licence(i));
                for (const auto & v : *l)
                    extras.push_back(v);
            }
        }

        list.splice(list.end(), extras, extras.begin(), extras.end());
    }
}

bool
LicensesConf::query(const std::string & t, const std::shared_ptr<const PackageID> & e) const
{
    {
        std::unique_lock<std::mutex> lock(_imp->expanded_mutex);
        if (! _imp->expanded)
        {
            _imp->expanded = true;

            for (auto & q : _imp->qualified)
            {
                for (auto & p : q.second)
                    expand(_imp->env, p.second);
            }

            for (auto & p : _imp->unqualified)
                expand(_imp->env, p.second);

            for (auto & p : _imp->set)
                expand(_imp->env, p.second.second);
        }
    }

    /* highest priority: specific */
    bool break_when_done(false);
    {
        SpecificMap::const_iterator i(_imp->qualified.find(e->name()));
        if (i != _imp->qualified.end())
        {
            for (const auto & j : i->second)
            {
                if (! match_package(*_imp->env, *j.first, e, nullptr, { }))
                    continue;

                for (const auto & l : j.second)
                {
                    if (l == t)
                        return true;

                    if (l == "*")
                        return true;

                    if (l == "-*")
                        break_when_done = true;
                }
            }
        }
    }

    if (break_when_done)
        return false;

    /* next: named sets */
    {
        std::unique_lock<std::mutex> lock(_imp->set_mutex);
        for (auto & i : _imp->set)
        {
            if (! i.second.first)
            {
                i.second.first = _imp->env->set(i.first);
                if (! i.second.first)
                {
                    Log::get_instance()->message("paludis_environment.licenses_conf.unknown_set", ll_warning, lc_no_context) << "Set name '"
                        << i.first << "' does not exist";
                    i.second.first = std::make_shared<SetSpecTree>(std::make_shared<AllDepSpec>());
                }
            }

            if (! match_package_in_set(*_imp->env, *i.second.first, e, { }))
                continue;

            for (const auto & l : i.second.second)
            {
                if (l == t)
                    return true;

                if (l == "*")
                    return true;

                if (l == "-*")
                    break_when_done = true;
            }
        }
    }


    if (break_when_done)
        return false;

    /* last: unspecific */
    for (const auto & j : _imp->unqualified)
    {
        if (! match_package(*_imp->env, *j.first, e, nullptr, { }))
            continue;

        for (const auto & l : j.second)
        {
            if (l == t)
                return true;

            if (l == "*")
                return true;
        }
    }

    return false;
}

