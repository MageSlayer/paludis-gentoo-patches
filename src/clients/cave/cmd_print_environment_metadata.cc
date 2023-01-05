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

#include "cmd_print_environment_metadata.hh"
#include "format_plain_metadata_key.hh"
#include "exceptions.hh"
#include <paludis/args/args.hh>
#include <paludis/args/do_help.hh>
#include <paludis/util/visitor_cast.hh>
#include <paludis/util/set.hh>
#include <paludis/environment.hh>
#include <paludis/metadata_key.hh>
#include <iostream>
#include <algorithm>

#include "command_command_line.hh"

using namespace paludis;
using namespace cave;
using std::cout;

namespace
{
    struct PrintEnvironmentMetadataCommandLine :
        CaveCommandCommandLine
    {
        std::string app_name() const override
        {
            return "cave print-environment-metadata";
        }

        std::string app_synopsis() const override
        {
            return "Prints environment metadata.";
        }

        std::string app_description() const override
        {
            return "Prints environment metadata. No formatting is used, making the output suitable for "
                "parsing by scripts.";
        }

        args::ArgsGroup g_filters;
        args::StringSetArg a_raw_name;
        args::StringSetArg a_human_name;

        args::ArgsGroup g_display_options;
        args::StringArg a_format;

        PrintEnvironmentMetadataCommandLine() :
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
            add_usage_line("[ --raw-name key ] [ --human-name key ] [ --format format ]");
        }
    };

    void do_one_key(
            const std::shared_ptr<const MetadataKey> & k,
            const PrintEnvironmentMetadataCommandLine & cmdline,
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

        const MetadataSectionKey * section(visitor_cast<const MetadataSectionKey>(*k));
        if (section)
        {
            for (MetadataSectionKey::MetadataConstIterator s(section->begin_metadata()), s_end(section->end_metadata()) ;
                    s != s_end ; ++s)
                do_one_key(*s, cmdline, name_prefix + " ");
        }
    }
}

int
PrintEnvironmentMetadataCommand::run(
        const std::shared_ptr<Environment> & env,
        const std::shared_ptr<const Sequence<std::string > > & args
        )
{
    PrintEnvironmentMetadataCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_PRINT_ENVIRONMENT_METADATA_OPTIONS", "CAVE_PRINT_ENVIRONMENT_METADATA_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    if (cmdline.begin_parameters() != cmdline.end_parameters())
        throw args::DoHelp("print-environment-metadata takes no parameters");

    for (Environment::MetadataConstIterator m(env->begin_metadata()), m_end(env->end_metadata()) ;
            m != m_end ; ++m)
        do_one_key(*m, cmdline, "");

    return EXIT_SUCCESS;
}

std::shared_ptr<args::ArgsHandler>
PrintEnvironmentMetadataCommand::make_doc_cmdline()
{
    return std::make_shared<PrintEnvironmentMetadataCommandLine>();
}

CommandImportance
PrintEnvironmentMetadataCommand::importance() const
{
    return ci_scripting;
}

