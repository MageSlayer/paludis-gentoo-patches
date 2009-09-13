/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 Alexander Færøy
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

#include "cmd_print_owners.hh"
#include "command_command_line.hh"

#include <paludis/action.hh>
#include <paludis/args/args.hh>
#include <paludis/args/do_help.hh>
#include <paludis/contents.hh>
#include <paludis/environment.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/generator.hh>
#include <paludis/metadata_key.hh>
#include <paludis/name.hh>
#include <paludis/package_database.hh>
#include <paludis/package_id.hh>
#include <paludis/repository.hh>
#include <paludis/selection.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/wrapped_forward_iterator.hh>

#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <tr1/functional>

using namespace paludis;
using namespace cave;
using std::cout;
using std::endl;

namespace
{
    struct PrintOwnersCommandLine :
        CaveCommandCommandLine
    {
        virtual std::string app_name() const
        {
            return "cave print-owners";
        }

        virtual std::string app_synopsis() const
        {
            return "Prints a list of package IDs owning a given file.";
        }

        virtual std::string app_description() const
        {
            return "Prints a list of package IDs owning a given file. No formatting is used, making the output suitable for parsing by scripts.";
        }

        args::ArgsGroup g_owner_options;
        args::EnumArg a_match;

        PrintOwnersCommandLine() :
            g_owner_options(main_options_section(), "Owner options", "Alter how the search is performed."),
            a_match(&g_owner_options, "match", 'm', "Which match algorithm to use",
                    args::EnumArg::EnumArgOptions
                    ("auto",          "If pattern starts with a /, full; if it contains a /, partial; otherwise, basename")
                    ("basename",      "Basename match")
                    ("full",          "Full match")
                    ("partial",       "Partial match"),
                    "auto")
        {
            add_usage_line("[ --match algorithm ] pattern");
        }
    };

    bool handle_full(const std::string & q, const std::tr1::shared_ptr<const ContentsEntry> & e)
    {
        return q == stringify(e->location_key()->value());
    }

    bool handle_basename(const std::string & q, const std::tr1::shared_ptr<const ContentsEntry> & e)
    {
        return q == e->location_key()->value().basename();
    }

    bool handle_partial(const std::string & q, const std::tr1::shared_ptr<const ContentsEntry> & e)
    {
        return std::string::npos != stringify(e->location_key()->value()).find(q);
    }
}

int
PrintOwnersCommand::run(
        const std::tr1::shared_ptr<Environment> & env,
        const std::tr1::shared_ptr<const Sequence<std::string > > & args
        )
{
    using namespace std::tr1::placeholders;

    PrintOwnersCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_PRINT_OWNERS_OPTIONS", "CAVE_PRINT_OWNERS_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    if (std::distance(cmdline.begin_parameters(), cmdline.end_parameters()) != 1)
        throw args::DoHelp("print-owners takes exactly one parameter");

    bool found(false);
    const std::string match(cmdline.a_match.argument());
    const std::string query(*cmdline.begin_parameters());
    std::tr1::function<bool (const std::string &, const std::tr1::shared_ptr<const ContentsEntry> &)> handler;

    if ("full" == match)
        handler = handle_full;
    else if ("basename" == match)
        handler = handle_basename;
    else if ("partial" == match)
        handler = handle_partial;
    else
    {
        if (! query.empty() && '/' == query.at(0))
            handler = handle_full;
        else if (std::string::npos != query.find("/"))
            handler = handle_partial;
        else
            handler = handle_basename;
    }

    std::tr1::shared_ptr<const PackageIDSequence> ids((*env)[selection::AllVersionsSorted(generator::All() |
                filter::InstalledAtRoot(env->root()))]);

    for (PackageIDSequence::ConstIterator p(ids->begin()), p_end(ids->end()); p != p_end; ++p)
    {
        if (! (*p)->contents_key())
            continue;

        std::tr1::shared_ptr<const Contents> contents((*p)->contents_key()->value());
        if (contents->end() != std::find_if(contents->begin(), contents->end(), std::tr1::bind(handler, query, _1)))
        {
            cout << **p << endl;
            found = true;
        }
    }

    return found ? EXIT_SUCCESS : EXIT_FAILURE;
}

std::tr1::shared_ptr<args::ArgsHandler>
PrintOwnersCommand::make_doc_cmdline()
{
    return make_shared_ptr(new PrintOwnersCommandLine);
}
