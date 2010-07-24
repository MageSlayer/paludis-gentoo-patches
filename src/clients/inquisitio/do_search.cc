/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010 Ciaran McCreesh
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
#include <paludis/package_database.hh>
#include <paludis/action.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/notifier_callback.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/create_iterator-impl.hh>
#include <paludis/util/forward_parallel_for_each.hh>
#include <paludis/util/system.hh>
#include <paludis/util/destringify.hh>
#include <functional>
#include <list>
#include <set>
#include <map>
#include <iostream>
#include <algorithm>

using namespace paludis;
using namespace inquisitio;

namespace
{
    struct Searched
    {
    };

    struct DisplayCallback
    {
        mutable Mutex mutex;
        mutable unsigned width;
        mutable std::map<std::string, int> metadata, steps;

        DisplayCallback() :
            width(0)
        {
        }

        void operator() (const NotifierCallbackEvent & event) const
        {
            event.accept(*this);
        }

        void visit(const NotifierCallbackGeneratingMetadataEvent & e) const
        {
            Lock lock(mutex);
            ++metadata.insert(std::make_pair(stringify(e.repository()), 0)).first->second;
            update();
        }

        void visit(const NotifierCallbackResolverStageEvent &) const
        {
        }

        void visit(const NotifierCallbackLinkageStepEvent &) const
        {
        }

        void visit(const NotifierCallbackResolverStepEvent &) const
        {
            Lock lock(mutex);
            ++steps.insert(std::make_pair("steps", 0)).first->second;
            update();
        }

        void visit(const Searched &) const
        {
            Lock lock(mutex);
            ++steps.insert(std::make_pair("searched", 0)).first->second;
            update();
        }

        void update() const
        {
            std::string s;
            if (! steps.empty())
            {
                for (std::map<std::string, int>::const_iterator i(steps.begin()), i_end(steps.end()) ;
                        i != i_end ; ++i)
                {
                    if (! s.empty())
                        s.append(", ");

                    s.append(stringify(i->second) + " " + i->first);
                }
            }

            if (! metadata.empty())
            {
                std::multimap<int, std::string> biggest;
                for (std::map<std::string, int>::const_iterator i(metadata.begin()), i_end(metadata.end()) ;
                        i != i_end ; ++i)
                    biggest.insert(std::make_pair(i->second, i->first));

                int t(0), n(0);
                std::string ss;
                for (std::multimap<int, std::string>::const_reverse_iterator i(biggest.rbegin()), i_end(biggest.rend()) ;
                        i != i_end ; ++i)
                {
                    ++n;

                    if (n == 4)
                    {
                        ss.append(", ...");
                        break;
                    }

                    if (n < 4)
                    {
                        if (! ss.empty())
                            ss.append(", ");

                        ss.append(stringify(i->first) + " " + i->second);
                    }

                    t += i->first;
                }

                if (! s.empty())
                    s.append(", ");
                s.append(stringify(t) + " metadata (" + ss + ")");
            }

            std::cout << std::string(width, '\010') << s;

            if (width > s.length())
                std::cout
                    << std::string(width - s.length(), ' ')
                    << std::string(width - s.length(), '\010');

            width = s.length();
            std::cout << std::flush;
        }
    };

    struct Eligible
    {
        typedef bool result;

        const bool visible_only;
        bool installed, installable;

        Eligible(const bool v, const std::string & k) :
            visible_only(v),
            installed(true),
            installable(true)
        {
            if (k == "all")
            {
            }
            else if (k == "installable")
                installed = false;
            else if (k == "installed")
                installable = false;
            else
                throw InternalError(PALUDIS_HERE, "Bad --kind '" + k + "'");
        }

        bool operator() (const PackageID & id) const
        {
            if ((! installed) && id.repository()->installed_root_key())
                return false;

            if ((! installable) && id.supports_action(SupportsActionTest<InstallAction>()))
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

        const std::list<std::shared_ptr<Matcher> > & matchers;
        const std::list<std::shared_ptr<Extractor> > & extractors;

        Matches(
                const std::list<std::shared_ptr<Matcher> > & m,
                const std::list<std::shared_ptr<Extractor> > & e) :
            matchers(m),
            extractors(e)
        {
        }

        bool operator() (const PackageID & id) const
        {
            for (std::list<std::shared_ptr<Extractor> >::const_iterator e(extractors.begin()), e_end(extractors.end()) ;
                    e != e_end ; ++e)
                for (std::list<std::shared_ptr<Matcher> >::const_iterator m(matchers.begin()), m_end(matchers.end()) ;
                        m != m_end ; ++m)
                    if ((**e)(**m, id))
                        return true;

            return false;
        }
    };

    std::shared_ptr<const PackageID> fetch_id(
            const Environment & env,
            const std::shared_ptr<const Repository> & r,
            const QualifiedPackageName & q,
            const std::function<bool (const PackageID &)> & e,
            const std::function<bool (const PackageID &)> & m,
            const bool all_versions,
            const bool invert_match,
            const DisplayCallback & display_callback)
    {
        std::shared_ptr<const PackageIDSequence> ids(r->package_ids(q));
        if (ids->empty())
            return std::shared_ptr<const PackageID>();
        else
        {
            std::list<std::shared_ptr<const PackageID> > sids(ids->begin(), ids->end());
            PackageIDComparator c(env.package_database().get());
            sids.sort(c);

            for (std::list<std::shared_ptr<const PackageID> >::const_reverse_iterator i(sids.rbegin()), i_end(sids.rend()) ;
                    i != i_end ; ++i)
            {
                try
                {
                    display_callback.visit(Searched());
                    if (e(**i))
                    {
                        if (invert_match ^ m(**i))
                            return *i;
                        else
                        {
                            (*i)->can_drop_in_memory_cache();
                            if (! all_versions)
                                return std::shared_ptr<const PackageID>();
                        }
                    }
                    else
                        (*i)->can_drop_in_memory_cache();
                }
                catch (const InternalError &)
                {
                    throw;
                }
                catch (const Exception & ex)
                {
                    Log::get_instance()->message("inquisitio.failure", ll_warning, lc_context)
                        << "Caught exception while handling '" << **i << "': '" << ex.message() << "' (" << ex.what() << ")";
                }
            }

            return std::shared_ptr<const PackageID>();
        }
    }

    void set_id(
            const Environment & env,
            const std::list<std::shared_ptr<const Repository> > & repos,
            std::pair<const QualifiedPackageName, std::shared_ptr<const PackageID> > & q,
            const std::function<bool (const PackageID &)> & e,
            const std::function<bool (const PackageID &)> & m,
            const bool all_versions,
            const bool invert_match,
            const DisplayCallback & display_callback)
    {
        std::shared_ptr<const PackageID> best_id;
        for (std::list<std::shared_ptr<const Repository> >::const_iterator r(repos.begin()), r_end(repos.end()) ;
                r != r_end ; ++r)
        {
            std::shared_ptr<const PackageID> id(fetch_id(env, *r, q.first, e, m, all_versions, invert_match,
                        display_callback));
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
do_search(Environment & env)
{
    using namespace std::placeholders;

    std::cout << "Searching: " << std::flush;
    DisplayCallback display_callback;
    ScopedNotifierCallback display_callback_holder(&env, NotifierCallbackFunction(std::cref(display_callback)));

    if (CommandLine::get_instance()->a_repository.specified() &&
        ! CommandLine::get_instance()->a_kind.specified())
        CommandLine::get_instance()->a_kind.set_argument("all");

    std::list<std::shared_ptr<Matcher> > matchers;
    for (CommandLine::ParametersConstIterator p(CommandLine::get_instance()->begin_parameters()),
            p_end(CommandLine::get_instance()->end_parameters()) ; p != p_end ; ++p)
        matchers.push_back(MatcherFactory::get_instance()->create(
                    CommandLine::get_instance()->a_matcher.argument(), *p));

    std::list<std::shared_ptr<Extractor> > extractors;
    if (CommandLine::get_instance()->a_keys.begin_args() == CommandLine::get_instance()->a_keys.end_args())
        extractors.push_back(std::make_shared<NameDescriptionExtractor>());
    else
        for (args::StringSetArg::ConstIterator i(CommandLine::get_instance()->a_keys.begin_args()),
                i_end(CommandLine::get_instance()->a_keys.end_args()) ; i != i_end ; ++i)
            extractors.push_back(std::make_shared<KeyExtractor>(*i,
                            CommandLine::get_instance()->a_flatten.specified(),
                            CommandLine::get_instance()->a_enabled_only.specified(),
                            env));

    std::list<std::shared_ptr<const Repository> > repos;
    for (PackageDatabase::RepositoryConstIterator r(env.package_database()->begin_repositories()),
            r_end(env.package_database()->end_repositories()) ; r != r_end ; ++r)
    {
        if (CommandLine::get_instance()->a_repository.begin_args() != CommandLine::get_instance()->a_repository.end_args())
            if (CommandLine::get_instance()->a_repository.end_args() ==
                    std::find_if(CommandLine::get_instance()->a_repository.begin_args(),
                        CommandLine::get_instance()->a_repository.end_args(),
                        std::bind(std::equal_to<std::string>(), _1, stringify((*r)->name()))))
                continue;

        if (CommandLine::get_instance()->a_repository_format.begin_args() != CommandLine::get_instance()->a_repository_format.end_args())
            if (CommandLine::get_instance()->a_repository_format.end_args() ==
                    std::find_if(CommandLine::get_instance()->a_repository_format.begin_args(),
                        CommandLine::get_instance()->a_repository_format.end_args(),
                        std::bind(std::equal_to<std::string>(), _1, (*r)->format_key() ? (*r)->format_key()->value() : "?")))
                continue;

        if (CommandLine::get_instance()->a_kind.argument() == "installable")
        {
            if (! (*r)->some_ids_might_support_action(SupportsActionTest<InstallAction>()))
                continue;
        }
        else if (CommandLine::get_instance()->a_kind.argument() == "installed")
        {
            if (! (*r)->installed_root_key())
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
        for (std::list<std::shared_ptr<const Repository> >::const_iterator r(repos.begin()), r_end(repos.end()) ;
                r != r_end ; ++r)
        {
            std::shared_ptr<const CategoryNamePartSet> c((*r)->category_names());
            std::copy(c->begin(), c->end(), std::inserter(cats, cats.begin()));
        }
    }

    std::map<QualifiedPackageName, std::shared_ptr<const PackageID> > ids;
    if (CommandLine::get_instance()->a_package.begin_args() != CommandLine::get_instance()->a_package.end_args())
    {
        for (std::set<CategoryNamePart>::const_iterator c(cats.begin()), c_end(cats.end()) ;
                c != c_end ; ++c)
            for (args::StringSetArg::ConstIterator i(CommandLine::get_instance()->a_package.begin_args()),
                    i_end(CommandLine::get_instance()->a_package.end_args()) ; i != i_end ; ++i)
                ids.insert(std::make_pair(*c + PackageNamePart(*i), std::shared_ptr<const PackageID>()));
    }
    else
    {
        for (std::list<std::shared_ptr<const Repository> >::const_iterator r(repos.begin()), r_end(repos.end()) ;
                r != r_end ; ++r)
            for (std::set<CategoryNamePart>::const_iterator c(cats.begin()), c_end(cats.end()) ;
                    c != c_end ; ++c)
            {
                std::shared_ptr<const QualifiedPackageNameSet> q((*r)->package_names(*c));
                for (QualifiedPackageNameSet::ConstIterator i(q->begin()), i_end(q->end()) ;
                        i != i_end ; ++i)
                    ids.insert(std::make_pair(*i, std::shared_ptr<const PackageID>()));
            }
    }

    Eligible eligible(
            CommandLine::get_instance()->a_visible_only.specified(),
            CommandLine::get_instance()->a_kind.argument());

    Matches matches(
            matchers,
            extractors
            );

    const unsigned n_threads(destringify<int>(getenv_with_default("INQUISITIO_THREADS", "5")));
    forward_parallel_for_each(ids.begin(), ids.end(),
            std::bind(&set_id, std::cref(env), std::cref(repos), _1, eligible, matches,
                CommandLine::get_instance()->a_all_versions.specified(),
                CommandLine::get_instance()->a_not.specified(),
                std::cref(display_callback)),
            n_threads, 10);

    display_callback_holder.remove_now();
    std::cout << std::endl;

    bool any(false);
    InquisitioQueryTask task(&env);
    for (std::map<QualifiedPackageName, std::shared_ptr<const PackageID> >::const_iterator
            i(ids.begin()), i_end(ids.end()) ; i != i_end ; ++i)
        if (i->second)
        {
            task.show(make_package_dep_spec({ }).package(i->first), i->second);
            any = true;
        }

    return any ? 0 : 1;
}

