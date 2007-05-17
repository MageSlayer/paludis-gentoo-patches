/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 David Leverton <u01drl3@abdn.ac.uk>
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

#include <paludis/environment.hh>
#include <paludis/hook.hh>
#include <paludis/util/graph.hh>
#include <iostream>
#include <fstream>

using namespace paludis;

namespace
{
    HookResult
    so_hook_run(const Environment *, const Hook &)
    {
        return HookResult(6, "");
    }

    HookResult
    so_hook_output_run(const Environment *, const Hook &)
    {
        return HookResult(0, "foo");
    }

    HookResult
    ordering_run(const Environment *, const Hook &)
    {
        std::ofstream f("hooker_TEST_dir/ordering.out", std::ios_base::app);
        f << "sohook" << std::endl;
        return HookResult(0, "");
    }

    void
    ordering_add_dependencies(const Environment *, const Hook &,
                              DirectedGraph<std::string, int> & graph)
    {
        graph.add_edge("k", "libpaludissohooks_TEST", 0);
    }
}

extern "C" HookResult PALUDIS_VISIBLE
paludis_hook_run(const Environment * env, const Hook & hook)
{
    if ("so_hook" == hook.name())
        return so_hook_run(env, hook);
    if ("so_hook_output" == hook.name())
        return so_hook_output_run(env, hook);
    if ("ordering" == hook.name())
        return ordering_run(env, hook);

    return HookResult(0, "");
}

extern "C" void PALUDIS_VISIBLE
paludis_hook_add_dependencies(const Environment * env, const Hook & hook,
                              DirectedGraph<std::string, int> & graph)
{
    if ("ordering" == hook.name())
        ordering_add_dependencies(env, hook, graph);
}

