/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
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

#include "script_command.hh"
#include "exceptions.hh"
#include <paludis/args/args.hh>
#include <paludis/args/do_help.hh>
#include <paludis/args/escape.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/system.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/stringify.hh>
#include <iostream>
#include <cstdlib>

#include "command_command_line.hh"

using namespace paludis;
using namespace cave;
using std::cout;
using std::endl;

namespace paludis
{
    template <>
    struct Implementation<ScriptCommand>
    {
        const FSEntry executable;

        Implementation(const FSEntry & e) :
            executable(e)
        {
        }
    };
}

ScriptCommand::ScriptCommand(const std::string &, const FSEntry & e) :
    PrivateImplementationPattern<ScriptCommand>(e)
{
}

ScriptCommand::~ScriptCommand()
{
}

int
ScriptCommand::run(
        const std::shared_ptr<Environment> &,
        const std::shared_ptr<const Sequence<std::string > > & args
        )
{
    std::string arg_str;
    for (Sequence<std::string>::ConstIterator n(args->begin()), n_end(args->end()) ;
            n != n_end ; ++n)
        arg_str = " " + args::escape(*n);

    paludis::Command cmd(stringify(_imp->executable) + arg_str);
    become_command(cmd);

    throw InternalError(PALUDIS_HERE, "become_command failed");
}

std::shared_ptr<args::ArgsHandler>
ScriptCommand::make_doc_cmdline()
{
    throw InternalError(PALUDIS_HERE, "no script cmdline");
}

