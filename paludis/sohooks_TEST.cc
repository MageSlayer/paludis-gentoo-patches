/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 David Leverton
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
#include <paludis/util/graph-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/safe_ofstream.hh>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

using namespace paludis;

namespace
{
    HookResult
    so_hook_run(const Environment *, const Hook &)
    {
        return make_named_values<HookResult>(n::max_exit_status() = 6, n::output() = "");
    }

    HookResult
    so_hook_output_run(const Environment *, const Hook &)
    {
        return make_named_values<HookResult>(n::max_exit_status() = 0, n::output() = "foo");
    }

    HookResult
    ordering_run(const Environment *, const Hook &)
    {
        SafeOFStream f(FSEntry("hooker_TEST_dir/ordering.out"), O_CREAT | O_WRONLY | O_APPEND);
        f << "sohook" << std::endl;
        return make_named_values<HookResult>(n::max_exit_status() = 0, n::output() = "");
    }

    void
    ordering_add_dependencies(const Environment *, const Hook &,
                              DirectedGraph<std::string, int> & graph)
    {
        graph.add_edge("k", "libpaludissohooks_TEST", 0);
    }
}

HookResult
paludis_hook_run(const Environment * env, const Hook & hook)
{
    if ("so_hook" == hook.name())
        return so_hook_run(env, hook);
    if ("so_hook_output" == hook.name())
        return so_hook_output_run(env, hook);
    if ("ordering" == hook.name())
        return ordering_run(env, hook);

    return make_named_values<HookResult>(n::max_exit_status() = 0, n::output() = "");
}

void
paludis_hook_add_dependencies(const Environment * env, const Hook & hook,
                              DirectedGraph<std::string, int> & graph)
{
    if ("ordering" == hook.name())
        ordering_add_dependencies(env, hook, graph);
}

