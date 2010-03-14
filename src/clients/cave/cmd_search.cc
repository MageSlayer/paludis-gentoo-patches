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

#include "cmd_search.hh"
#include "cmd_search_cmdline.hh"
#include "cmd_find_candidates.hh"
#include "cmd_match.hh"
#include "cmd_show.hh"
#include <paludis/args/args.hh>
#include <paludis/args/do_help.hh>
#include <paludis/name.hh>
#include <paludis/environment.hh>
#include <paludis/package_database.hh>
#include <paludis/repository.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/filter.hh>
#include <paludis/generator.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/selection.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/util/set.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/action.hh>
#include <paludis/mask.hh>
#include <paludis/choice.hh>
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
    struct SearchCommandLine :
        CaveCommandCommandLine
    {
        virtual std::string app_name() const
        {
            return "cave search";
        }

        virtual std::string app_synopsis() const
        {
            return "Search for packages with particular characteristics.";
        }

        virtual std::string app_description() const
        {
            return "Searches for packages with particular characteristics.";
        }

        SearchCommandLineCandidateOptions search_options;
        SearchCommandLineMatchOptions match_options;

        SearchCommandLine() :
            search_options(this),
            match_options(this)
        {
            add_usage_line("[ --name | --description | --key HOMEPAGE ] pattern ...");
        }
    };

    void found_match(
            const std::tr1::shared_ptr<Environment> & env,
            const std::tr1::shared_ptr<Set<QualifiedPackageName> > & result,
            const PackageDepSpec & spec)
    {
        const std::tr1::shared_ptr<const PackageID> id(*((*env)[selection::RequireExactlyOne(
                        generator::Matches(spec, MatchPackageOptions()))])->begin());
        result->insert(id->name());
    }

    void found_candidate(
            const std::tr1::shared_ptr<Environment> & env,
            MatchCommand & match_command,
            const SearchCommandLineMatchOptions & match_options,
            const PackageDepSpec & spec,
            const std::tr1::shared_ptr<const Set<std::string> > & patterns,
            const std::tr1::function<void (const PackageDepSpec &)> & success
            )
    {
        if (match_command.run_hosted(env, match_options, patterns, spec))
            success(spec);
    }
}

bool
SearchCommand::important() const
{
    return true;
}

int
SearchCommand::run(
        const std::tr1::shared_ptr<Environment> & env,
        const std::tr1::shared_ptr<const Sequence<std::string > > & args
        )
{
    SearchCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_SEARCH_OPTIONS", "CAVE_SEARCH_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    if (cmdline.begin_parameters() == cmdline.end_parameters())
        throw args::DoHelp("search requires at least one parameter");

    const std::tr1::shared_ptr<Set<std::string> > patterns(new Set<std::string>);
    std::copy(cmdline.begin_parameters(), cmdline.end_parameters(), patterns->inserter());

    FindCandidatesCommand find_candidates_command;
    MatchCommand match_command;

    std::tr1::shared_ptr<Set<QualifiedPackageName> > matches(new Set<QualifiedPackageName>);
    find_candidates_command.run_hosted(env, cmdline.search_options, cmdline.match_options,
            patterns, std::tr1::bind(
                &found_candidate, env, std::tr1::ref(match_command), std::tr1::cref(cmdline.match_options),
                std::tr1::placeholders::_1, patterns, std::tr1::function<void (const PackageDepSpec &)>(std::tr1::bind(
                        &found_match, env, std::tr1::ref(matches), std::tr1::placeholders::_1
                        ))));

    if (matches->empty())
        return EXIT_FAILURE;

    ShowCommand show_command;
    const std::tr1::shared_ptr<Sequence<std::string> > show_args(new Sequence<std::string>);
    for (Set<QualifiedPackageName>::ConstIterator p(matches->begin()), p_end(matches->end()) ;
            p != p_end ; ++p)
        show_args->push_back(stringify(*p));

    return show_command.run(env, show_args);
}

std::tr1::shared_ptr<args::ArgsHandler>
SearchCommand::make_doc_cmdline()
{
    return make_shared_ptr(new SearchCommandLine);
}

