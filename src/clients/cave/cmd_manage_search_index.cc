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

#include "cmd_manage_search_index.hh"
#include "search_extras.hh"
#include "search_extras_handle.hh"

#include <paludis/args/args.hh>
#include <paludis/args/do_help.hh>

#include <paludis/name.hh>
#include <paludis/environment.hh>
#include <paludis/package_database.hh>
#include <paludis/repository.hh>
#include <paludis/generator.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/filter.hh>
#include <paludis/filter_handler.hh>
#include <paludis/selection.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/package_id.hh>
#include <paludis/mask.hh>
#include <paludis/metadata_key.hh>
#include <paludis/choice.hh>
#include <paludis/about.hh>
#include <paludis/notifier_callback.hh>

#include <paludis/util/set.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/visitor_cast.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/accept_visitor.hh>
#include <paludis/util/mutex.hh>

#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <list>
#include <map>

#include "config.h"
#include "command_command_line.hh"

using namespace paludis;
using namespace cave;
using std::cout;
using std::endl;

namespace
{
    struct ManageStep
    {
        std::string stage;
    };

    struct DisplayCallback
    {
        mutable Mutex mutex;
        mutable std::map<std::string, int> metadata;
        mutable int steps;
        int total;
        mutable std::string stage;
        mutable unsigned width;

        bool output;

        DisplayCallback() :
            steps(0),
            total(-1),
            stage("Querying"),
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
            if (-1 != total)
                s.append("/" + stringify(total));

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

        void operator() (const ManageStep & s) const
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

    struct ManageSearchIndexCommandLine :
        CaveCommandCommandLine
    {
        args::ArgsGroup g_actions;
        args::SwitchArg a_create;

        virtual std::string app_name() const
        {
            return "cave manage-search-index";
        }

        virtual std::string app_synopsis() const
        {
            return "Manages a search index for use by cave search.";
        }

        virtual std::string app_description() const
        {
            return "Manages a search index for use by cave search. A search index is only valid until "
                "a package is installed or uninstalled, or a sync is performed, or configuration is "
                "changed.";
        }

        ManageSearchIndexCommandLine() :
            g_actions(main_options_section(), "Actions", "Specify which action to perform. Exactly one action must be specified."),
            a_create(&g_actions, "create", 'c', "Create a new search index. The existing search index is removed if "
                    "it already exists", true)
        {
            add_usage_line("--create ~/cave-search-index");
        }
    };
}

int
ManageSearchIndexCommand::run(
        const std::shared_ptr<Environment> & env,
        const std::shared_ptr<const Sequence<std::string > > & args
        )
{
    ManageSearchIndexCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_MANAGE_SEARCH_INDEX_OPTIONS", "CAVE_MANAGE_SEARCH_INDEX_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    if (capped_distance(cmdline.begin_parameters(), cmdline.end_parameters(), 2) != 1)
        throw args::DoHelp("manage-search-index requires exactly one parameter");

    if (! cmdline.a_create.specified())
        throw args::DoHelp("exactly one action must be specified");

    FSPath index_file(*cmdline.begin_parameters());
    index_file.unlink();

    {
        DisplayCallback display_callback;
        ScopedNotifierCallback display_callback_holder(env.get(),
                NotifierCallbackFunction(std::cref(display_callback)));

        display_callback(ManageStep{"Creating DB"});
        CaveSearchExtrasDB * db(SearchExtrasHandle::get_instance()->create_db_function(stringify(index_file).c_str()));

        display_callback(ManageStep{"Querying"});
        auto ids((*env)[selection::AllVersionsSorted(generator::All())]);
        display_callback.total = display_callback.steps + std::distance(ids->begin(), ids->end()) + 1;

        SearchExtrasHandle::get_instance()->starting_adds_function(db);

        bool is_best(false), had_best_visible(false);
        std::string old_name;
        for (auto i(ids->rbegin()), i_end(ids->rend()) ;
                i != i_end ; ++i)
        {
            display_callback(ManageStep{"Writing"});

            std::string name(stringify((*i)->name())), short_desc, long_desc;
            if ((*i)->short_description_key())
                short_desc = (*i)->short_description_key()->value();
            if ((*i)->long_description_key())
                long_desc = (*i)->long_description_key()->value();

            bool is_visible(! (*i)->masked());

            if (name != old_name)
            {
                is_best = true;
                had_best_visible = false;
                old_name = name;
            }

            bool is_best_visible(is_visible && ! had_best_visible);
            if (is_best_visible)
                had_best_visible = true;

            SearchExtrasHandle::get_instance()->add_candidate_function(db, stringify((*i)->uniquely_identifying_spec()),
                    is_visible, is_best, is_best_visible, name, short_desc, long_desc);

            is_best = false;
        }

        display_callback(ManageStep{"Finalising"});
        SearchExtrasHandle::get_instance()->done_adds_function(db);
        SearchExtrasHandle::get_instance()->cleanup_db_function(db);
    }

    return EXIT_SUCCESS;
}

std::shared_ptr<args::ArgsHandler>
ManageSearchIndexCommand::make_doc_cmdline()
{
    return std::make_shared<ManageSearchIndexCommandLine>();
}

