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

#include "use_conf.hh"
#include <paludis/environment.hh>
#include <paludis/package_database_entry.hh>
#include <paludis/hashed_containers.hh>
#include <paludis/name.hh>
#include <paludis/dep_spec.hh>
#include <paludis/match_package.hh>
#include <paludis/environments/paludis/paludis_environment.hh>
#include <paludis/environments/paludis/bashable_conf.hh>
#include <paludis/util/log.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/iterator.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/collection_concrete.hh>
#include <list>
#include <vector>

using namespace paludis;

typedef MakeHashedMap<UseFlagName, UseFlagState>::Type UseFlagWithStateMap;
typedef std::list<std::string> MinusStarPrefixList;
typedef std::pair<UseFlagWithStateMap, MinusStarPrefixList> UseInfo;
typedef std::pair<std::tr1::shared_ptr<const PackageDepSpec>, UseInfo> PDSWithUseInfo;
typedef std::pair<std::tr1::shared_ptr<const DepSpec>, UseInfo> DSWithUseInfo;
typedef std::list<PDSWithUseInfo> PDSWithUseInfoList;
typedef MakeHashedMap<QualifiedPackageName, PDSWithUseInfoList>::Type Qualified;
typedef std::list<PDSWithUseInfo> Unqualified;
typedef MakeHashedMap<SetName, DSWithUseInfo>::Type Sets;

namespace paludis
{
    template<>
    struct Implementation<UseConf>
    {
        const PaludisEnvironment * const env;
        Qualified qualified;
        Unqualified unqualified;
        mutable Sets sets;

        Implementation(const PaludisEnvironment * const e) :
            env(e)
        {
        }
    };
}

UseConf::UseConf(const PaludisEnvironment * const e) :
    PrivateImplementationPattern<UseConf>(new Implementation<UseConf>(e))
{
}

UseConf::~UseConf()
{
}

void
UseConf::add(const FSEntry & filename)
{
    Context context("When adding source '" + stringify(filename) + "' as a use file:");

    std::tr1::shared_ptr<LineConfigFile> f(make_bashable_conf(filename));
    if (! f)
        return;

    for (LineConfigFile::Iterator line(f->begin()), line_end(f->end()) ;
            line != line_end ; ++line)
    {
        std::vector<std::string> tokens;
        WhitespaceTokeniser::get_instance()->tokenise(*line, std::back_inserter(tokens));

        if (tokens.size() < 2)
            continue;

        if ("*" == tokens.at(0))
        {
            Log::get_instance()->message(ll_warning, lc_context) << "Use of token '*' is deprecated, use '*/*' instead";
            tokens.at(0) = "*/*";
        }

        if (std::string::npos == tokens.at(0).find("/"))
        {
            Sets::iterator i(_imp->sets.insert(std::make_pair(SetName(tokens.at(0)), DSWithUseInfo())).first);
            std::string prefix_upper, prefix_lower;
            for (std::vector<std::string>::const_iterator t(next(tokens.begin())), t_end(tokens.end()) ;
                    t != t_end ; ++t)
            {
                if (*t == "-*")
                    i->second.second.second.push_back(strip_trailing(prefix_lower, "_"));
                else if ('-' == t->at(0))
                    i->second.second.first.insert(std::make_pair(
                                UseFlagName(prefix_lower + t->substr(1)), use_disabled)).first->second = use_disabled;
                else if (':' == t->at(t->length() - 1))
                {
                    std::transform(t->begin(), previous(t->end()), std::back_inserter(prefix_lower), &::tolower);
                    std::transform(t->begin(), previous(t->end()), std::back_inserter(prefix_upper), &::toupper);
                    prefix_lower.append("_");
                    prefix_upper.append("_");
                }
                else
                    i->second.second.first.insert(std::make_pair(
                                UseFlagName(prefix_lower + *t), use_enabled)).first->second = use_enabled;
            }
        }
        else
        {
            std::tr1::shared_ptr<PackageDepSpec> d(new PackageDepSpec(tokens.at(0), pds_pm_unspecific));

            if (d->use_requirements_ptr())
            {
                Log::get_instance()->message(ll_warning, lc_context) << "Dependency specification '" << stringify(*d)
                    << "' includes use requirements, which cannot be used in use.conf";
                continue;
            }

            if (d->package_ptr())
            {
                Qualified::iterator ii(_imp->qualified.insert(std::make_pair(*d->package_ptr(), PDSWithUseInfoList())).first);
                PDSWithUseInfoList::iterator i(ii->second.insert(ii->second.end(), PDSWithUseInfo(d, UseInfo())));

                std::string prefix_upper, prefix_lower;
                for (std::vector<std::string>::const_iterator t(next(tokens.begin())), t_end(tokens.end()) ;
                        t != t_end ; ++t)
                {
                    if (*t == "-*")
                        i->second.second.push_back(strip_trailing(prefix_lower, "_"));
                    else if ('-' == t->at(0))
                        i->second.first.insert(std::make_pair(
                                    UseFlagName(prefix_lower + t->substr(1)), use_disabled)).first->second = use_disabled;
                    else if (':' == t->at(t->length() - 1))
                    {
                        std::transform(t->begin(), previous(t->end()), std::back_inserter(prefix_lower), &::tolower);
                        std::transform(t->begin(), previous(t->end()), std::back_inserter(prefix_upper), &::toupper);
                        prefix_lower.append("_");
                        prefix_upper.append("_");
                    }
                    else
                        i->second.first.insert(std::make_pair(UseFlagName(prefix_lower + *t), use_enabled)).first->second = use_enabled;
                }
            }
            else
            {
                Unqualified::iterator i(_imp->unqualified.insert(_imp->unqualified.end(), PDSWithUseInfo(d, UseInfo())));

                std::string prefix_upper, prefix_lower;
                for (std::vector<std::string>::const_iterator t(next(tokens.begin())), t_end(tokens.end()) ;
                        t != t_end ; ++t)
                {
                    if (*t == "-*")
                        i->second.second.push_back(strip_trailing(prefix_lower, "_"));
                    else if ('-' == t->at(0))
                        i->second.first.insert(
                                std::make_pair(UseFlagName(prefix_lower + t->substr(1)), use_disabled)).first->second = use_disabled;
                    else if (':' == t->at(t->length() - 1))
                    {
                        std::transform(t->begin(), previous(t->end()), std::back_inserter(prefix_lower), &::tolower);
                        std::transform(t->begin(), previous(t->end()), std::back_inserter(prefix_upper), &::toupper);
                        prefix_lower.append("_");
                        prefix_upper.append("_");
                    }
                    else
                        i->second.first.insert(std::make_pair(UseFlagName(prefix_lower + *t), use_enabled)).first->second = use_enabled;
                }
            }
        }
    }
}

UseFlagState
UseConf::query(const UseFlagName & f, const PackageDatabaseEntry & e) const
{
    Context context("When checking state of flag '" + stringify(f) + "' for '" + stringify(e) + "':");

    UseFlagState result(use_unspecified);

    bool ignore_empty_minus_star(false);
    std::tr1::shared_ptr<const Repository> repo(_imp->env->package_database()->fetch_repository(e.repository));
    if (repo->use_interface)
    {
        std::tr1::shared_ptr<const UseFlagNameCollection> prefixes(repo->use_interface->use_expand_prefixes());
        for (UseFlagNameCollection::Iterator p(prefixes->begin()), p_end(prefixes->end()) ;
                p != p_end ; ++p)
            if (0 == p->data().compare(0, p->data().length(), stringify(f), 0, p->data().length()))
            {
                ignore_empty_minus_star = true;
                break;
            }
    }

    /* highest priority: specific */
    Qualified::const_iterator q(_imp->qualified.find(e.name));
    if (_imp->qualified.end() != q)
    {
        for (PDSWithUseInfoList::const_iterator p(q->second.begin()), p_end(q->second.end()) ; p != p_end ; ++p)
        {
            if (! match_package(*_imp->env, *p->first, e))
                continue;

            UseFlagWithStateMap::const_iterator i(p->second.first.find(f));
            if (p->second.first.end() != i)
                result = i->second;

            if (use_unspecified == result)
                for (MinusStarPrefixList::const_iterator m(p->second.second.begin()), m_end(p->second.second.end()) ;
                        m != m_end ; ++m)
                {
                    if (m->empty() && ignore_empty_minus_star)
                        continue;

                    if (0 == m->compare(0, m->length(), stringify(f), 0, m->length()))
                    {
                        result = use_disabled;
                        break;
                    }
                }
        }
    }

    if (use_unspecified != result)
        return result;

    /* next: named sets */
    for (Sets::iterator r(_imp->sets.begin()), r_end(_imp->sets.end()) ; r != r_end ; ++r)
    {
        if (! r->second.first)
        {
            r->second.first = _imp->env->set(r->first);
            if (! r->second.first)
            {
                Log::get_instance()->message(ll_warning, lc_no_context) << "Set name '"
                    << r->first << "' does not exist";
                r->second.first.reset(new AllDepSpec);
            }
        }

        if (! match_package_in_heirarchy(*_imp->env, *r->second.first, e))
            continue;

        UseFlagWithStateMap::const_iterator i(r->second.second.first.find(f));
        if (i != r->second.second.first.end())
            result = i->second;

        if (use_unspecified == result)
            for (MinusStarPrefixList::const_iterator m(r->second.second.second.begin()), m_end(r->second.second.second.end()) ;
                    m != m_end ; ++m)
            {
                if (m->empty() && ignore_empty_minus_star)
                    continue;

                if (0 == m->compare(0, m->length(), stringify(f), 0, m->length()))
                {
                    result = use_disabled;
                    break;
                }
            }
    }

    if (use_unspecified != result)
        return result;

    /* last: unspecific */

    for (Unqualified::const_iterator p(_imp->unqualified.begin()), p_end(_imp->unqualified.end()) ; p != p_end ; ++p)
    {
        if (! match_package(*_imp->env, *p->first, e))
            continue;

        UseFlagWithStateMap::const_iterator i(p->second.first.find(f));
        if (p->second.first.end() != i)
            result = i->second;

        if (use_unspecified == result)
            for (MinusStarPrefixList::const_iterator m(p->second.second.begin()), m_end(p->second.second.end()) ;
                    m != m_end ; ++m)
            {
                if (m->empty() && ignore_empty_minus_star)
                    continue;

                if (0 == m->compare(0, m->length(), stringify(f), 0, m->length()))
                {
                    result = use_disabled;
                    break;
                }
            }
    }

    return result;
}

std::tr1::shared_ptr<const UseFlagNameCollection>
UseConf::known_use_expand_names(const UseFlagName & prefix, const PackageDatabaseEntry & e) const
{
    Context context("When loading known use expand names for prefix '" + stringify(prefix) + ":");

    std::tr1::shared_ptr<UseFlagNameCollection> result(new UseFlagNameCollection::Concrete);
    std::string prefix_lower;
    std::transform(prefix.data().begin(), prefix.data().end(), std::back_inserter(prefix_lower), &::tolower);
    prefix_lower.append("_");

    Qualified::const_iterator q(_imp->qualified.find(e.name));
    if (_imp->qualified.end() != q)
        for (PDSWithUseInfoList::const_iterator p(q->second.begin()), p_end(q->second.end()) ; p != p_end ; ++p)
        {
            if (! match_package(*_imp->env, *p->first, e))
                continue;

            for (UseFlagWithStateMap::const_iterator i(p->second.first.begin()), i_end(p->second.first.end()) ;
                    i != i_end ; ++i)
                if (0 == i->first.data().compare(0, prefix_lower.length(), prefix_lower))
                    result->insert(i->first);
        }

    for (Sets::iterator r(_imp->sets.begin()), r_end(_imp->sets.end()) ; r != r_end ; ++r)
    {
        if (! r->second.first)
        {
            r->second.first = _imp->env->set(r->first);
            if (! r->second.first)
            {
                Log::get_instance()->message(ll_warning, lc_no_context) << "Set name '"
                    << r->first << "' does not exist";
                r->second.first.reset(new AllDepSpec);
            }
        }

        if (! match_package_in_heirarchy(*_imp->env, *r->second.first, e))
            continue;

        for (UseFlagWithStateMap::const_iterator i(r->second.second.first.begin()), i_end(r->second.second.first.end()) ;
                i != i_end ; ++i)
            if (0 == i->first.data().compare(0, prefix_lower.length(), prefix_lower))
                result->insert(i->first);
    }

    for (Unqualified::const_iterator p(_imp->unqualified.begin()), p_end(_imp->unqualified.end()) ; p != p_end ; ++p)
    {
        if (! match_package(*_imp->env, *p->first, e))
            continue;

        for (UseFlagWithStateMap::const_iterator i(p->second.first.begin()), i_end(p->second.first.end()) ;
                i != i_end ; ++i)
            if (0 == i->first.data().compare(0, prefix_lower.length(), prefix_lower))
                result->insert(i->first);
    }

    return result;
}

