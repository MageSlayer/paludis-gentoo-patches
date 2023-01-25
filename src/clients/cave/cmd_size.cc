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

#include "cmd_size.hh"
#include "command_command_line.hh"
#include "size_common.hh"

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

namespace
{
    struct SizeCommandLine :
        CaveCommandCommandLine
    {
        std::string app_name() const override
        {
            return "cave size";
        }

        std::string app_synopsis() const override
        {
            return "Shows the size of files installed by a package.";
        }

        std::string app_description() const override
        {
            return "Shows the size of files installed by a package.";
        }

        SizeCommandLine()
        {
            add_usage_line("spec");
        }
    };
}

int
SizeCommand::run(
        const std::shared_ptr<Environment> & env,
        const std::shared_ptr<const Sequence<std::string > > & args
        )
{
    SizeCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_SIZE_OPTIONS", "CAVE_SIZE_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    if (cmdline.parameters().size() != 1)
        throw args::DoHelp("size takes exactly one parameter");

    return size_common(env, true, *cmdline.begin_parameters(), true, false);
}

std::shared_ptr<args::ArgsHandler>
SizeCommand::make_doc_cmdline()
{
    return std::make_shared<SizeCommandLine>();
}

CommandImportance
SizeCommand::importance() const
{
    return ci_supplemental;
}

