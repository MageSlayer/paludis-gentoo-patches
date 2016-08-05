/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009 Alexander Færøy
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

#include "cmd_executables.hh"
#include "command_command_line.hh"
#include "executables_common.hh"
#include "colours.hh"
#include "format_user_config.hh"

#include <paludis/args/args.hh>
#include <paludis/args/do_help.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/system.hh>
#include <paludis/util/tokeniser.hh>

#include <iostream>
#include <algorithm>
#include <set>
#include <cstdlib>
#include <memory>

using namespace paludis;
using namespace cave;
using std::cout;
using std::endl;

namespace
{
#include "cmd_executables-fmt.hh"

    struct ExecutablesCommandLine :
        CaveCommandCommandLine
    {
        std::string app_name() const override
        {
            return "cave executables";
        }

        std::string app_synopsis() const override
        {
            return "Display executables belonging to an ID.";
        }

        std::string app_description() const override
        {
            return "Display executables belonging to an ID.";
        }

        ExecutablesCommandLine()
        {
            add_usage_line("spec");
        }
    };

    void format_fsentry(const FSPath & f)
    {
        cout << fuc(fs_file(), fv<'s'>(stringify(f)));
    }
}

int
ExecutablesCommand::run(
        const std::shared_ptr<Environment> & env,
        const std::shared_ptr<const Sequence<std::string > > & args
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

    return executables_common(env, *cmdline.begin_parameters(), &format_fsentry, true, false);
}

std::shared_ptr<args::ArgsHandler>
ExecutablesCommand::make_doc_cmdline()
{
    return std::make_shared<ExecutablesCommandLine>();
}

CommandImportance
ExecutablesCommand::importance() const
{
    return ci_supplemental;
}

