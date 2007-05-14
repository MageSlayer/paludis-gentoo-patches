/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
 * Copyright (c) 2007 Piotr Jaroszyński <peper@gentoo.org>
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
#include <paludis/hook.hh>
#include <paludis/hashed_containers.hh>
#include <paludis/package_database.hh>
#include <paludis/util/collection_concrete.hh>
#include <paludis/util/log.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/system.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/graph.hh>
#include <paludis/util/graph-impl.hh>
#include <paludis/util/pstream.hh>
#include <paludis/util/tokeniser.hh>
#include <list>
#include <iterator>

using namespace paludis;

namespace
{
    class HookFile;

    class HookFile :
        private InstantiationPolicy<HookFile, instantiation_method::NonCopyableTag>
    {
        public:
            virtual ~HookFile()
            {
            }

            virtual HookResult run(const Hook &) const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
            virtual const FSEntry file_name() const = 0;
            virtual void add_dependencies(const Hook &, DirectedGraph<std::string, int> &) = 0;
    };

    class BashHookFile :
        public HookFile
    {
        private:
            const FSEntry _file_name;
            const bool _run_prefixed;
            const Environment * const _env;

        public:
            BashHookFile(const FSEntry & f, const bool r, const Environment * const e) :
                _file_name(f),
                _run_prefixed(r),
                _env(e)
            {
            }

            virtual HookResult run(const Hook &) const PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual const FSEntry file_name() const
            {
                return _file_name;
            }

            virtual void add_dependencies(const Hook &, DirectedGraph<std::string, int> &)
            {
            }
    };

    class FancyHookFile :
        public HookFile
    {
        private:
            const FSEntry _file_name;
            const bool _run_prefixed;
            const Environment * const _env;

            virtual void _add_dependency_class(const Hook &, DirectedGraph<std::string, int> &, bool);

        public:
            FancyHookFile(const FSEntry & f, const bool r, const Environment * const e) :
                _file_name(f),
                _run_prefixed(r),
                _env(e)
            {
            }

            virtual HookResult run(const Hook &) const PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual const FSEntry file_name() const
            {
                return _file_name;
            }

            virtual void add_dependencies(const Hook &, DirectedGraph<std::string, int> &);
    };
}

HookResult
BashHookFile::run(const Hook & hook) const
{
    Context c("When running hook script '" + stringify(file_name()) + "' for hook '" + hook.name() + "':");

    Log::get_instance()->message(ll_debug, lc_no_context, "Starting hook script '" +
            stringify(file_name()) + "' for '" + hook.name() + "'");

    Command cmd(Command("bash '" + stringify(file_name()) + "'")
            .with_setenv("ROOT", stringify(_env->root()))
            .with_setenv("HOOK", hook.name())
            .with_setenv("HOOK_FILE", stringify(file_name()))
            .with_setenv("HOOK_LOG_LEVEL", stringify(Log::get_instance()->log_level()))
            .with_setenv("PALUDIS_EBUILD_DIR", getenv_with_default("PALUDIS_EBUILD_DIR", LIBEXECDIR "/paludis"))
            .with_setenv("PALUDIS_REDUCED_GID", stringify(_env->reduced_gid()))
            .with_setenv("PALUDIS_REDUCED_UID", stringify(_env->reduced_uid()))
            .with_setenv("PALUDIS_COMMAND", _env->paludis_command()));

    if (hook.output_dest == hod_stdout && _run_prefixed)
        cmd
            .with_stdout_prefix(strip_trailing_string(file_name().basename(), ".bash") + "> ")
            .with_stderr_prefix(strip_trailing_string(file_name().basename(), ".bash") + "> ");

    for (Hook::Iterator x(hook.begin()), x_end(hook.end()) ; x != x_end ; ++x)
        cmd.with_setenv(x->first, x->second);

    int exit_status(0);
    std::string output("");
    if (hook.output_dest == hod_grab)
    {
        PStream s(cmd);
        output = strip_trailing(std::string((std::istreambuf_iterator<char>(s)), std::istreambuf_iterator<char>()),
                " \t\n");
        exit_status = s.exit_status();
    }
    else
        exit_status = run_command(cmd);

    if (0 == exit_status)
        Log::get_instance()->message(ll_debug, lc_no_context, "Hook '" + stringify(file_name())
                + "' returned success '" + stringify(exit_status) + "'");
    else
        Log::get_instance()->message(ll_warning, lc_no_context, "Hook '" + stringify(file_name())
                + "' returned failure '" + stringify(exit_status) + "'");

    return HookResult(exit_status, output);
}

HookResult
FancyHookFile::run(const Hook & hook) const
{
    Context c("When running hook script '" + stringify(file_name()) + "' for hook '" + hook.name() + "':");

    Log::get_instance()->message(ll_debug, lc_no_context, "Starting hook script '" +
            stringify(file_name()) + "' for '" + hook.name() + "'");

    Command cmd(getenv_with_default("PALUDIS_HOOKER_DIR", LIBEXECDIR "/paludis") +
            "/hooker.bash '" + stringify(file_name()) + "' 'hook_run_" + stringify(hook.name()) + "'");

    cmd
        .with_setenv("ROOT", stringify(_env->root()))
        .with_setenv("HOOK", hook.name())
        .with_setenv("HOOK_FILE", stringify(file_name()))
        .with_setenv("HOOK_LOG_LEVEL", stringify(Log::get_instance()->log_level()))
        .with_setenv("PALUDIS_EBUILD_DIR", getenv_with_default("PALUDIS_EBUILD_DIR", LIBEXECDIR "/paludis"))
        .with_setenv("PALUDIS_REDUCED_GID", stringify(_env->reduced_gid()))
        .with_setenv("PALUDIS_REDUCED_UID", stringify(_env->reduced_uid()))
        .with_setenv("PALUDIS_COMMAND", _env->paludis_command());

    if (hook.output_dest == hod_stdout && _run_prefixed)
        cmd
            .with_stdout_prefix(strip_trailing_string(file_name().basename(), ".hook") + "> ")
            .with_stderr_prefix(strip_trailing_string(file_name().basename(), ".hook") + "> ");

    for (Hook::Iterator x(hook.begin()), x_end(hook.end()) ; x != x_end ; ++x)
        cmd.with_setenv(x->first, x->second);

    int exit_status(0);
    std::string output("");
    if (hook.output_dest == hod_grab)
    {
        PStream s(cmd);
        output = strip_trailing(std::string((std::istreambuf_iterator<char>(s)), std::istreambuf_iterator<char>()),
                " \t\n");
        exit_status = s.exit_status();
    }
    else
        exit_status = run_command(cmd);

    if (0 == exit_status)
        Log::get_instance()->message(ll_debug, lc_no_context, "Hook '" + stringify(file_name())
                + "' returned success '" + stringify(exit_status) + "'");
    else
        Log::get_instance()->message(ll_warning, lc_no_context, "Hook '" + stringify(file_name())
                + "' returned failure '" + stringify(exit_status) + "'");

    return HookResult(exit_status, output);
}

void
FancyHookFile::add_dependencies(const Hook & hook, DirectedGraph<std::string, int> & g)
{
    Context c("When finding dependencies of hook script '" + stringify(file_name()) + "' for hook '" + hook.name() + "':");

    _add_dependency_class(hook, g, false);
    _add_dependency_class(hook, g, true);
}

void
FancyHookFile::_add_dependency_class(const Hook & hook, DirectedGraph<std::string, int> & g, bool depend)
{
    Context context("When adding dependency class '" + stringify(depend ? "depend" : "after") + "' for hook '"
            + stringify(hook.name()) + "' file '" + stringify(file_name()) + "':");

    Log::get_instance()->message(ll_debug, lc_no_context, "Starting hook script '" +
            stringify(file_name()) + "' for dependencies of '" + hook.name() + "'");

    Command cmd(getenv_with_default("PALUDIS_HOOKER_DIR", LIBEXECDIR "/paludis") +
            "/hooker.bash '" + stringify(file_name()) + "' 'hook_" + (depend ? "depend" : "after") + "_" +
            stringify(hook.name()) + "'");

    cmd
        .with_setenv("ROOT", stringify(_env->root()))
        .with_setenv("HOOK", hook.name())
        .with_setenv("HOOK_FILE", stringify(file_name()))
        .with_setenv("HOOK_LOG_LEVEL", stringify(Log::get_instance()->log_level()))
        .with_setenv("PALUDIS_EBUILD_DIR", getenv_with_default("PALUDIS_EBUILD_DIR", LIBEXECDIR "/paludis"))
        .with_setenv("PALUDIS_REDUCED_GID", stringify(_env->reduced_gid()))
        .with_setenv("PALUDIS_REDUCED_UID", stringify(_env->reduced_uid()))
        .with_setenv("PALUDIS_COMMAND", _env->paludis_command());

    cmd.with_stderr_prefix(strip_trailing_string(file_name().basename(), ".bash") + "> ");

    for (Hook::Iterator x(hook.begin()), x_end(hook.end()) ; x != x_end ; ++x)
        cmd.with_setenv(x->first, x->second);

    PStream s(cmd);
    std::string deps((std::istreambuf_iterator<char>(s)), std::istreambuf_iterator<char>());

    int exit_status(s.exit_status());
    if (0 == exit_status)
    {
        Log::get_instance()->message(ll_debug, lc_no_context, "Hook dependencies for '" + stringify(file_name())
                + "' returned success '" + stringify(exit_status) + "', result '" + deps + "'");

        std::set<std::string> deps_s;
        WhitespaceTokeniser::get_instance()->tokenise(deps, std::inserter(deps_s, deps_s.end()));

        for (std::set<std::string>::const_iterator d(deps_s.begin()), d_end(deps_s.end()) ;
                d != d_end ; ++d)
        {
            if (g.has_node(*d))
                g.add_edge(strip_trailing_string(file_name().basename(), ".hook"), *d, 0);
            else if (depend)
                Log::get_instance()->message(ll_warning, lc_context, "Hook dependency '" + stringify(*d) +
                        "' for '" + stringify(file_name()) + "' not found");
            else
                Log::get_instance()->message(ll_debug, lc_context, "Hook after '" + stringify(*d) +
                        "' for '" + stringify(file_name()) + "' not found");
        }
    }
    else
        Log::get_instance()->message(ll_warning, lc_no_context, "Hook dependencies for '" + stringify(file_name())
                + "' returned failure '" + stringify(exit_status) + "'");
}

namespace paludis
{
    template<>
    struct Implementation<Hooker>
    {
        const Environment * const env;
        std::list<std::pair<FSEntry, bool> > dirs;
        mutable std::map<std::string, std::list<std::tr1::shared_ptr<HookFile> > > hook_files;

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
    _imp->hook_files.clear();
    _imp->dirs.push_back(std::make_pair(dir, v));
}

HookResult
Hooker::perform_hook(const Hook & hook) const
{
    HookResult result(0, "");

    Context context("When triggering hook '" + hook.name() + "':");
    Log::get_instance()->message(ll_debug, lc_no_context, "Starting hook '" + hook.name() + "'");

    /* repo hooks first */

    do
    {
        switch (hook.output_dest)
        {
            case hod_stdout:
                for (PackageDatabase::RepositoryIterator r(_imp->env->package_database()->begin_repositories()),
                        r_end(_imp->env->package_database()->end_repositories()) ; r != r_end ; ++r)
                    if ((*r)->hook_interface)
                        result.max_exit_status = std::max(result.max_exit_status,
                                ((*r)->hook_interface->perform_hook(hook)).max_exit_status);
                continue;

            case hod_grab:
                for (PackageDatabase::RepositoryIterator r(_imp->env->package_database()->begin_repositories()),
                        r_end(_imp->env->package_database()->end_repositories()) ; r != r_end ; ++r)
                    if ((*r)->hook_interface)
                    {
                        HookResult tmp((*r)->hook_interface->perform_hook(hook));
                        if (tmp > result)
                            result = tmp;
                        else if (! tmp.output.empty())
                        {
                            if (hook.validate_value(tmp.output))
                            {
                                if (result.max_exit_status == 0)
                                    return tmp;
                            }
                            else
                                Log::get_instance()->message(ll_warning, lc_context)
                                    << "Hook returned invalid output: '" << tmp.output << "'";
                        }
                    }
                continue;

            case last_hod:
                ;
        }
        throw InternalError(PALUDIS_HERE, "Bad HookOutputDestination value '" + paludis::stringify(
                    static_cast<int>(hook.output_dest)));
    } while(false);

    /* file hooks, but only if necessary */

    std::map<std::string, std::list<std::tr1::shared_ptr<HookFile> > >::iterator h(_imp->hook_files.find(hook.name()));

    if (h == _imp->hook_files.end())
    {
        h = _imp->hook_files.insert(std::make_pair(hook.name(), std::list<std::tr1::shared_ptr<HookFile> >())).first;

        std::map<std::string, std::tr1::shared_ptr<HookFile> > hook_files;

        for (std::list<std::pair<FSEntry, bool> >::const_iterator d(_imp->dirs.begin()), d_end(_imp->dirs.end()) ;
                d != d_end ; ++d)
        {
            if (! (d->first / hook.name()).is_directory())
                continue;

            for (DirIterator e(d->first / hook.name()), e_end ; e != e_end ; ++e)
            {
                if (is_file_with_extension(*e, ".bash", IsFileWithOptions()))
                    if (! hook_files.insert(std::make_pair(strip_trailing_string(e->basename(), ".bash"),
                                    std::tr1::shared_ptr<HookFile>(new BashHookFile(*e, d->second, _imp->env)))).second)
                        Log::get_instance()->message(ll_warning, lc_context, "Discarding hook file '" + stringify(*e)
                                + "' because of naming conflict with '" + stringify(
                                    hook_files.find(stringify(strip_trailing_string(e->basename(), ".bash")))->second->file_name()) + "'");

                if (is_file_with_extension(*e, ".hook", IsFileWithOptions()))
                    if (! hook_files.insert(std::make_pair(strip_trailing_string(e->basename(), ".hook"),
                                    std::tr1::shared_ptr<HookFile>(new FancyHookFile(*e, d->second, _imp->env)))).second)
                        Log::get_instance()->message(ll_warning, lc_context, "Discarding hook file '" + stringify(*e)
                                + "' because of naming conflict with '" + stringify(
                                    hook_files.find(stringify(strip_trailing_string(e->basename(), ".hook")))->second->file_name()) + "'");
            }
        }

        DirectedGraph<std::string, int> hook_deps;
        {
            Context context_local("When determining hook dependencies for '" + hook.name() + "':");
            for (std::map<std::string, std::tr1::shared_ptr<HookFile> >::const_iterator f(hook_files.begin()), f_end(hook_files.end()) ;
                    f != f_end ; ++f)
                hook_deps.add_node(f->first);

            for (std::map<std::string, std::tr1::shared_ptr<HookFile> >::const_iterator f(hook_files.begin()), f_end(hook_files.end()) ;
                    f != f_end ; ++f)
                f->second->add_dependencies(hook, hook_deps);
        }

        std::list<std::string> ordered;
        {
            Context context_local("When determining hook ordering for '" + hook.name() + "':");
            try
            {
                hook_deps.topological_sort(std::back_inserter(ordered));
            }
            catch (const NoGraphTopologicalOrderExistsError & e)
            {
                Log::get_instance()->message(ll_warning, lc_context, "Could not resolve dependency order for hook '"
                        + hook.name() + "' due to exception '" + e.message() + "' (" + e.what() + "'), skipping hooks '" +
                        join(e.remaining_nodes()->begin(), e.remaining_nodes()->end(), "', '") + "' and using hooks '" + join(ordered.begin(),
                            ordered.end(), "', '") + "' in that order");;
            }
        }

        for (std::list<std::string>::const_iterator o(ordered.begin()), o_end(ordered.end()) ;
                o != o_end ; ++o)
            h->second.push_back(hook_files.find(*o)->second);
    }

    if (! h->second.empty())
    {
        do
        {
            switch (hook.output_dest)
            {
                case hod_stdout:
                    for (std::list<std::tr1::shared_ptr<HookFile> >::const_iterator f(h->second.begin()),
                            f_end(h->second.end()) ; f != f_end ; ++f)
                        result.max_exit_status = std::max(result.max_exit_status, (*f)->run(hook).max_exit_status);
                    continue;

                case hod_grab:
                    for (std::list<std::tr1::shared_ptr<HookFile> >::const_iterator f(h->second.begin()),
                            f_end(h->second.end()) ; f != f_end ; ++f)
                    {
                        HookResult tmp((*f)->run(hook));
                        if (tmp > result)
                            result = tmp;
                        else if (! tmp.output.empty())
                        {
                            if (hook.validate_value(tmp.output))
                            {
                                if (result.max_exit_status == 0)
                                    return tmp;
                            }
                            else
                                Log::get_instance()->message(ll_warning, lc_context)
                                    << "Hook returned invalid output: '" << tmp.output << "'";
                        }
                    }
                    continue;

                case last_hod:
                    ;
            }
            throw InternalError(PALUDIS_HERE, "Bad HookOutputDestination value '" + paludis::stringify(
                        static_cast<int>(hook.output_dest)));
        } while(false);

    }

    return result;
}

