/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include "hooker.hh"
#include <paludis/environment.hh>
#include <paludis/hashed_containers.hh>
#include <paludis/util/collection_concrete.hh>
#include <paludis/util/log.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/system.hh>
#include <paludis/util/strip.hh>
#include <list>

using namespace paludis;

typedef MakeHashedMap<std::string, bool>::Type HookPresentCache;

namespace paludis
{
    template<>
    struct Implementation<Hooker>
    {
        const Environment * const env;
        mutable HookPresentCache hook_cache;
        std::list<std::pair<FSEntry, bool> > dirs;

        Implementation(const Environment * const e) :
            env(e)
        {
        }
    };
}

Hooker::Hooker(const Environment * const e) :
    PrivateImplementationPattern<Hooker>(new Implementation<Hooker>(e))
{
}

Hooker::~Hooker()
{
}

void
Hooker::add_dir(const FSEntry & dir, const bool v)
{
    _imp->hook_cache.clear();
    _imp->dirs.push_back(std::make_pair(dir, v));
}

int
Hooker::perform_hook(const Hook & hook) const
{
    int max_exit_status(0);

    Context context("When triggering hook '" + hook.name() + "'");
    Log::get_instance()->message(ll_debug, lc_no_context, "Starting hook '" + hook.name() + "'");

    /* repo hooks first */

    for (PackageDatabase::RepositoryIterator r(_imp->env->package_database()->begin_repositories()),
            r_end(_imp->env->package_database()->end_repositories()) ; r != r_end ; ++r)
        if ((*r)->hook_interface)
            max_exit_status = std::max(max_exit_status, ((*r)->hook_interface->perform_hook(hook)));

    /* file hooks, but only if necessary */

    HookPresentCache::iterator cache_entry(_imp->hook_cache.end());

    if (_imp->hook_cache.end() != ((cache_entry = _imp->hook_cache.find(hook.name()))))
        if (! cache_entry->second)
            return max_exit_status;

    bool had_hook(false);
    for (std::list<std::pair<FSEntry, bool> >::const_iterator h(_imp->dirs.begin()), h_end(_imp->dirs.end()) ;
            h != h_end ; ++h)
    {
        FSEntry hh(h->first / hook.name());
        if (! hh.is_directory())
            continue;

        std::list<FSEntry> hooks;
        std::copy(DirIterator(hh), DirIterator(),
                filter_inserter(std::back_inserter(hooks), IsFileWithExtension(".bash")));

        if (! hooks.empty())
            had_hook = true;

        for (std::list<FSEntry>::const_iterator hk(hooks.begin()),
                hk_end(hooks.end()) ; hk != hk_end ; ++hk)
        {
            Context c("When running hook script '" + stringify(*hk) +
                    "' for hook '" + hook.name() + "':");
            Log::get_instance()->message(ll_debug, lc_no_context, "Starting hook script '" +
                    stringify(hh) + "' for '" + hook.name() + "'");

            Command cmd(Command("bash '" + stringify(*hk) + "'")
                    .with_setenv("ROOT", stringify(_imp->env->root()))
                    .with_setenv("HOOK", hook.name())
                    .with_setenv("HOOK_LOG_LEVEL", stringify(Log::get_instance()->log_level()))
                    .with_setenv("PALUDIS_EBUILD_DIR", getenv_with_default("PALUDIS_EBUILD_DIR", LIBEXECDIR "/paludis"))
                    .with_setenv("PALUDIS_REDUCED_GID", stringify(_imp->env->reduced_gid()))
                    .with_setenv("PALUDIS_REDUCED_UID", stringify(_imp->env->reduced_uid()))
                    .with_setenv("PALUDIS_COMMAND", _imp->env->paludis_command()));

            if (h->second)
                cmd
                    .with_stdout_prefix(strip_trailing_string(hk->basename(), ".bash") + "> ")
                    .with_stderr_prefix(strip_trailing_string(hk->basename(), ".bash") + "> ");

            for (Hook::Iterator x(hook.begin()), x_end(hook.end()) ; x != x_end ; ++x)
                cmd.with_setenv(x->first, x->second);

            int exit_status(run_command(cmd));
            if (0 == exit_status)
                Log::get_instance()->message(ll_debug, lc_no_context, "Hook '" + stringify(*hk)
                        + "' returned success '" + stringify(exit_status) + "'");
            else
                Log::get_instance()->message(ll_warning, lc_no_context, "Hook '" + stringify(*hk)
                        + "' returned failure '" + stringify(exit_status) + "'");

            max_exit_status = std::max(max_exit_status, exit_status);
        }
    }

    if (_imp->hook_cache.end() == cache_entry)
        _imp->hook_cache.insert(std::make_pair(hook.name(), had_hook));

    return max_exit_status;
}

