/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 Alexander Færøy
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

#include "cmd_print_owners.hh"
#include "command_command_line.hh"
#include "format_string.hh"
#include "owner_common.hh"
#include <paludis/args/args.hh>
#include <paludis/args/do_help.hh>
#include <paludis/dep_spec.hh>
#include <paludis/filter.hh>
#include <paludis/metadata_key.hh>
#include <paludis/package_id.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/util/map.hh>
#include <paludis/util/stringify.hh>
#include <paludis/version_spec.hh>
#include <iostream>
#include <cstdlib>

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
        args::EnumArg a_type;
        args::StringSetArg a_matching;

        args::ArgsGroup g_display_options;
        args::StringArg a_format;

        PrintOwnersCommandLine() :
            g_owner_options(main_options_section(), "Owner options", "Alter how the search is performed."),
            a_type(&g_owner_options, "type", 't', "Which type of match algorithm to use",
                    args::EnumArg::EnumArgOptions
                    ("auto",          "If pattern starts with a /, full; if it contains a /, partial; otherwise, basename")
                    ("basename",      "Basename match")
                    ("full",          "Full match")
                    ("partial",       "Partial match"),
                    "auto"),
            a_matching(&g_owner_options, "matching", 'm', "Show only IDs matching this spec. If specified multiple "
                    "times, only IDs matching every spec are selected.",
                    args::StringSetArg::StringSetArgOptions()),
            g_display_options(main_options_section(), "Display Options", "Controls the output format."),
            a_format(&g_display_options, "format", 'f', "Select the output format. Special tokens recognised are "
                    "%c for category, %p for package, %v for version, %s for slot, %: for ':' if we have a slot and "
                    "empty otherwise, %r for repository, %F for the canonical full form, %V for the canonical full "
                    "version, %W for the canonical full unversioned form, %N for the canonical full unnamed form, "
                    "\\n for newline, \\t for tab. Default is '%F\\n'.")
        {
            add_usage_line("[ --type algorithm ] [ --matching spec ] pattern");
            a_format.set_argument("%F\\n");
        }
    };

    std::string format_id(
            const std::string & format,
            const std::shared_ptr<const PackageID> & i)
    {
        std::shared_ptr<Map<char, std::string> > m(std::make_shared<Map<char, std::string>>());
        m->insert('c', stringify(i->name().category()));
        m->insert('p', stringify(i->name().package()));
        m->insert('v', stringify(i->version()));
        m->insert('s', i->slot_key() ? stringify(i->slot_key()->parse_value()) : "");
        m->insert(':', i->slot_key() ? ":" : "");
        m->insert('r', stringify(i->repository_name()));
        m->insert('F', i->canonical_form(idcf_full));
        m->insert('V', i->canonical_form(idcf_version));
        m->insert('W', i->canonical_form(idcf_no_version));
        m->insert('N', i->canonical_form(idcf_no_name));
        return format_string(format, m);
    }

    void print_package_id(
            const std::string & format,
            const std::shared_ptr<const PackageID> & id)
    {
        cout << format_id(format, id);
    }
}

int
PrintOwnersCommand::run(
        const std::shared_ptr<Environment> & env,
        const std::shared_ptr<const Sequence<std::string > > & args
        )
{
    PrintOwnersCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_PRINT_OWNERS_OPTIONS", "CAVE_PRINT_OWNERS_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    if (std::distance(cmdline.begin_parameters(), cmdline.end_parameters()) != 1)
        throw args::DoHelp("print-owners takes exactly one parameter");

    Filter matches((filter::All()));
    if (cmdline.a_matching.specified())
    {
        for (args::StringSetArg::ConstIterator m(cmdline.a_matching.begin_args()),
                m_end(cmdline.a_matching.end_args()) ;
                m != m_end ; ++m)
        {
            PackageDepSpec s(parse_user_package_dep_spec(*m, env.get(), { updso_allow_wildcards }));
            matches = filter::And(matches, filter::Matches(s, make_null_shared_ptr(), { }));
        }
    }

    return owner_common(env, cmdline.a_type.argument(), matches,
            *cmdline.begin_parameters(), false,
            std::bind(&print_package_id, cmdline.a_format.argument(), std::placeholders::_1));
}

std::shared_ptr<args::ArgsHandler>
PrintOwnersCommand::make_doc_cmdline()
{
    return std::make_shared<PrintOwnersCommandLine>();
}

CommandImportance
PrintOwnersCommand::importance() const
{
    return ci_scripting;
}

