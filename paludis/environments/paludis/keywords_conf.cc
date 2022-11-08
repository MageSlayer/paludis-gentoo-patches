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

#include "keywords_conf.hh"
#include <paludis/environment.hh>
#include <paludis/name.hh>
#include <paludis/dep_spec.hh>
#include <paludis/spec_tree.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/match_package.hh>
#include <paludis/util/config_file.hh>
#include <paludis/util/options.hh>
#include <paludis/package_id.hh>
#include <paludis/environments/paludis/paludis_environment.hh>
#include <paludis/environments/paludis/bashable_conf.hh>
#include <paludis/util/log.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/set.hh>
#include <paludis/util/hashes.hh>
#include <list>
#include <map>
#include <mutex>
#include <unordered_map>
#include <vector>

using namespace paludis;
using namespace paludis::paludis_environment;

typedef std::list<KeywordName> KeywordsList;
typedef std::map<std::shared_ptr<const PackageDepSpec>, KeywordsList> PDSToKeywordsList;
typedef std::pair<std::shared_ptr<const SetSpecTree>, KeywordsList> SetNameEntry;

typedef std::unordered_map<QualifiedPackageName, PDSToKeywordsList, Hash<QualifiedPackageName> > SpecificMap;
typedef PDSToKeywordsList UnspecificMap;
typedef std::unordered_map<SetName, SetNameEntry, Hash<SetName> > NamedSetMap;

namespace paludis
{
    template<>
    struct Imp<KeywordsConf>
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

KeywordsConf::KeywordsConf(const PaludisEnvironment * const e) :
    _imp(e)
{
}

KeywordsConf::~KeywordsConf() = default;

void
KeywordsConf::add(const FSPath & filename)
{
    Context context("When adding source '" + stringify(filename) + "' as a keywords file:");

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
                            tokens.at(0), _imp->env, { updso_allow_wildcards, updso_no_disambiguation, updso_throw_if_set })));
            if (d->package_ptr())
            {
                KeywordsList & k(_imp->qualified[*d->package_ptr()][d]);
                for (std::vector<std::string>::const_iterator t(next(tokens.begin())), t_end(tokens.end()) ;
                        t != t_end ; ++t)
                    k.push_back(KeywordName(*t));
            }
            else
            {
                KeywordsList & k(_imp->unqualified[d]);
                for (std::vector<std::string>::const_iterator t(next(tokens.begin())), t_end(tokens.end()) ;
                        t != t_end ; ++t)
                    k.push_back(KeywordName(*t));
            }
        }
        catch (const GotASetNotAPackageDepSpec &)
        {
            NamedSetMap::iterator i(_imp->set.insert(std::make_pair(SetName(tokens.at(0)), std::make_pair(
                                nullptr, KeywordsList()))).first);

            for (std::vector<std::string>::const_iterator t(next(tokens.begin())), t_end(tokens.end()) ;
                    t != t_end ; ++t)
                i->second.second.push_back(KeywordName(*t));
        }
    }
}

bool
KeywordsConf::query(const std::shared_ptr<const KeywordNameSet> & k, const std::shared_ptr<const PackageID> & e) const
{
    static const KeywordName star_keyword("*");
    static const KeywordName minus_star_keyword("-*");

    if (k->end() != k->find(star_keyword))
        return true;

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
                    if (l == star_keyword)
                        return true;

                    else if (l == minus_star_keyword)
                        break_when_done = true;

                    else if (k->end() != k->find(l))
                        return true;
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
                    Log::get_instance()->message("paludis_environment.keywords_conf.unknown_set", ll_warning, lc_no_context) << "Set name '"
                        << i.first << "' does not exist";
                    i.second.first = std::make_shared<SetSpecTree>(std::make_shared<AllDepSpec>());
                }
            }

            if (! match_package_in_set(*_imp->env, *i.second.first, e, { }))
                continue;

            for (const auto & l : i.second.second)
            {
                if (k->end() != k->find(l))
                    return true;

                if (l == star_keyword)
                    return true;

                if (l == minus_star_keyword)
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
            if (k->end() != k->find(l))
                return true;

            if (l == star_keyword)
                return true;
        }
    }

    return false;
}

