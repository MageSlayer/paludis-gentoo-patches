/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010, 2011 Ciaran McCreesh
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

#include "cmd_find_candidates.hh"
#include "search_extras_handle.hh"

#include <paludis/args/args.hh>
#include <paludis/args/do_help.hh>

#include <paludis/name.hh>
#include <paludis/environment.hh>
#include <paludis/repository.hh>
#include <paludis/generator.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/filter.hh>
#include <paludis/filter_handler.hh>
#include <paludis/selection.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/package_id.hh>
#include <paludis/mask.hh>
#include <paludis/match_package.hh>

#include <paludis/util/set.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/make_shared_copy.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/visitor_cast.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/util/stringify.hh>

#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <list>
#include <set>

#include "command_command_line.hh"

using namespace paludis;
using namespace cave;
using std::cout;
using std::endl;

namespace
{
    struct FindCandidatesCommandLine :
        CaveCommandCommandLine
    {
        virtual std::string app_name() const
        {
            return "cave find-candidates";
        }

        virtual std::string app_synopsis() const
        {
            return "Find a list of candidate packages for a search.";
        }

        virtual std::string app_description() const
        {
            return "Finds a list of candidate packages for a search. Used by 'cave search'; not generally called "
                "directly by end users. Note that this command will often select candidates that do not actually "
                "match some of the supplied restrictions; use 'cave match' to obtain accurate results.";
        }

        SearchCommandLineCandidateOptions search_options;
        SearchCommandLineMatchOptions match_options;
        SearchCommandLineIndexOptions index_options;

        args::ArgsGroup g_hints;
        args::StringArg a_name_description_substring_hint;

        FindCandidatesCommandLine() :
            search_options(this),
            match_options(this),
            index_options(this),
            g_hints(main_options_section(), "Hints", "Hints allow, but do not require, the search to return a "
                    "reduced set of results"),
            a_name_description_substring_hint(&g_hints, "name-description-substring", '\0',
                    "Candidates whose name or description does not include the specified string as a substring "
                    "may be omitted")
        {
        }
    };

    void print_spec(const PackageDepSpec & spec)
    {
        cout << spec << endl;
    }

    void no_step(const std::string &)
    {
    }

    void check_candidates(
            const std::function<void (const PackageDepSpec &)> & yield,
            const std::function<void (const std::string &)> & step,
            const std::shared_ptr<const PackageIDSequence> & ids)
    {
        for (PackageIDSequence::ConstIterator i(ids->begin()), i_end(ids->end()) ;
                i != i_end ; ++i)
        {
            step("Checking candidates");
            yield((*i)->uniquely_identifying_spec());
        }
    }
}

int
FindCandidatesCommand::run(
        const std::shared_ptr<Environment> & env,
        const std::shared_ptr<const Sequence<std::string > > & args
        )
{
    FindCandidatesCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_FIND_CANDIDATES_OPTIONS", "CAVE_FIND_CANDIDATES_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    if (cmdline.begin_parameters() != cmdline.end_parameters())
        throw args::DoHelp("find-candidates takes no parameters");

    return run_hosted(env, cmdline.search_options, cmdline.match_options, cmdline.index_options,
            cmdline.a_name_description_substring_hint.argument(), &print_spec, &no_step);
}

typedef std::set<RepositoryName> RepositoryNames;
typedef std::set<CategoryNamePart> CategoryNames;
typedef std::set<QualifiedPackageName> QualifiedPackageNames;

int
FindCandidatesCommand::run_hosted(
        const std::shared_ptr<Environment> & env,
        const SearchCommandLineCandidateOptions & search_options,
        const SearchCommandLineMatchOptions &,
        const SearchCommandLineIndexOptions & index_options,
        const std::string & name_description_substring_hint,
        const std::function<void (const PackageDepSpec &)> & yield,
        const std::function<void (const std::string &)> & step)
{
    int retcode(0);

    if (index_options.a_index.specified())
    {
        step("Searching index");

        CaveSearchExtrasDB * db(SearchExtrasHandle::get_instance()->open_db_function(stringify(index_options.a_index.argument()).c_str()));

        std::list<std::string> specs;

        SearchExtrasHandle::get_instance()->find_candidates_function(db, specs,
                search_options.a_all_versions.specified(),
                search_options.a_visible.specified(),
                name_description_substring_hint);

        SearchExtrasHandle::get_instance()->cleanup_db_function(db);

        for (auto s(specs.begin()), s_end(specs.end()) ;
                s != s_end ; ++s)
        {
            step("Checking indexed candidates");

            std::list<PackageDepSpec> matches;
            for (args::StringSetArg::ConstIterator k(search_options.a_matching.begin_args()),
                    k_end(search_options.a_matching.end_args()) ;
                    k != k_end ; ++k)
                matches.push_back(parse_user_package_dep_spec(*k, env.get(), { updso_allow_wildcards }));

            if (! matches.empty())
            {
                bool ok(false);

                for (auto m(matches.begin()), m_end(matches.end()) ;
                        m != m_end ; ++m)
                    if (match_package(*env, *m, *(*env)[selection::RequireExactlyOne(generator::Matches(
                                        parse_user_package_dep_spec(*s, env.get(), { }), make_null_shared_ptr(), { }))]->begin(), make_null_shared_ptr(), { }))
                    {
                        ok = true;
                        break;
                    }

                if (! ok)
                    continue;
            }

            yield(parse_user_package_dep_spec(*s, env.get(), { }));
        }
    }
    else if (! search_options.a_matching.specified())
    {
        step("Searching repositories");

        RepositoryNames repository_names;
        for (Environment::RepositoryConstIterator r(env->begin_repositories()),
                r_end(env->end_repositories()) ; r != r_end ; ++r)
            repository_names.insert((*r)->name());

        step("Searching categories");

        CategoryNames category_names;
        for (RepositoryNames::const_iterator r(repository_names.begin()), r_end(repository_names.end()) ;
                r != r_end ; ++r)
        {
            const std::shared_ptr<const Repository> repo(env->fetch_repository(*r));
            const std::shared_ptr<const CategoryNamePartSet> cats(repo->category_names({ }));
            std::copy(cats->begin(), cats->end(), std::inserter(category_names, category_names.end()));
        }

        step("Searching packages");

        QualifiedPackageNames package_names;
        for (RepositoryNames::const_iterator r(repository_names.begin()), r_end(repository_names.end()) ;
                r != r_end ; ++r)
        {
            const std::shared_ptr<const Repository> repo(env->fetch_repository(*r));
            for (CategoryNames::const_iterator c(category_names.begin()), c_end(category_names.end()) ;
                    c != c_end ; ++c)
            {
                const std::shared_ptr<const QualifiedPackageNameSet> qpns(repo->package_names(*c, { }));
                std::copy(qpns->begin(), qpns->end(), std::inserter(package_names, package_names.end()));
            }
        }

        step("Searching versions");

        for (QualifiedPackageNames::const_iterator q(package_names.begin()), q_end(package_names.end()) ;
                q != q_end ; ++q)
        {
            try
            {
                if (search_options.a_all_versions.specified())
                {
                    if (search_options.a_visible.specified())
                    {
                        const auto ids((*env)[selection::AllVersionsUnsorted(generator::Package(*q) | filter::NotMasked())]);
                        check_candidates(yield, step, ids);
                    }
                    else
                    {
                        const auto ids((*env)[selection::AllVersionsUnsorted(generator::Package(*q))]);
                        check_candidates(yield, step, ids);
                    }
                }
                else
                {
                    std::shared_ptr<const PackageIDSequence> ids;

                    ids = ((*env)[selection::BestVersionOnly(generator::Package(*q) | filter::SupportsAction<InstallAction>() | filter::NotMasked())]);

                    if (search_options.a_visible.specified())
                    {
                        if (ids->empty())
                            ids = ((*env)[selection::BestVersionOnly(generator::Package(*q) | filter::NotMasked())]);
                    }
                    else
                    {
                        if (ids->empty())
                            ids = ((*env)[selection::BestVersionOnly(generator::Package(*q) | filter::SupportsAction<InstallAction>())]);
                        if (ids->empty())
                            ids = ((*env)[selection::BestVersionOnly(generator::Package(*q))]);
                    }

                    check_candidates(yield, step, ids);
                }
            }
            catch (const InternalError &)
            {
                throw;
            }
            catch (const Exception & e)
            {
                std::cerr << "When processing '" << *q << "' got exception '" << e.message() << "' (" << e.what() << ")" << std::endl;
                retcode |= 1;
            }
        }
    }
    else
    {
        step("Searching matches");

        std::shared_ptr<Generator> match_generator;
        std::shared_ptr<Filter> mask_filter;

        if (search_options.a_visible.specified())
            mask_filter = std::make_shared<filter::NotMasked>();
        else
            mask_filter = std::make_shared<filter::All>();

        for (args::StringSetArg::ConstIterator k(search_options.a_matching.begin_args()),
                k_end(search_options.a_matching.end_args()) ;
                k != k_end ; ++k)
        {
            generator::Matches m(parse_user_package_dep_spec(*k, env.get(), { updso_allow_wildcards }), make_null_shared_ptr(), { });

            if (match_generator)
                match_generator = std::make_shared<generator::Union>(*match_generator, m);
            else
                match_generator = make_shared_copy(m);
        }

        if (search_options.a_all_versions.specified())
        {
            const std::shared_ptr<const PackageIDSequence> ids((*env)[selection::AllVersionsUnsorted(
                        *match_generator | *mask_filter)]);
            check_candidates(yield, step, ids);
        }
        else
        {
            const std::shared_ptr<const PackageIDSequence> ids((*env)[selection::BestVersionOnly(
                        *match_generator | *mask_filter)]);
            check_candidates(yield, step, ids);
        }
    }

    return retcode;
}

std::shared_ptr<args::ArgsHandler>
FindCandidatesCommand::make_doc_cmdline()
{
    return std::make_shared<FindCandidatesCommandLine>();
}

CommandImportance
FindCandidatesCommand::importance() const
{
    return ci_internal;
}

