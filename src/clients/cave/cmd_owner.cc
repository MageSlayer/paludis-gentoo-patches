/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 Alexander Færøy
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

#include "cmd_owner.hh"
#include "command_command_line.hh"
#include "owner_common.hh"
#include "format_general.hh"
#include "formats.hh"

#include <paludis/action.hh>
#include <paludis/args/args.hh>
#include <paludis/args/do_help.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/stringify.hh>

#include <iostream>
#include <cstdlib>
#include <functional>

using namespace paludis;
using namespace cave;
using std::cout;
using std::endl;

namespace
{
    struct OwnerCommandLine :
        CaveCommandCommandLine
    {
        virtual std::string app_name() const
        {
            return "cave owner";
        }

        virtual std::string app_synopsis() const
        {
            return "Shows package IDs owning a given file.";
        }

        virtual std::string app_description() const
        {
            return "Shows package IDs owning a given file.";
        }

        args::ArgsGroup g_owner_options;
        args::EnumArg a_match;

        OwnerCommandLine() :
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

    void format_id(const std::shared_ptr<const PackageID> & id)
    {
        cout << format_general_s(f::owner_id(), stringify(*id));
    }
}

int
OwnerCommand::run(
        const std::shared_ptr<Environment> & env,
        const std::shared_ptr<const Sequence<std::string > > & args
        )
{
    OwnerCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_OWNER_OPTIONS", "CAVE_OWNER_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    if (std::distance(cmdline.begin_parameters(), cmdline.end_parameters()) != 1)
        throw args::DoHelp("owner takes exactly one parameter");

    return owner_common(env, cmdline.a_match.argument(), *cmdline.begin_parameters(), &format_id);
}

std::shared_ptr<args::ArgsHandler>
OwnerCommand::make_doc_cmdline()
{
    return std::make_shared<OwnerCommandLine>();
}

