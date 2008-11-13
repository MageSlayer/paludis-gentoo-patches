/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008 Ciaran McCreesh
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
#include <paludis/name.hh>
#include <paludis/dep_spec.hh>
#include <paludis/match_package.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/util/config_file.hh>
#include <paludis/util/options.hh>
#include <paludis/package_id.hh>
#include <paludis/environments/paludis/paludis_environment.hh>
#include <paludis/environments/paludis/bashable_conf.hh>
#include <paludis/util/log.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/set.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/tribool.hh>
#include <paludis/choice.hh>
#include <tr1/unordered_map>
#include <algorithm>
#include <list>
#include <vector>

using namespace paludis;
using namespace paludis::paludis_environment;

typedef std::pair<ChoicePrefixName, UnprefixedChoiceName> FlagNamePair;

typedef std::tr1::unordered_map<FlagNamePair, Tribool, Hash<FlagNamePair> > FlagNamePairWithStateMap;
typedef std::list<ChoicePrefixName> MinusStarPrefixList;
typedef std::pair<FlagNamePairWithStateMap, MinusStarPrefixList> UseInfo;
typedef std::pair<std::tr1::shared_ptr<const PackageDepSpec>, UseInfo> PDSWithUseInfo;
typedef std::pair<std::tr1::shared_ptr<const SetSpecTree::ConstItem>, UseInfo> DSWithUseInfo;
typedef std::list<PDSWithUseInfo> PDSWithUseInfoList;
typedef std::tr1::unordered_map<QualifiedPackageName, PDSWithUseInfoList, Hash<QualifiedPackageName> > Qualified;
typedef std::list<PDSWithUseInfo> Unqualified;
typedef std::tr1::unordered_map<SetName, DSWithUseInfo, Hash<SetName> > Sets;

namespace paludis
{
    template<>
    struct Implementation<UseConf>
    {
        const PaludisEnvironment * const env;
        Qualified qualified;
        Unqualified unqualified;
        mutable Mutex set_mutex;
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

    for (LineConfigFile::ConstIterator line(f->begin()), line_end(f->end()) ;
            line != line_end ; ++line)
    {
        std::vector<std::string> tokens;
        tokenise_whitespace_quoted(*line, std::back_inserter(tokens));

        if (tokens.size() < 2)
            continue;

        if ("*" == tokens.at(0))
        {
            Log::get_instance()->message("paludis_environment.use_conf.deprecated", ll_warning, lc_context)
                << "Use of token '*' is deprecated, use '*/*' instead";
            tokens.at(0) = "*/*";
        }

        try
        {
            std::tr1::shared_ptr<PackageDepSpec> d(new PackageDepSpec(parse_user_package_dep_spec(
                            tokens.at(0), _imp->env,
                            UserPackageDepSpecOptions() + updso_allow_wildcards + updso_no_disambiguation + updso_throw_if_set)));

            if (d->additional_requirements_ptr())
            {
                Log::get_instance()->message("paludis_environment.use_conf.bad_spec", ll_warning, lc_context)
                    << "Dependency specification '" << stringify(*d)
                    << "' includes use requirements, which cannot be used in use.conf";
                continue;
            }

            if (d->package_ptr())
            {
                Qualified::iterator ii(_imp->qualified.insert(std::make_pair(*d->package_ptr(), PDSWithUseInfoList())).first);
                PDSWithUseInfoList::iterator i(ii->second.insert(ii->second.end(), PDSWithUseInfo(d, UseInfo())));

                ChoicePrefixName prefix("");
                for (std::vector<std::string>::const_iterator t(next(tokens.begin())), t_end(tokens.end()) ;
                        t != t_end ; ++t)
                {
                    if (*t == "-*")
                        i->second.second.push_back(prefix);
                    else if ('-' == t->at(0))
                        i->second.first.insert(std::make_pair(
                                    FlagNamePair(prefix, UnprefixedChoiceName(t->substr(1))), false)).first->second = false;
                    else if (':' == t->at(t->length() - 1))
                    {
                        std::string p;
                        std::transform(t->begin(), previous(t->end()), std::back_inserter(p), &::tolower);
                        prefix = ChoicePrefixName(p);
                    }
                    else
                        i->second.first.insert(std::make_pair(FlagNamePair(prefix, UnprefixedChoiceName(*t)), true)).first->second = true;
                }
            }
            else
            {
                Unqualified::iterator i(_imp->unqualified.insert(_imp->unqualified.end(), PDSWithUseInfo(d, UseInfo())));

                ChoicePrefixName prefix("");
                for (std::vector<std::string>::const_iterator t(next(tokens.begin())), t_end(tokens.end()) ;
                        t != t_end ; ++t)
                {
                    if (*t == "-*")
                        i->second.second.push_back(prefix);
                    else if ('-' == t->at(0))
                        i->second.first.insert(
                                std::make_pair(FlagNamePair(prefix, UnprefixedChoiceName(t->substr(1))), false)).first->second = false;
                    else if (':' == t->at(t->length() - 1))
                    {
                        std::string p;
                        std::transform(t->begin(), previous(t->end()), std::back_inserter(p), &::tolower);
                        prefix = ChoicePrefixName(p);
                    }
                    else
                        i->second.first.insert(std::make_pair(FlagNamePair(prefix, UnprefixedChoiceName(*t)), true)).first->second = true;
                }
            }
        }
        catch (const GotASetNotAPackageDepSpec &)
        {
            Sets::iterator i(_imp->sets.insert(std::make_pair(SetName(tokens.at(0)), DSWithUseInfo())).first);
            ChoicePrefixName prefix("");
            for (std::vector<std::string>::const_iterator t(next(tokens.begin())), t_end(tokens.end()) ;
                    t != t_end ; ++t)
            {
                if (*t == "-*")
                    i->second.second.second.push_back(prefix);
                else if ('-' == t->at(0))
                    i->second.second.first.insert(std::make_pair(
                                FlagNamePair(prefix, UnprefixedChoiceName(t->substr(1))), false)).first->second = false;
                else if (':' == t->at(t->length() - 1))
                {
                    std::string p;
                    std::transform(t->begin(), previous(t->end()), std::back_inserter(p), &::tolower);
                    prefix = ChoicePrefixName(p);
                }
                else
                    i->second.second.first.insert(std::make_pair(
                                FlagNamePair(prefix, UnprefixedChoiceName(*t)), true)).first->second = true;
            }
        }
    }
}

const Tribool
UseConf::want_choice_enabled(
        const std::tr1::shared_ptr<const PackageID> & id,
        const std::tr1::shared_ptr<const Choice> & choice,
        const UnprefixedChoiceName & f
        ) const
{
    Context context("When checking state of flag prefix '" + stringify(choice->prefix()) +
            "' name '" + stringify(f) + "' for '" + stringify(*id) + "':");

    Tribool result(indeterminate);

    bool ignore_empty_minus_star(! stringify(choice->prefix()).empty());

    /* highest priority: specific */
    Qualified::const_iterator q(_imp->qualified.find(id->name()));
    if (_imp->qualified.end() != q)
    {
        for (PDSWithUseInfoList::const_iterator p(q->second.begin()), p_end(q->second.end()) ; p != p_end ; ++p)
        {
            if (! match_package(*_imp->env, *p->first, *id, MatchPackageOptions()))
                continue;

            FlagNamePairWithStateMap::const_iterator i(p->second.first.find(std::make_pair(choice->prefix(), f)));
            if (p->second.first.end() != i)
                result = i->second;

            if (result.is_indeterminate())
                for (MinusStarPrefixList::const_iterator m(p->second.second.begin()), m_end(p->second.second.end()) ;
                        m != m_end ; ++m)
                {
                    if (stringify(*m).empty() && ignore_empty_minus_star)
                        continue;

                    if (choice->prefix() == *m)
                    {
                        result = false;
                        break;
                    }
                }
        }
    }

    if (! result.is_indeterminate())
        return result;

    /* next: named sets */
    for (Sets::iterator r(_imp->sets.begin()), r_end(_imp->sets.end()) ; r != r_end ; ++r)
    {
        Lock lock(_imp->set_mutex);
        if (! r->second.first)
        {
            r->second.first = _imp->env->set(r->first);
            if (! r->second.first)
            {
                Log::get_instance()->message("paludis_environment.use_conf.unknown_set", ll_warning, lc_no_context) << "Set name '"
                    << r->first << "' does not exist";
                r->second.first.reset(new ConstTreeSequence<SetSpecTree, AllDepSpec>(
                            std::tr1::shared_ptr<AllDepSpec>(new AllDepSpec)));
            }
        }

        if (! match_package_in_set(*_imp->env, *r->second.first, *id, MatchPackageOptions()))
            continue;

        FlagNamePairWithStateMap::const_iterator i(r->second.second.first.find(std::make_pair(choice->prefix(), f)));
        if (i != r->second.second.first.end())
            result = i->second;

        if (result.is_indeterminate())
            for (MinusStarPrefixList::const_iterator m(r->second.second.second.begin()), m_end(r->second.second.second.end()) ;
                    m != m_end ; ++m)
            {
                if (stringify(*m).empty() && ignore_empty_minus_star)
                    continue;

                if (choice->prefix() == *m)
                {
                    result = false;
                    break;
                }
            }
    }

    if (! result.is_indeterminate())
        return result;

    /* last: unspecific */

    for (Unqualified::const_iterator p(_imp->unqualified.begin()), p_end(_imp->unqualified.end()) ; p != p_end ; ++p)
    {
        if (! match_package(*_imp->env, *p->first, *id, MatchPackageOptions()))
            continue;

        FlagNamePairWithStateMap::const_iterator i(p->second.first.find(std::make_pair(choice->prefix(), f)));
        if (p->second.first.end() != i)
            result = i->second;

        if (result.is_indeterminate())
            for (MinusStarPrefixList::const_iterator m(p->second.second.begin()), m_end(p->second.second.end()) ;
                    m != m_end ; ++m)
            {
                if (stringify(*m).empty() && ignore_empty_minus_star)
                    continue;

                if (choice->prefix() == *m)
                {
                    result = false;
                    break;
                }
            }
    }

    return result;
}

std::tr1::shared_ptr<const Set<UnprefixedChoiceName> >
UseConf::known_choice_value_names(
        const std::tr1::shared_ptr<const PackageID> & id,
        const std::tr1::shared_ptr<const Choice> & choice
        ) const
{
    Context context("When loading known use expand names for prefix '" + stringify(choice->prefix()) + "':");

    std::tr1::shared_ptr<Set<UnprefixedChoiceName> > result(new Set<UnprefixedChoiceName>);

    Qualified::const_iterator q(_imp->qualified.find(id->name()));
    if (_imp->qualified.end() != q)
        for (PDSWithUseInfoList::const_iterator p(q->second.begin()), p_end(q->second.end()) ; p != p_end ; ++p)
        {
            if (! match_package(*_imp->env, *p->first, *id, MatchPackageOptions()))
                continue;

            for (FlagNamePairWithStateMap::const_iterator i(p->second.first.begin()), i_end(p->second.first.end()) ;
                    i != i_end ; ++i)
                if (i->first.first == choice->prefix())
                    result->insert(i->first.second);
        }

    {
        Lock lock(_imp->set_mutex);
        for (Sets::iterator r(_imp->sets.begin()), r_end(_imp->sets.end()) ; r != r_end ; ++r)
        {
            if (! r->second.first)
            {
                r->second.first = _imp->env->set(r->first);
                if (! r->second.first)
                {
                    Log::get_instance()->message("paludis_environment.use_conf.unknown_set", ll_warning, lc_no_context) << "Set name '"
                        << r->first << "' does not exist";
                    r->second.first.reset(new ConstTreeSequence<SetSpecTree, AllDepSpec>(
                                std::tr1::shared_ptr<AllDepSpec>(new AllDepSpec)));
                }
            }

            if (! match_package_in_set(*_imp->env, *r->second.first, *id, MatchPackageOptions()))
                continue;

            for (FlagNamePairWithStateMap::const_iterator i(r->second.second.first.begin()), i_end(r->second.second.first.end()) ;
                    i != i_end ; ++i)
                if (i->first.first == choice->prefix())
                    result->insert(i->first.second);
        }
    }

    for (Unqualified::const_iterator p(_imp->unqualified.begin()), p_end(_imp->unqualified.end()) ; p != p_end ; ++p)
    {
        if (! match_package(*_imp->env, *p->first, *id, MatchPackageOptions()))
            continue;

        for (FlagNamePairWithStateMap::const_iterator i(p->second.first.begin()), i_end(p->second.first.end()) ;
                i != i_end ; ++i)
            if (i->first.first == choice->prefix())
                result->insert(i->first.second);
    }

    return result;
}

