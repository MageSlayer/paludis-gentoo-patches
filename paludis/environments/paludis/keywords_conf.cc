/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh
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
#include <paludis/hashed_containers.hh>
#include <paludis/name.hh>
#include <paludis/dep_spec.hh>
#include <paludis/match_package.hh>
#include <paludis/util/config_file.hh>
#include <paludis/package_id.hh>
#include <paludis/environments/paludis/paludis_environment.hh>
#include <paludis/environments/paludis/bashable_conf.hh>
#include <paludis/util/log.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/set.hh>
#include <list>
#include <vector>
#include <map>

using namespace paludis;
using namespace paludis::paludis_environment;

typedef std::list<KeywordName> KeywordsList;
typedef std::map<tr1::shared_ptr<const PackageDepSpec>, KeywordsList> PDSToKeywordsList;
typedef std::pair<tr1::shared_ptr<const SetSpecTree::ConstItem>, KeywordsList> SetNameEntry;

typedef MakeHashedMap<QualifiedPackageName, PDSToKeywordsList>::Type SpecificMap;
typedef PDSToKeywordsList UnspecificMap;
typedef MakeHashedMap<SetName, SetNameEntry>::Type NamedSetMap;

namespace paludis
{
    template<>
    struct Implementation<KeywordsConf>
    {
        const PaludisEnvironment * const env;

        SpecificMap qualified;
        UnspecificMap unqualified;
        mutable NamedSetMap set;
        mutable Mutex set_mutex;

        Implementation(const PaludisEnvironment * const e) :
            env(e)
        {
        }
    };
}

KeywordsConf::KeywordsConf(const PaludisEnvironment * const e) :
    PrivateImplementationPattern<KeywordsConf>(new Implementation<KeywordsConf>(e))
{
}

KeywordsConf::~KeywordsConf()
{
}

void
KeywordsConf::add(const FSEntry & filename)
{
    Context context("When adding source '" + stringify(filename) + "' as a keywords file:");

    tr1::shared_ptr<LineConfigFile> f(make_bashable_conf(filename));
    if (! f)
        return;

    for (LineConfigFile::ConstIterator line(f->begin()), line_end(f->end()) ;
            line != line_end ; ++line)
    {
        std::vector<std::string> tokens;
        tokenise_whitespace(*line, std::back_inserter(tokens));

        if (tokens.size() < 2)
            continue;

        if ("*" == tokens.at(0))
        {
            Log::get_instance()->message(ll_warning, lc_context) << "Use of token '*' is deprecated, use '*/*' instead";
            tokens.at(0) = "*/*";
        }

        if (std::string::npos == tokens.at(0).find("/"))
        {
            NamedSetMap::iterator i(_imp->set.insert(std::make_pair(SetName(tokens.at(0)), std::make_pair(
                                tr1::shared_ptr<SetSpecTree::ConstItem>(), KeywordsList()))).first);

            for (std::vector<std::string>::const_iterator t(next(tokens.begin())), t_end(tokens.end()) ;
                    t != t_end ; ++t)
                i->second.second.push_back(KeywordName(*t));
        }
        else
        {
            tr1::shared_ptr<PackageDepSpec> d(new PackageDepSpec(tokens.at(0), pds_pm_unspecific));
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
    }
}

bool
KeywordsConf::query(tr1::shared_ptr<const KeywordNameSet> k, const PackageID & e) const
{
    static const KeywordName star_keyword("*");
    static const KeywordName minus_star_keyword("-*");

    if (k->end() != k->find(star_keyword))
        return true;

    /* highest priority: specific */
    bool break_when_done(false);
    {
        SpecificMap::const_iterator i(_imp->qualified.find(e.name()));
        if (i != _imp->qualified.end())
        {
            for (PDSToKeywordsList::const_iterator j(i->second.begin()), j_end(i->second.end()) ;
                    j != j_end ; ++j)
            {
                if (! match_package(*_imp->env, *j->first, e))
                    continue;

                for (KeywordsList::const_iterator l(j->second.begin()), l_end(j->second.end()) ;
                        l != l_end ; ++l)
                {
                    if (*l == star_keyword)
                        return true;

                    else if (*l == minus_star_keyword)
                        break_when_done = true;

                    else if (k->end() != k->find(*l))
                        return true;
                }
            }
        }
    }

    if (break_when_done)
        return false;

    /* next: named sets */
    {
        Lock lock(_imp->set_mutex);

        for (NamedSetMap::iterator i(_imp->set.begin()), i_end(_imp->set.end()) ;
                 i != i_end ; ++i)
        {
            if (! i->second.first)
            {
                i->second.first = _imp->env->set(i->first);
                if (! i->second.first)
                {
                    Log::get_instance()->message(ll_warning, lc_no_context) << "Set name '"
                        << i->first << "' does not exist";
                    i->second.first.reset(new ConstTreeSequence<SetSpecTree, AllDepSpec>(
                                tr1::shared_ptr<AllDepSpec>(new AllDepSpec)));
                }
            }

            if (! match_package_in_set(*_imp->env, *i->second.first, e))
                continue;

            for (KeywordsList::const_iterator l(i->second.second.begin()), l_end(i->second.second.end()) ;
                    l != l_end ; ++l)
            {
                if (k->end() != k->find(*l))
                    return true;

                if (*l == star_keyword)
                    return true;

                if (*l == minus_star_keyword)
                    break_when_done = true;
            }
        }
    }


    if (break_when_done)
        return false;

    /* last: unspecific */
    for (PDSToKeywordsList::const_iterator j(_imp->unqualified.begin()), j_end(_imp->unqualified.end()) ;
            j != j_end ; ++j)
    {
        if (! match_package(*_imp->env, *j->first, e))
            continue;

        for (KeywordsList::const_iterator l(j->second.begin()), l_end(j->second.end()) ;
                l != l_end ; ++l)
        {
            if (k->end() != k->find(*l))
                return true;

            if (*l == star_keyword)
                return true;
        }
    }

    return false;
}

