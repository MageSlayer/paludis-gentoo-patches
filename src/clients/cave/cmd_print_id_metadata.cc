/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#include "cmd_print_id_metadata.hh"
#include "format_plain_metadata_key.hh"
#include "exceptions.hh"
#include <paludis/args/args.hh>
#include <paludis/args/do_help.hh>
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/util/set.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/options.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/environment.hh>
#include <paludis/metadata_key.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/filter.hh>
#include <paludis/generator.hh>
#include <paludis/selection.hh>
#include <paludis/package_id.hh>
#include <iostream>
#include <algorithm>

#include "command_command_line.hh"

using namespace paludis;
using namespace cave;
using std::cout;
using std::endl;

namespace
{
    struct PrintIDMetadataCommandLine :
        CaveCommandCommandLine
    {
        virtual std::string app_name() const
        {
            return "cave print-id-metadata";
        }

        virtual std::string app_synopsis() const
        {
            return "Prints ID metadata.";
        }

        virtual std::string app_description() const
        {
            return "Prints ID metadata. No formatting is used, making the output suitable for "
                "parsing by scripts.";
        }

        args::ArgsGroup g_spec_options;
        args::SwitchArg a_all;
        args::SwitchArg a_best;

        args::ArgsGroup g_filters;
        args::StringSetArg a_raw_name;
        args::StringSetArg a_human_name;

        args::ArgsGroup g_display_options;
        args::StringArg a_format;

        PrintIDMetadataCommandLine() :
            g_spec_options(main_options_section(), "Spec Options", "Alter how the supplied spec is used."),
            a_all(&g_spec_options, "all", 'a', "If the spec matches multiple IDs, display all matches.", true),
            a_best(&g_spec_options, "best", 'b', "If the spec matches multiple IDs, select the best ID rather than giving an error.", true),
            g_filters(main_options_section(), "Filters", "Filter the output. Each filter may be specified more than once."),
            a_raw_name(&g_filters, "raw-name", '\0', "Show only keys with this raw name. If specified more than once, "
                    "any name match is accepted."),
            a_human_name(&g_filters, "human-name", '\0', "Show only keys with this human name. If specified more than once, "
                    "any name match is accepted."),
            g_display_options(main_options_section(), "Display Options", "Controls the output format."),
            a_format(&g_display_options, "format", '\0', "Select the output format. Special tokens recognised are "
                    "%r for raw name, %h for human name, %v for value, %i for one space per subkey level, "
                    "\\n for newline, \\t for tab. Default is '%i%i%r=%v\\n'.")
        {
            a_format.set_argument("%i%i%r=%v\\n");
            add_usage_line("[ --raw-name key ] [ --human-name key ] [ --format format ] spec");

            add_example(
                "cave print-id-metadata --format \"%r\\n\" sys-apps/paludis::installed",
                "Print the raw names for all keys a given ID has.");
        }
    };

    void do_one_key(
            const std::shared_ptr<const MetadataKey> & k,
            const PrintIDMetadataCommandLine & cmdline,
            const std::string & name_prefix
            )
    {
        do
        {
            if (cmdline.a_raw_name.specified())
                if (cmdline.a_raw_name.end_args() == std::find(cmdline.a_raw_name.begin_args(), cmdline.a_raw_name.end_args(),
                            k->raw_name()))
                    continue;

            if (cmdline.a_human_name.specified())
                if (cmdline.a_human_name.end_args() == std::find(cmdline.a_human_name.begin_args(), cmdline.a_human_name.end_args(),
                            k->human_name()))
                    continue;

            cout << format_plain_metadata_key(k, name_prefix, cmdline.a_format.argument());
        } while (false);

        const MetadataSectionKey * section(simple_visitor_cast<const MetadataSectionKey>(*k));
        if (section)
        {
            for (MetadataSectionKey::MetadataConstIterator s(section->begin_metadata()), s_end(section->end_metadata()) ;
                    s != s_end ; ++s)
                do_one_key(*s, cmdline, name_prefix + " ");
        }
    }
}

int
PrintIDMetadataCommand::run(
        const std::shared_ptr<Environment> & env,
        const std::shared_ptr<const Sequence<std::string > > & args
        )
{
    PrintIDMetadataCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_PRINT_ID_METADATA_OPTIONS", "CAVE_PRINT_ID_METADATA_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    if (1 != std::distance(cmdline.begin_parameters(), cmdline.end_parameters()))
        throw args::DoHelp("print-id-metadata takes exactly one parameter");

    PackageDepSpec spec(parse_user_package_dep_spec(*cmdline.begin_parameters(), env.get(), { updso_allow_wildcards }));

    std::shared_ptr<const PackageIDSequence> entries(
            (*env)[selection::AllVersionsSorted(generator::Matches(spec, make_null_shared_ptr(), { }))]);

    if (entries->empty())
        throw NothingMatching(spec);

    if ((! cmdline.a_best.specified()) && (! cmdline.a_all.specified())
            && (next(entries->begin()) != entries->end()))
        throw BeMoreSpecific(spec, entries);

    for (auto i(cmdline.a_best.specified() ? entries->last() : entries->begin()), i_end(entries->end()) ;
            i != i_end ; ++i)
        for (PackageID::MetadataConstIterator m((*i)->begin_metadata()), m_end((*i)->end_metadata()) ;
                m != m_end ; ++m)
            do_one_key(*m, cmdline, "");

    return EXIT_SUCCESS;
}

std::shared_ptr<args::ArgsHandler>
PrintIDMetadataCommand::make_doc_cmdline()
{
    return std::make_shared<PrintIDMetadataCommandLine>();
}

