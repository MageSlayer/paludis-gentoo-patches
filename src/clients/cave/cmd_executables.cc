/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009 Alexander Færøy
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

#include "cmd_executables.hh"
#include "command_command_line.hh"
#include "executables_common.hh"
#include "formats.hh"
#include "format_general.hh"

#include <paludis/args/args.hh>
#include <paludis/args/do_help.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/system.hh>
#include <paludis/util/tokeniser.hh>

#include <iostream>
#include <algorithm>
#include <set>
#include <cstdlib>
#include <tr1/memory>

using namespace paludis;
using namespace cave;
using std::cout;
using std::endl;

namespace
{
    struct ExecutablesCommandLine :
        CaveCommandCommandLine
    {
        virtual std::string app_name() const
        {
            return "cave executables";
        }

        virtual std::string app_synopsis() const
        {
            return "Display executables belonging to an ID.";
        }

        virtual std::string app_description() const
        {
            return "Display executables belonging to an ID.";
        }

        ExecutablesCommandLine()
        {
            add_usage_line("spec");
        }
    };

    void format_fsentry(const FSEntry & f)
    {
        cout << format_general_s(f::executables_file(), stringify(f));
    }
}

int
ExecutablesCommand::run(
        const std::tr1::shared_ptr<Environment> & env,
        const std::tr1::shared_ptr<const Sequence<std::string > > & args
        )
{
    ExecutablesCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_EXECUTABLES_OPTIONS", "CAVE_EXECUTABLES_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    if (1 != std::distance(cmdline.begin_parameters(), cmdline.end_parameters()))
        throw args::DoHelp("executables takes exactly one parameter");

    return executables_common(env, *cmdline.begin_parameters(), &format_fsentry);
}

std::tr1::shared_ptr<args::ArgsHandler>
ExecutablesCommand::make_doc_cmdline()
{
    return make_shared_ptr(new ExecutablesCommandLine);
}

