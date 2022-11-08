/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010, 2011, 2013 Ciaran McCreesh
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

#include <paludis/environments/paludis/suggestions_conf.hh>
#include <paludis/environments/paludis/paludis_environment.hh>
#include <paludis/environments/paludis/paludis_config.hh>
#include <paludis/environments/paludis/bashable_conf.hh>
#include <paludis/util/config_file.hh>
#include <paludis/util/options.hh>
#include <paludis/util/log.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/hashes.hh>
#include <paludis/environment.hh>
#include <paludis/name.hh>
#include <paludis/dep_spec.hh>
#include <paludis/spec_tree.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/match_package.hh>
#include <paludis/package_id.hh>
#include <paludis/dep_spec_annotations.hh>
#include <list>
#include <map>
#include <mutex>
#include <unordered_map>
#include <vector>

using namespace paludis;
using namespace paludis::paludis_environment;

namespace
{
    struct ValueFlag
    {
        bool negated;
        std::string cat_requirement;
        std::string pkg_requirement;
        std::string group_requirement;

        ValueFlag(const std::string & s) :
            negated(false)
        {
            std::string s_fixed(s);

            if ((! s_fixed.empty()) && ('-' == s_fixed.at(0)))
            {
                s_fixed.erase(0, 1);
                negated = true;
            }

            if (s_fixed.empty())
                throw PaludisConfigError("Empty value flag '" + s + "'");

            std::string::size_type slash_p(s_fixed.find('/'));
            if (std::string::npos == slash_p)
                group_requirement = s_fixed;
            else
            {
                std::string c(s_fixed.substr(0, slash_p));
                if (c != "*")
                    cat_requirement = c;

                std::string p(s_fixed.substr(slash_p + 1));
                if (p != "*")
                    pkg_requirement = p;
            }
        }
    };
}

typedef std::list<ValueFlag> ValuesList;
typedef std::map<std::shared_ptr<const PackageDepSpec>, ValuesList> PDSToValuesList;
typedef std::pair<std::shared_ptr<const SetSpecTree>, ValuesList> SetNameEntry;

typedef std::unordered_map<QualifiedPackageName, PDSToValuesList, Hash<QualifiedPackageName> > SpecificMap;
typedef PDSToValuesList UnspecificMap;
typedef std::unordered_map<SetName, SetNameEntry, Hash<SetName> > NamedSetMap;

namespace paludis
{
    template<>
    struct Imp<SuggestionsConf>
    {
        const PaludisEnvironment * const env;

        SpecificMap qualified;
        UnspecificMap unqualified;
        mutable NamedSetMap set;
        mutable std::mutex set_mutex;

        Imp(const PaludisEnvironment * const e) :
            env(e)
        {
        }
    };
}

SuggestionsConf::SuggestionsConf(const PaludisEnvironment * const e) :
    _imp(e)
{
}

SuggestionsConf::~SuggestionsConf() = default;

void
SuggestionsConf::add(const FSPath & filename)
{
    Context context("When adding source '" + stringify(filename) + "' as a suggestions file:");

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
                ValuesList & k(_imp->qualified[*d->package_ptr()][d]);
                for (std::vector<std::string>::const_iterator t(next(tokens.begin())), t_end(tokens.end()) ;
                        t != t_end ; ++t)
                    k.push_back(ValueFlag(*t));
            }
            else
            {
                ValuesList & k(_imp->unqualified[d]);
                for (std::vector<std::string>::const_iterator t(next(tokens.begin())), t_end(tokens.end()) ;
                        t != t_end ; ++t)
                    k.push_back(ValueFlag(*t));
            }
        }
        catch (const GotASetNotAPackageDepSpec &)
        {
            NamedSetMap::iterator i(_imp->set.insert(std::make_pair(SetName(tokens.at(0)), std::make_pair(
                                nullptr, ValuesList()))).first);

            for (std::vector<std::string>::const_iterator t(next(tokens.begin())), t_end(tokens.end()) ;
                    t != t_end ; ++t)
                i->second.second.push_back(ValueFlag(*t));
        }
    }
}

Tribool
SuggestionsConf::interest_in_suggestion(
        const std::shared_ptr<const PackageID> & from_id,
        const PackageDepSpec & spec) const
{
    std::string spec_group;
    if (spec.maybe_annotations())
    {
        auto a(spec.maybe_annotations()->find(dsar_suggestions_group_name));
        if (a != spec.maybe_annotations()->end())
            spec_group = a->value();
    }

    /* highest priority: specific */
    {
        SpecificMap::const_iterator i(_imp->qualified.find(from_id->name()));
        if (i != _imp->qualified.end())
        {
            for (const auto & j : i->second)
            {
                if (! match_package(*_imp->env, *j.first, from_id, nullptr, { }))
                    continue;

                for (const auto & l : j.second)
                {
                    if (! l.group_requirement.empty())
                    {
                        if (spec_group == l.group_requirement)
                            return l.negated ? false : true;
                    }
                    else
                    {
                        if (! l.pkg_requirement.empty())
                            if (stringify(spec.package_ptr()->package()) != l.pkg_requirement)
                                continue;
                        if (! l.cat_requirement.empty())
                            if (stringify(spec.package_ptr()->category()) != l.cat_requirement)
                                continue;

                        return l.negated ? false : true;
                    }
                }
            }
        }
    }

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
                    Log::get_instance()->message("paludis_environment.suggestions_conf.unknown_set", ll_warning, lc_no_context) << "Set name '"
                        << i.first << "' does not exist";
                    i.second.first = std::make_shared<SetSpecTree>(std::make_shared<AllDepSpec>());
                }
            }

            if (! match_package_in_set(*_imp->env, *i.second.first, from_id, { }))
                continue;

            for (const auto & l : i.second.second)
            {
                if (! l.group_requirement.empty())
                {
                    if (spec_group == l.group_requirement)
                        return l.negated ? false : true;
                }
                else
                {
                    if (! l.pkg_requirement.empty())
                        if (stringify(spec.package_ptr()->package()) != l.pkg_requirement)
                            continue;
                    if (! l.cat_requirement.empty())
                        if (stringify(spec.package_ptr()->category()) != l.cat_requirement)
                            continue;

                    return l.negated ? false : true;
                }
            }
        }
    }


    /* last: unspecific */
    for (const auto & j : _imp->unqualified)
    {
        if (! match_package(*_imp->env, *j.first, from_id, nullptr, { }))
            continue;

        for (const auto & l : j.second)
        {
            if (! l.group_requirement.empty())
            {
                if (spec_group == l.group_requirement)
                    return l.negated ? false : true;
            }
            else
            {
                if (! l.pkg_requirement.empty())
                    if (stringify(spec.package_ptr()->package()) != l.pkg_requirement)
                        continue;
                if (! l.cat_requirement.empty())
                    if (stringify(spec.package_ptr()->category()) != l.cat_requirement)
                        continue;

                return l.negated ? false : true;
            }
        }
    }

    return indeterminate;
}

namespace paludis
{
    template class Pimp<SuggestionsConf>;
}
