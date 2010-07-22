/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010 Ciaran McCreesh
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
#include <paludis/args/args.hh>
#include <paludis/args/do_help.hh>
#include <paludis/name.hh>
#include <paludis/environment.hh>
#include <paludis/package_database.hh>
#include <paludis/repository.hh>
#include <paludis/util/set.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/make_shared_copy.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/generator.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/filter.hh>
#include <paludis/filter_handler.hh>
#include <paludis/selection.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/package_id.hh>
#include <paludis/mask.hh>
#include <cstdlib>
#include <iostream>
#include <algorithm>
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

        FindCandidatesCommandLine() :
            search_options(this),
            match_options(this)
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

    if (cmdline.begin_parameters() == cmdline.end_parameters())
        throw args::DoHelp("find-candidates requires at least one parameter");

    const std::shared_ptr<Set<std::string> > patterns(new Set<std::string>);
    std::copy(cmdline.begin_parameters(), cmdline.end_parameters(), patterns->inserter());

    run_hosted(env, cmdline.search_options, cmdline.match_options, patterns, &print_spec, &no_step);

    return EXIT_SUCCESS;
}

typedef std::set<RepositoryName> RepositoryNames;
typedef std::set<CategoryNamePart> CategoryNames;
typedef std::set<QualifiedPackageName> QualifiedPackageNames;

void
FindCandidatesCommand::run_hosted(
        const std::shared_ptr<Environment> & env,
        const SearchCommandLineCandidateOptions & search_options,
        const SearchCommandLineMatchOptions &,
        const std::shared_ptr<const Set<std::string> > &,
        const std::function<void (const PackageDepSpec &)> & yield,
        const std::function<void (const std::string &)> & step)
{
    if (! search_options.a_matching.specified())
    {
        step("Searching repositories");

        RepositoryNames repository_names;
        for (PackageDatabase::RepositoryConstIterator r(env->package_database()->begin_repositories()),
                r_end(env->package_database()->end_repositories()) ; r != r_end ; ++r)
            repository_names.insert((*r)->name());

        step("Searching categories");

        CategoryNames category_names;
        for (RepositoryNames::const_iterator r(repository_names.begin()), r_end(repository_names.end()) ;
                r != r_end ; ++r)
        {
            const std::shared_ptr<const Repository> repo(env->package_database()->fetch_repository(*r));
            const std::shared_ptr<const CategoryNamePartSet> cats(repo->category_names());
            std::copy(cats->begin(), cats->end(), std::inserter(category_names, category_names.end()));
        }

        step("Searching packages");

        QualifiedPackageNames package_names;
        for (RepositoryNames::const_iterator r(repository_names.begin()), r_end(repository_names.end()) ;
                r != r_end ; ++r)
        {
            const std::shared_ptr<const Repository> repo(env->package_database()->fetch_repository(*r));
            for (CategoryNames::const_iterator c(category_names.begin()), c_end(category_names.end()) ;
                    c != c_end ; ++c)
            {
                const std::shared_ptr<const QualifiedPackageNameSet> qpns(repo->package_names(*c));
                std::copy(qpns->begin(), qpns->end(), std::inserter(package_names, package_names.end()));
            }
        }

        step("Searching versions");

        for (QualifiedPackageNames::const_iterator q(package_names.begin()), q_end(package_names.end()) ;
                q != q_end ; ++q)
        {
            if (search_options.a_all_versions.specified())
            {
                const std::shared_ptr<const PackageIDSequence> ids((*env)[selection::AllVersionsUnsorted(
                            generator::Package(*q))]);
                check_candidates(yield, step, ids);
            }
            else
            {
                std::shared_ptr<const PackageIDSequence> ids;

                ids = ((*env)[selection::BestVersionOnly(generator::Package(*q) | filter::SupportsAction<InstallAction>() | filter::NotMasked())]);
                if (ids->empty())
                    ids = ((*env)[selection::BestVersionOnly(generator::Package(*q) | filter::SupportsAction<InstallAction>())]);
                if (ids->empty())
                    ids = ((*env)[selection::BestVersionOnly(generator::Package(*q))]);

                check_candidates(yield, step, ids);
            }
        }
    }
    else
    {
        step("Searching matches");

        std::shared_ptr<Generator> match_generator;

        for (args::StringSetArg::ConstIterator k(search_options.a_matching.begin_args()),
                k_end(search_options.a_matching.end_args()) ;
                k != k_end ; ++k)
        {
            generator::Matches m(parse_user_package_dep_spec(*k, env.get(), UserPackageDepSpecOptions() + updso_allow_wildcards), MatchPackageOptions());

            if (match_generator)
                match_generator = make_shared_ptr(new generator::Union(*match_generator, m));
            else
                match_generator = make_shared_copy(m);
        }

        if (search_options.a_all_versions.specified())
        {
            const std::shared_ptr<const PackageIDSequence> ids((*env)[selection::AllVersionsUnsorted(
                        *match_generator)]);
            check_candidates(yield, step, ids);
        }
        else
        {
            const std::shared_ptr<const PackageIDSequence> ids((*env)[selection::BestVersionOnly(
                        *match_generator)]);
            check_candidates(yield, step, ids);
        }
    }
}

std::shared_ptr<args::ArgsHandler>
FindCandidatesCommand::make_doc_cmdline()
{
    return make_shared_ptr(new FindCandidatesCommandLine);
}

