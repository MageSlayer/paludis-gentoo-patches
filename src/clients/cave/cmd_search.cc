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
#include <paludis/util/mutex.hh>
#include <paludis/action.hh>
#include <paludis/mask.hh>
#include <paludis/choice.hh>
#include <paludis/notifier_callback.hh>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <set>
#include <map>

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
            add_note("'cave search' should only be used when a complex metadata search is required. To see "
                    "information about a known package, use 'cave show' instead.");
        }
    };

    struct SearchStep
    {
        std::string stage;

        SearchStep(const std::string & s) :
            stage(s)
        {
        }
    };

    void found_match(
            const std::shared_ptr<Environment> & env,
            const std::shared_ptr<Set<QualifiedPackageName> > & result,
            const PackageDepSpec & spec)
    {
        const std::shared_ptr<const PackageID> id(*((*env)[selection::RequireExactlyOne(
                        generator::Matches(spec, MatchPackageOptions()))])->begin());
        result->insert(id->name());
    }

    void found_candidate(
            const std::shared_ptr<Environment> & env,
            MatchCommand & match_command,
            const SearchCommandLineMatchOptions & match_options,
            const PackageDepSpec & spec,
            const std::shared_ptr<const Set<std::string> > & patterns,
            const std::function<void (const PackageDepSpec &)> & success
            )
    {
        if (match_command.run_hosted(env, match_options, patterns, spec))
            success(spec);
    }

    struct DisplayCallback
    {
        mutable Mutex mutex;
        mutable std::map<std::string, int> metadata;
        mutable int steps;
        mutable std::string stage;
        mutable unsigned width;

        bool output;

        DisplayCallback() :
            steps(0),
            stage("Searching"),
            width(stage.length() + 2),
            output(::isatty(1))
        {
            if (output)
                cout << stage << ": " << std::flush;
        }

        ~DisplayCallback()
        {
            if (output)
                cout << endl << endl;
        }

        void update() const
        {
            if (! output)
                return;

            std::string s(stage + ": ");
            s.append(stringify(steps));

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
                        ss.append(", ...");

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
                s.append(stringify(t) + " metadata (" + ss + ") ");
            }

            std::cout << std::string(width, '\010') << s;

            if (width > s.length())
                std::cout
                    << std::string(width - s.length(), ' ')
                    << std::string(width - s.length(), '\010');

            width = s.length();
            std::cout << std::flush;
        }

        void operator() (const NotifierCallbackEvent & event) const
        {
            event.accept(*this);
        }

        void operator() (const SearchStep & s) const
        {
            if (! output)
                return;

            Lock lock(mutex);
            ++steps;
            stage = s.stage;
            update();
        }

        void visit(const NotifierCallbackGeneratingMetadataEvent & e) const
        {
            if (! output)
                return;

            Lock lock(mutex);
            ++metadata.insert(std::make_pair(stringify(e.repository()), 0)).first->second;
            update();
        }

        void visit(const NotifierCallbackResolverStepEvent &) const
        {
        }

        void visit(const NotifierCallbackResolverStageEvent &) const
        {
        }

        void visit(const NotifierCallbackLinkageStepEvent &) const
        {
        }
    };

    void step(DisplayCallback & display_callback, const std::string & s)
    {
        display_callback(SearchStep(s));
    }
}

bool
SearchCommand::important() const
{
    return true;
}

int
SearchCommand::run(
        const std::shared_ptr<Environment> & env,
        const std::shared_ptr<const Sequence<std::string > > & args
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

    const std::shared_ptr<Sequence<std::string> > show_args(new Sequence<std::string>);

    {
        DisplayCallback display_callback;
        ScopedNotifierCallback display_callback_holder(env.get(),
                NotifierCallbackFunction(std::cref(display_callback)));

        const std::shared_ptr<Set<std::string> > patterns(new Set<std::string>);
        std::copy(cmdline.begin_parameters(), cmdline.end_parameters(), patterns->inserter());

        FindCandidatesCommand find_candidates_command;
        MatchCommand match_command;

        std::shared_ptr<Set<QualifiedPackageName> > matches(new Set<QualifiedPackageName>);
        find_candidates_command.run_hosted(env, cmdline.search_options, cmdline.match_options,
                patterns, std::bind(
                    &found_candidate, env, std::ref(match_command), std::cref(cmdline.match_options),
                    std::placeholders::_1, patterns, std::function<void (const PackageDepSpec &)>(std::bind(
                            &found_match, env, std::ref(matches), std::placeholders::_1
                            ))),
                std::bind(&step, std::ref(display_callback), std::placeholders::_1)
                );

        for (Set<QualifiedPackageName>::ConstIterator p(matches->begin()), p_end(matches->end()) ;
                p != p_end ; ++p)
            show_args->push_back(stringify(*p));
    }

    if (show_args->empty())
        return EXIT_FAILURE;

    if (! cmdline.search_options.a_all_versions.specified())
        show_args->push_back("--one-version");

    show_args->push_back("--significant-keys-only");
    for (args::StringSetArg::ConstIterator k(cmdline.match_options.a_key.begin_args()), k_end(cmdline.match_options.a_key.end_args()) ;
            k != k_end ; ++k)
    {
        show_args->push_back("--key");
        show_args->push_back(*k);
    }

    ShowCommand show_command;
    return show_command.run(env, show_args);
}

std::shared_ptr<args::ArgsHandler>
SearchCommand::make_doc_cmdline()
{
    return make_shared_ptr(new SearchCommandLine);
}

