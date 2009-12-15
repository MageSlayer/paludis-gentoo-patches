/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009 Ciaran McCreesh
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

#include "cmd_resolve.hh"
#include "cmd_resolve_cmdline.hh"
#include "resolve_common.hh"

#include <paludis/util/make_shared_ptr.hh>

#include <iostream>
#include <cstdlib>

using namespace paludis;
using namespace cave;

using std::cout;
using std::endl;

bool
ResolveCommand::important() const
{
    return true;
}

int
ResolveCommand::run(
        const std::tr1::shared_ptr<Environment> & env,
        const std::tr1::shared_ptr<const Sequence<std::string > > & args
        )
{
    ResolveCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_RESOLVE_OPTIONS", "CAVE_RESOLVE_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    cmdline.resolution_options.apply_shortcuts();
    cmdline.resolution_options.verify(env);

    std::tr1::shared_ptr<Sequence<std::string> > targets(new Sequence<std::string>);
    for (ResolveCommandLine::ParametersConstIterator p(cmdline.begin_parameters()), p_end(cmdline.end_parameters()) ;
            p != p_end ; ++p)
        targets->push_back(*p);

    return resolve_common(env, cmdline.resolution_options, cmdline.execution_options, cmdline.display_options,
            cmdline.program_options, make_null_shared_ptr(), targets);
}

std::tr1::shared_ptr<args::ArgsHandler>
ResolveCommand::make_doc_cmdline()
{
    return make_shared_ptr(new ResolveCommandLine);
}

