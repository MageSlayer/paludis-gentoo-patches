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

#include "do_search.hh"
#include "command_line.hh"
#include "matcher.hh"
#include "extractor.hh"
#include "key_extractor.hh"
#include "name_description_extractor.hh"
#include "query_task.hh"

#include <paludis/environment.hh>
#include <paludis/query.hh>
#include <paludis/package_database.hh>
#include <paludis/action.hh>
#include <paludis/package_id.hh>
#include <paludis/util/iterator.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/tr1_functional.hh>
#include <paludis/util/parallel_for_each.hh>
#include <list>
#include <set>
#include <map>
#include <iostream>
#include <algorithm>

using namespace paludis;
using namespace inquisitio;

namespace
{
    struct Eligible
    {
        typedef bool result;

        const bool visible_only;
        tr1::shared_ptr<SupportsActionTestBase> action_test;

        Eligible(const bool v, const std::string & k) :
            visible_only(v)
        {
            if (k == "all")
            {
            }
            else if (k == "installable")
                action_test.reset(new SupportsActionTest<InstallAction>());
            else if (k == "installed")
                action_test.reset(new SupportsActionTest<InstalledAction>());
            else
                throw InternalError(PALUDIS_HERE, "Bad --kind '" + k + "'");
        }

        bool operator() (const PackageID & id) const
        {
            if (action_test && ! id.supports_action(*action_test))
                return false;

            if (visible_only)
                return ! id.masked();
            else
                return true;
        }
    };

    struct Matches
    {
        typedef bool result;

        const std::list<tr1::shared_ptr<Matcher> > & matchers;
        const std::list<tr1::shared_ptr<Extractor> > & extractors;

        Matches(
                const std::list<tr1::shared_ptr<Matcher> > & m,
                const std::list<tr1::shared_ptr<Extractor> > & e) :
            matchers(m),
            extractors(e)
        {
        }

        bool operator() (const PackageID & id) const
        {
            for (std::list<tr1::shared_ptr<Extractor> >::const_iterator e(extractors.begin()), e_end(extractors.end()) ;
                    e != e_end ; ++e)
                for (std::list<tr1::shared_ptr<Matcher> >::const_iterator m(matchers.begin()), m_end(matchers.end()) ;
                        m != m_end ; ++m)
                if ((**e)(**m, id))
                    return true;

            return false;
        }
    };

    tr1::shared_ptr<const PackageID> fetch_id(
            const Environment & env,
            const tr1::shared_ptr<const Repository> & r,
            const QualifiedPackageName & q,
            const tr1::function<bool (const PackageID &)> & e,
            const tr1::function<bool (const PackageID &)> & m)
    {
        tr1::shared_ptr<const PackageIDSequence> ids(r->package_ids(q));
        if (ids->empty())
            return tr1::shared_ptr<const PackageID>();
        else
        {
            std::list<tr1::shared_ptr<const PackageID> > sids(ids->begin(), ids->end());
            PackageIDComparator c(env.package_database().get());
            sids.sort(tr1::ref(c));

            for (std::list<tr1::shared_ptr<const PackageID> >::const_reverse_iterator i(sids.rbegin()), i_end(sids.rend()) ;
                    i != i_end ; ++i)
                if (e(**i))
                {
                    if (m(**i))
                        return *i;
                    else
                        return tr1::shared_ptr<const PackageID>();
                }

            return tr1::shared_ptr<const PackageID>();
        }
    }

    void set_id(
            const Environment & env,
            const std::list<tr1::shared_ptr<const Repository> > & repos,
            std::pair<const QualifiedPackageName, tr1::shared_ptr<const PackageID> > & q,
            const tr1::function<bool (const PackageID &)> & e,
            const tr1::function<bool (const PackageID &)> & m)
    {
        tr1::shared_ptr<const PackageID> best_id;
        for (std::list<tr1::shared_ptr<const Repository> >::const_iterator r(repos.begin()), r_end(repos.end()) ;
                r != r_end ; ++r)
        {
            tr1::shared_ptr<const PackageID> id(fetch_id(env, *r, q.first, e, m));
            if (id)
            {
                if (best_id)
                {
                    PackageIDComparator c(env.package_database().get());
                    if (c(best_id, id))
                        best_id = id;
                }
                else
                    best_id = id;
            }
        }

        q.second = best_id;
    }
}

int
do_search(const Environment & env)
{
    using namespace tr1::placeholders;

    std::list<tr1::shared_ptr<Matcher> > matchers;
    for (CommandLine::ParametersConstIterator p(CommandLine::get_instance()->begin_parameters()),
            p_end(CommandLine::get_instance()->end_parameters()) ; p != p_end ; ++p)
        matchers.push_back(MatcherMaker::get_instance()->find_maker(
                CommandLine::get_instance()->a_matcher.argument())(*p));

    std::list<tr1::shared_ptr<Extractor> > extractors;
    if (CommandLine::get_instance()->a_keys.begin_args() == CommandLine::get_instance()->a_keys.end_args())
        extractors.push_back(make_shared_ptr(new NameDescriptionExtractor));
    else
        for (args::StringSetArg::ConstIterator i(CommandLine::get_instance()->a_keys.begin_args()),
                i_end(CommandLine::get_instance()->a_keys.end_args()) ; i != i_end ; ++i)
            extractors.push_back(make_shared_ptr(new KeyExtractor(*i,
                            CommandLine::get_instance()->a_flatten.specified(),
                            CommandLine::get_instance()->a_enabled_only.specified(),
                            env)));

    std::list<tr1::shared_ptr<const Repository> > repos;
    for (PackageDatabase::RepositoryConstIterator r(env.package_database()->begin_repositories()),
            r_end(env.package_database()->end_repositories()) ; r != r_end ; ++r)
    {
        if (CommandLine::get_instance()->a_repository.begin_args() != CommandLine::get_instance()->a_repository.end_args())
            if (CommandLine::get_instance()->a_repository.end_args() ==
                    std::find_if(CommandLine::get_instance()->a_repository.begin_args(),
                        CommandLine::get_instance()->a_repository.end_args(),
                        tr1::bind(std::equal_to<std::string>(), _1, stringify((*r)->name()))))
                continue;

        if (CommandLine::get_instance()->a_repository_format.begin_args() != CommandLine::get_instance()->a_repository_format.end_args())
            if (CommandLine::get_instance()->a_repository_format.end_args() ==
                    std::find_if(CommandLine::get_instance()->a_repository_format.begin_args(),
                        CommandLine::get_instance()->a_repository_format.end_args(),
                        tr1::bind(std::equal_to<std::string>(), _1, stringify((*r)->format()))))
                continue;

        if (CommandLine::get_instance()->a_kind.argument() == "installable")
        {
            if (! (*r)->some_ids_might_support_action(SupportsActionTest<InstallAction>()))
                continue;
        }
        else if (CommandLine::get_instance()->a_kind.argument() == "installed")
        {
            if (! (*r)->some_ids_might_support_action(SupportsActionTest<InstalledAction>()))
                continue;
        }
        else if (CommandLine::get_instance()->a_kind.argument() == "all")
        {
        }
        else
            throw InternalError(PALUDIS_HERE, "Bad --kind '" + CommandLine::get_instance()->a_kind.argument() + "'");

        repos.push_back(*r);
    }

    std::set<CategoryNamePart> cats;
    if (CommandLine::get_instance()->a_category.begin_args() != CommandLine::get_instance()->a_category.end_args())
        std::copy(CommandLine::get_instance()->a_category.begin_args(), CommandLine::get_instance()->a_category.end_args(),
                create_inserter<CategoryNamePart>(std::inserter(cats, cats.begin())));
    else
    {
        for (std::list<tr1::shared_ptr<const Repository> >::const_iterator r(repos.begin()), r_end(repos.end()) ;
                r != r_end ; ++r)
        {
            tr1::shared_ptr<const CategoryNamePartSet> c((*r)->category_names());
            std::copy(c->begin(), c->end(), std::inserter(cats, cats.begin()));
        }
    }

    std::map<QualifiedPackageName, tr1::shared_ptr<const PackageID> > ids;
    if (CommandLine::get_instance()->a_package.begin_args() != CommandLine::get_instance()->a_package.end_args())
    {
        for (std::set<CategoryNamePart>::const_iterator c(cats.begin()), c_end(cats.end()) ;
                c != c_end ; ++c)
            for (args::StringSetArg::ConstIterator i(CommandLine::get_instance()->a_package.begin_args()),
                    i_end(CommandLine::get_instance()->a_package.end_args()) ; i != i_end ; ++i)
                ids.insert(std::make_pair(*c + PackageNamePart(*i), tr1::shared_ptr<const PackageID>()));
    }
    else
    {
        for (std::list<tr1::shared_ptr<const Repository> >::const_iterator r(repos.begin()), r_end(repos.end()) ;
                r != r_end ; ++r)
            for (std::set<CategoryNamePart>::const_iterator c(cats.begin()), c_end(cats.end()) ;
                    c != c_end ; ++c)
            {
                tr1::shared_ptr<const QualifiedPackageNameSet> q((*r)->package_names(*c));
                for (QualifiedPackageNameSet::ConstIterator i(q->begin()), i_end(q->end()) ;
                        i != i_end ; ++i)
                    ids.insert(std::make_pair(*i, tr1::shared_ptr<const PackageID>()));
            }
    }

    Eligible eligible(
            CommandLine::get_instance()->a_visible_only.specified(),
            CommandLine::get_instance()->a_kind.argument());

    Matches matches(
            matchers,
            extractors
            );

    parallel_for_each(ids.begin(), ids.end(), tr1::bind(&set_id, tr1::cref(env), tr1::cref(repos), _1, eligible, matches));

    bool any(false);
    InquisitioQueryTask task(&env);
    for (std::map<QualifiedPackageName, tr1::shared_ptr<const PackageID> >::const_iterator
            i(ids.begin()), i_end(ids.end()) ; i != i_end ; ++i)
        if (i->second)
        {
            task.show(PackageDepSpec(make_shared_ptr(new QualifiedPackageName(i->first))), i->second);
            any = true;
        }

    return any ? 0 : 1;
}

