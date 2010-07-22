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

#include "cmd_print_owners.hh"
#include "command_command_line.hh"
#include "owner_common.hh"
#include <paludis/args/args.hh>
#include <paludis/args/do_help.hh>
#include <paludis/util/make_shared_ptr.hh>
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

    void print_package_id(const std::shared_ptr<const PackageID> & id)
    {
        cout << *id << endl;
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

    return owner_common(env, cmdline.a_match.argument(), *cmdline.begin_parameters(), &print_package_id);
}

std::shared_ptr<args::ArgsHandler>
PrintOwnersCommand::make_doc_cmdline()
{
    return make_shared_ptr(new PrintOwnersCommandLine);
}

