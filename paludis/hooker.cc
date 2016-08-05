/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010, 2011, 2013 Ciaran McCreesh
 * Copyright (c) 2007 Piotr Jaroszyński
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

#include <paludis/hooker.hh>
#include <paludis/environment.hh>
#include <paludis/hook.hh>
#include <paludis/about.hh>
#include <paludis/output_manager.hh>
#include <paludis/metadata_key.hh>
#include <paludis/repository.hh>

#include <paludis/util/log.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/system.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/graph-impl.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/join.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/singleton-impl.hh>
#include <paludis/util/process.hh>
#include <paludis/util/fs_iterator.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/util/env_var_names.hh>

#include <list>
#include <iterator>
#include <mutex>
#include <dlfcn.h>
#include <stdint.h>

#include "config.h"

using namespace paludis;

HookFile::~HookFile()
{
}

namespace
{
    static const std::string so_suffix("_" + stringify(PALUDIS_PC_SLOT)
            + ".so." + stringify(100 * PALUDIS_VERSION_MAJOR + PALUDIS_VERSION_MINOR));

    class BashHookFile :
        public HookFile
    {
        private:
            const FSPath _file_name;
            const bool _run_prefixed;
            const Environment * const _env;

        public:
            BashHookFile(const FSPath & f, const bool r, const Environment * const e) :
                _file_name(f),
                _run_prefixed(r),
                _env(e)
            {
            }

            virtual HookResult run(
                    const Hook &,
                    const std::shared_ptr<OutputManager> &) const PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual const FSPath file_name() const
            {
                return _file_name;
            }

            virtual void add_dependencies(const Hook &, DirectedGraph<std::string, int> &)
            {
            }

            virtual const std::shared_ptr<const Sequence<std::string> > auto_hook_names() const
            {
                return std::make_shared<Sequence<std::string>>();
            }
    };

    class FancyHookFile :
        public HookFile
    {
        private:
            const FSPath _file_name;
            const bool _run_prefixed;
            const Environment * const _env;

            void _add_dependency_class(const Hook &, DirectedGraph<std::string, int> &, bool);

        public:
            FancyHookFile(const FSPath & f, const bool r, const Environment * const e) :
                _file_name(f),
                _run_prefixed(r),
                _env(e)
            {
            }

            virtual HookResult run(
                    const Hook &,
                    const std::shared_ptr<OutputManager> &) const PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual const FSPath file_name() const
            {
                return _file_name;
            }

            virtual void add_dependencies(const Hook &, DirectedGraph<std::string, int> &);

            virtual const std::shared_ptr<const Sequence<std::string> > auto_hook_names() const;
    };

    class SoHookFile :
        public HookFile
    {
        private:
            const FSPath _file_name;
            const Environment * const _env;

            void * _dl;
            HookResult (*_run)(const Environment *, const Hook &, const std::shared_ptr<OutputManager> &);
            void (*_add_dependencies)(const Environment *, const Hook &, DirectedGraph<std::string, int> &);
            const std::shared_ptr<const Sequence<std::string > > (*_auto_hook_names)(const Environment *);

        public:
            SoHookFile(const FSPath &, const bool, const Environment * const);

            virtual HookResult run(
                    const Hook &,
                    const std::shared_ptr<OutputManager> &) const PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual const FSPath file_name() const
            {
                return _file_name;
            }

            virtual void add_dependencies(const Hook &, DirectedGraph<std::string, int> &);

            virtual const std::shared_ptr<const Sequence<std::string> > auto_hook_names() const;
    };
}

HookResult
BashHookFile::run(
        const Hook & hook,
        const std::shared_ptr<OutputManager> & optional_output_manager) const
{
    Context c("When running hook script '" + stringify(file_name()) + "' for hook '" + hook.name() + "':");

    Log::get_instance()->message("hook.bash.starting", ll_debug, lc_no_context) << "Starting hook script '" <<
        file_name() << "' for '" << hook.name() << "'";

    Process process(ProcessCommand({ "bash", stringify(file_name()) }));
    process
        .setenv("ROOT", stringify(_env->preferred_root_key()->parse_value()))
        .setenv("HOOK", hook.name())
        .setenv("HOOK_FILE", stringify(file_name()))
        .setenv("HOOK_LOG_LEVEL", stringify(Log::get_instance()->log_level()))
        .setenv("PALUDIS_EBUILD_DIR", getenv_with_default(env_vars::ebuild_dir, LIBEXECDIR "/paludis"))
        .setenv("PALUDIS_REDUCED_GID", stringify(_env->reduced_gid()))
        .setenv("PALUDIS_REDUCED_UID", stringify(_env->reduced_uid()));

    if (hook.output_dest == hod_stdout && _run_prefixed)
        process
            .prefix_stdout(strip_trailing_string(file_name().basename(), ".bash") + "> ")
            .prefix_stderr(strip_trailing_string(file_name().basename(), ".bash") + "> ");

    for (Hook::ConstIterator x(hook.begin()), x_end(hook.end()) ; x != x_end ; ++x)
        process.setenv(x->first, x->second);

    if (optional_output_manager)
    {
        /* hod_grab can override this later */
        process.capture_stdout(optional_output_manager->stdout_stream());
        process.capture_stderr(optional_output_manager->stderr_stream());
    }

    int exit_status(0);
    std::string output("");
    if (hook.output_dest == hod_grab)
    {
        std::stringstream s;
        process.capture_stdout(s);
        exit_status = process.run().wait();
        output = strip_trailing(std::string((std::istreambuf_iterator<char>(s)), std::istreambuf_iterator<char>()),
                " \t\n");
    }
    else
        exit_status = process.run().wait();

    if (0 == exit_status)
        Log::get_instance()->message("hook.bash.success", ll_debug, lc_no_context) << "Hook '" << file_name()
                << "' returned success '" << exit_status << "'";
    else
        Log::get_instance()->message("hook.bash.failure", ll_warning, lc_no_context) << "Hook '" << file_name()
            << "' returned failure '" << exit_status << "'";

    return make_named_values<HookResult>(
            n::max_exit_status() = exit_status,
            n::output() = output);
}

HookResult
FancyHookFile::run(const Hook & hook,
        const std::shared_ptr<OutputManager> & optional_output_manager) const
{
    Context c("When running hook script '" + stringify(file_name()) + "' for hook '" + hook.name() + "':");

    Log::get_instance()->message("hook.fancy.starting", ll_debug, lc_no_context) << "Starting hook script '"
        << file_name() << "' for '" << hook.name() << "'";

    Process process(ProcessCommand({ "sh", "-c", getenv_with_default(env_vars::hooker_dir, LIBEXECDIR "/paludis") +
                "/hooker.bash '" + stringify(file_name()) + "' 'hook_run_" + stringify(hook.name()) + "'" }));

    process
        .setenv("ROOT", stringify(_env->preferred_root_key()->parse_value()))
        .setenv("HOOK", hook.name())
        .setenv("HOOK_FILE", stringify(file_name()))
        .setenv("HOOK_LOG_LEVEL", stringify(Log::get_instance()->log_level()))
        .setenv("PALUDIS_EBUILD_DIR", getenv_with_default(env_vars::ebuild_dir, LIBEXECDIR "/paludis"))
        .setenv("PALUDIS_REDUCED_GID", stringify(_env->reduced_gid()))
        .setenv("PALUDIS_REDUCED_UID", stringify(_env->reduced_uid()));

    if (hook.output_dest == hod_stdout && _run_prefixed)
        process
            .prefix_stdout(strip_trailing_string(file_name().basename(), ".hook") + "> ")
            .prefix_stderr(strip_trailing_string(file_name().basename(), ".hook") + "> ");

    for (Hook::ConstIterator x(hook.begin()), x_end(hook.end()) ; x != x_end ; ++x)
        process.setenv(x->first, x->second);

    if (optional_output_manager)
    {
        /* hod_grab can override this later */
        process.capture_stdout(optional_output_manager->stdout_stream());
        process.capture_stderr(optional_output_manager->stderr_stream());
    }

    int exit_status(0);
    std::string output("");
    if (hook.output_dest == hod_grab)
    {
        std::stringstream s;
        process.capture_stdout(s);
        exit_status = process.run().wait();
        output = strip_trailing(std::string((std::istreambuf_iterator<char>(s)), std::istreambuf_iterator<char>()),
                " \t\n");
    }
    else
        exit_status = process.run().wait();

    if (0 == exit_status)
        Log::get_instance()->message("hook.fancy.success", ll_debug, lc_no_context) << "Hook '" << file_name()
            << "' returned success '" << exit_status << "'";
    else
        Log::get_instance()->message("hook.fancy.failure", ll_warning, lc_no_context) << "Hook '" << file_name()
            << "' returned failure '" << exit_status << "'";

    return make_named_values<HookResult>(
            n::max_exit_status() = exit_status,
            n::output() = output
            );
}

const std::shared_ptr<const Sequence<std::string > >
FancyHookFile::auto_hook_names() const
{
    Context c("When querying auto hook names for fancy hook '" + stringify(file_name()) + "':");

    Log::get_instance()->message("hook.fancy.starting", ll_debug, lc_no_context) << "Starting hook script '" <<
        file_name() << "' for auto hook names";

    Process process(ProcessCommand({ "sh", "-c", getenv_with_default(env_vars::hooker_dir, LIBEXECDIR "/paludis") +
            "/hooker.bash '" + stringify(file_name()) + "' 'hook_auto_names'" }));

    process
        .setenv("ROOT", stringify(_env->preferred_root_key()->parse_value()))
        .setenv("HOOK_FILE", stringify(file_name()))
        .setenv("HOOK_LOG_LEVEL", stringify(Log::get_instance()->log_level()))
        .setenv("PALUDIS_EBUILD_DIR", getenv_with_default(env_vars::ebuild_dir, LIBEXECDIR "/paludis"))
        .setenv("PALUDIS_REDUCED_GID", stringify(_env->reduced_gid()))
        .setenv("PALUDIS_REDUCED_UID", stringify(_env->reduced_uid()));

    int exit_status(0);
    std::string output("");
    {
        std::stringstream s;
        process.capture_stdout(s);
        exit_status = process.run().wait();
        output = strip_trailing(std::string((std::istreambuf_iterator<char>(s)), std::istreambuf_iterator<char>()),
                " \t\n");
    }

    if (0 == exit_status)
    {
        std::shared_ptr<Sequence<std::string> > result(std::make_shared<Sequence<std::string>>());
        tokenise_whitespace(output, result->back_inserter());
        Log::get_instance()->message("hook.fancy.success", ll_debug, lc_no_context) << "Hook '" << file_name()
            << "' returned success '" << exit_status << "' for auto hook names, result ("
            << join(result->begin(), result->end(), ", ") << ")";
        return result;
    }
    else
    {
        Log::get_instance()->message("hook.fancy.failure", ll_warning, lc_no_context) << "Hook '" << file_name()
            << "' returned failure '" << exit_status << "' for auto hook names";
        return std::make_shared<Sequence<std::string>>();
    }
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

    Log::get_instance()->message("hook.fancy.starting_dependencies", ll_debug, lc_no_context)
        << "Starting hook script '" << file_name() << "' for dependencies of '" << hook.name() << "'";

    Process process(ProcessCommand({ "sh", "-c", getenv_with_default(env_vars::hooker_dir, LIBEXECDIR "/paludis") +
            "/hooker.bash '" + stringify(file_name()) + "' 'hook_" + (depend ? "depend" : "after") + "_" +
            stringify(hook.name()) + "'" }));

    process
        .setenv("ROOT", stringify(_env->preferred_root_key()->parse_value()))
        .setenv("HOOK", hook.name())
        .setenv("HOOK_FILE", stringify(file_name()))
        .setenv("HOOK_LOG_LEVEL", stringify(Log::get_instance()->log_level()))
        .setenv("PALUDIS_EBUILD_DIR", getenv_with_default(env_vars::ebuild_dir, LIBEXECDIR "/paludis"))
        .setenv("PALUDIS_REDUCED_GID", stringify(_env->reduced_gid()))
        .setenv("PALUDIS_REDUCED_UID", stringify(_env->reduced_uid()));

    process.prefix_stderr(strip_trailing_string(file_name().basename(), ".bash") + "> ");

    for (Hook::ConstIterator x(hook.begin()), x_end(hook.end()) ; x != x_end ; ++x)
        process.setenv(x->first, x->second);

    std::stringstream s;
    process.capture_stdout(s);
    int exit_status(process.run().wait());

    std::string deps((std::istreambuf_iterator<char>(s)), std::istreambuf_iterator<char>());

    if (0 == exit_status)
    {
        Log::get_instance()->message("hook.fancy.success_dependencies", ll_debug, lc_no_context)
            << "Hook dependencies for '" << file_name() << "' returned success '" << exit_status << "', result '" << deps << "'";

        std::set<std::string> deps_s;
        tokenise_whitespace(deps, std::inserter(deps_s, deps_s.end()));

        for (const auto & deps_ : deps_s)
        {
            if (g.has_node(deps_))
                g.add_edge(strip_trailing_string(file_name().basename(), ".hook"), deps_, 0);
            else if (depend)
                Log::get_instance()->message("hook.fancy.dependency_not_found", ll_warning, lc_context)
                    << "Hook dependency '" << deps_ << "' for '" << file_name() << "' not found";
            else
                Log::get_instance()->message("hook.fancy.after_not_found", ll_debug, lc_context)
                    << "Hook after '" << deps_ << "' for '" << file_name() << "' not found";
        }
    }
    else
        Log::get_instance()->message("hook.fancy.failure_dependencies", ll_warning, lc_no_context)
            << "Hook dependencies for '" << file_name() << "' returned failure '" << exit_status << "'";
}

SoHookFile::SoHookFile(const FSPath & f, const bool, const Environment * const e) :
    _file_name(f),
    _env(e),
    _dl(0),
    _run(0),
    _add_dependencies(0)
{
    /* don't use RTLD_LOCAL, g++ is over happy about template instantiations, and it
     * can lead to multiple singleton instances. */
    _dl = dlopen(stringify(f).c_str(), RTLD_GLOBAL | RTLD_NOW);

    if (_dl)
    {
        _run = reinterpret_cast<HookResult (*)(
            const Environment *, const Hook &, const std::shared_ptr<OutputManager> &)>(
                reinterpret_cast<uintptr_t>(dlsym(_dl, "paludis_hook_run_3")));

        if (! _run)
            Log::get_instance()->message("hook.so.no_paludis_hook_run_3", ll_warning, lc_no_context)
                << ".so hook '" << f << "' does not define the paludis_hook_run_3 function";

        _add_dependencies = reinterpret_cast<void (*)(
            const Environment *, const Hook &, DirectedGraph<std::string, int> &)>(
                reinterpret_cast<uintptr_t>(dlsym(_dl, "paludis_hook_add_dependencies")));

        _auto_hook_names = reinterpret_cast<const std::shared_ptr<const Sequence<std::string> > (*)(
            const Environment *)>(
                reinterpret_cast<uintptr_t>(dlsym(_dl, "paludis_hook_auto_phases")));
    }
    else
        Log::get_instance()->message("hook.so.dlopen_failed", ll_warning, lc_no_context)
            << "Opening .so hook '" << f << "' failed: " << dlerror();
}

HookResult
SoHookFile::run(const Hook & hook,
        const std::shared_ptr<OutputManager> & optional_output_manager) const
{
    Context c("When running .so hook '" + stringify(file_name()) + "' for hook '" + hook.name() + "':");

    if (! _run)
        return make_named_values<HookResult>(n::max_exit_status() = 0, n::output() = "");

    Log::get_instance()->message("hook.so.starting", ll_debug, lc_no_context) << "Starting .so hook '" <<
        file_name() << "' for '" << hook.name() << "'";

    return _run(_env, hook, optional_output_manager);
}

void
SoHookFile::add_dependencies(const Hook & hook, DirectedGraph<std::string, int> & g)
{
    if (_add_dependencies)
        _add_dependencies(_env, hook, g);
}

const std::shared_ptr<const Sequence<std::string > >
SoHookFile::auto_hook_names() const
{
    Context c("When querying auto hook names for .so hook '" + stringify(file_name()) + "':");

    if (! _auto_hook_names)
        return std::make_shared<Sequence<std::string>>();

    return _auto_hook_names(_env);
}

namespace paludis
{
    template<>
    struct Imp<Hooker>
    {
        const Environment * const env;
        std::list<std::pair<FSPath, bool> > dirs;

        mutable std::recursive_mutex hook_files_mutex;
        mutable std::map<std::string, std::shared_ptr<Sequence<std::shared_ptr<HookFile> > > > hook_files;
        mutable std::map<std::string, std::map<std::string, std::shared_ptr<HookFile> > > auto_hook_files;
        mutable bool has_auto_hook_files;

        Imp(const Environment * const e) :
            env(e),
            has_auto_hook_files(false)
        {
        }

        void need_auto_hook_files() const
        {
            std::unique_lock<std::recursive_mutex> l(hook_files_mutex);

            if (has_auto_hook_files)
                return;
            has_auto_hook_files = true;

            Context context("When loading auto hooks:");

            for (const auto & dir : dirs)
            {
                FSPath d_a(dir.first / "auto");
                if (! d_a.stat().is_directory())
                    continue;

                for (FSIterator e(d_a, { }), e_end ; e != e_end ; ++e)
                {
                    std::shared_ptr<HookFile> hook_file;
                    std::string name;

                    if (is_file_with_extension(*e, ".hook", { }))
                    {
                        hook_file = std::make_shared<FancyHookFile>(*e, dir.second, env);
                        name = strip_trailing_string(e->basename(), ".hook");
                    }
                    else if (is_file_with_extension(*e, so_suffix, { }))
                    {
                        hook_file = std::make_shared<SoHookFile>(*e, dir.second, env);
                        name = strip_trailing_string(e->basename(), so_suffix);
                    }

                    if (! hook_file)
                        continue;

                    const std::shared_ptr<const Sequence<std::string> > names(hook_file->auto_hook_names());
                    for (Sequence<std::string>::ConstIterator n(names->begin()), n_end(names->end()) ;
                            n != n_end ; ++n)
                    {
                        if (! auto_hook_files[*n].insert(std::make_pair(name, hook_file)).second)
                            Log::get_instance()->message("hook.discarding", ll_warning, lc_context) << "Discarding hook file '" << *e
                                << "' in phase '" << *n << "' because of naming conflict with '"
                                << auto_hook_files[*n].find(name)->second->file_name() << "'";
                    }
                }
            }
        }
    };
}

Hooker::Hooker(const Environment * const e) :
    _imp(e)
{
}

Hooker::~Hooker()
{
}

void
Hooker::add_dir(const FSPath & dir, const bool v)
{
    std::unique_lock<std::recursive_mutex> l(_imp->hook_files_mutex);
    _imp->hook_files.clear();
    _imp->auto_hook_files.clear();
    _imp->dirs.push_back(std::make_pair(dir, v));
}

namespace
{
    struct PyHookFileHandle :
        Singleton<PyHookFileHandle>
    {
        std::mutex mutex;
        void * handle;
        std::shared_ptr<HookFile> (* create_py_hook_file_handle)(const FSPath &,
                const bool, const Environment * const);


        PyHookFileHandle() :
            handle(0),
            create_py_hook_file_handle(0)
        {
        }

        ~PyHookFileHandle()
        {
            if (0 != handle)
                dlclose(handle);
        }
    };
}

std::shared_ptr<Sequence<std::shared_ptr<HookFile> > >
Hooker::_find_hooks(const Hook & hook) const
{
    std::map<std::string, std::shared_ptr<HookFile> > hook_files;
    std::set<std::string> ignore_hooks;
    tokenise<delim_kind::AnyOfTag, delim_mode::DelimiterTag>(getenv_with_default(env_vars::ignore_hooks_named, ""),
            ":", "", std::inserter(ignore_hooks, ignore_hooks.begin()));

    {
        _imp->need_auto_hook_files();
        std::map<std::string, std::map<std::string, std::shared_ptr<HookFile> > >::const_iterator h(
                _imp->auto_hook_files.find(hook.name()));
        if (_imp->auto_hook_files.end() != h)
            hook_files = h->second;
    }

    /* named subdirectories */
    for (std::list<std::pair<FSPath, bool> >::const_iterator d(_imp->dirs.begin()), d_end(_imp->dirs.end()) ;
            d != d_end ; ++d)
    {
        if (! (d->first / hook.name()).stat().is_directory())
            continue;

        for (FSIterator e(d->first / hook.name(), { }), e_end ; e != e_end ; ++e)
        {
            if (ignore_hooks.find(e->basename()) != ignore_hooks.end())
                continue;

            if (is_file_with_extension(*e, ".bash", { }))
                if (! hook_files.insert(std::make_pair(strip_trailing_string(e->basename(), ".bash"),
                                std::shared_ptr<HookFile>(std::make_shared<BashHookFile>(*e, d->second, _imp->env)))).second)
                    Log::get_instance()->message("hook.discarding", ll_warning, lc_context) << "Discarding hook file '" << *e
                        << "' because of naming conflict with '" <<
                        hook_files.find(stringify(strip_trailing_string(e->basename(), ".bash")))->second->file_name() << "'";

            if (is_file_with_extension(*e, ".hook", { }))
                if (! hook_files.insert(std::make_pair(strip_trailing_string(e->basename(), ".hook"),
                                std::shared_ptr<HookFile>(std::make_shared<FancyHookFile>(*e, d->second, _imp->env)))).second)
                    Log::get_instance()->message("hook.discarding", ll_warning, lc_context) << "Discarding hook file '" << *e
                        << "' because of naming conflict with '" <<
                        hook_files.find(stringify(strip_trailing_string(e->basename(), ".hook")))->second->file_name() << "'";

            if (is_file_with_extension(*e, so_suffix, { }))
                 if (! hook_files.insert(std::make_pair(strip_trailing_string(e->basename(), so_suffix),
                                 std::shared_ptr<HookFile>(std::make_shared<SoHookFile>(*e, d->second, _imp->env)))).second)
                     Log::get_instance()->message("hook.discarding", ll_warning, lc_context) << "Discarding hook file '" << *e
                         << "' because of naming conflict with '" <<
                         hook_files.find(stringify(strip_trailing_string(e->basename(), so_suffix)))->second->file_name() << "'";

#ifdef ENABLE_PYTHON_HOOKS
            if (is_file_with_extension(*e, ".py", { }))
            {
                static bool load_try(false);
                static bool load_ok(false);

                {
                    std::unique_lock<std::mutex> lock(PyHookFileHandle::get_instance()->mutex);

                    if (! load_try)
                    {
                        load_try = true;
                        std::string soname("libpaludispythonhooks_" + stringify(PALUDIS_PC_SLOT) + ".so");

                        PyHookFileHandle::get_instance()->handle = dlopen(soname.c_str(), RTLD_NOW | RTLD_GLOBAL);
                        if (PyHookFileHandle::get_instance()->handle)
                        {
                            PyHookFileHandle::get_instance()->create_py_hook_file_handle =
                                reinterpret_cast<std::shared_ptr<HookFile> (*)(
                                        const FSPath &, const bool, const Environment * const)>(
                                            reinterpret_cast<uintptr_t>(dlsym(
                                                    PyHookFileHandle::get_instance()->handle, "create_py_hook_file")));
                            if (PyHookFileHandle::get_instance()->create_py_hook_file_handle)
                            {
                                load_ok = true;
                            }
                            else
                            {
                                Log::get_instance()->message("hook.python.dlerror", ll_warning, lc_context) <<
                                    "dlsym(" + soname + ", create_py_hook_file) "
                                    "failed due to error '" << dlerror() << "'";
                            }
                        }
                        else
                        {
                            Log::get_instance()->message("hook.python.dlerror", ll_warning, lc_context) <<
                                "dlopen(" + soname + ") failed due to error '" << dlerror() << "'";
                        }
                    }
                }
                if (load_ok)
                {
                    if (! hook_files.insert(std::make_pair(strip_trailing_string(e->basename(), ".py"),
                                    std::shared_ptr<HookFile>(PyHookFileHandle::get_instance()->create_py_hook_file_handle(
                                             *e, d->second, _imp->env)))).second)
                        Log::get_instance()->message("hook.discarding", ll_warning, lc_context) <<
                            "Discarding hook file '" << *e
                            << "' because of naming conflict with '"
                            << hook_files.find(stringify(strip_trailing_string(e->basename(), ".py")))->second->file_name()
                            << "'";
                }
            }
#elif ENABLE_PYTHON
            if (is_file_with_extension(*e, ".py", { }))
            {
                Log::get_instance()->message("hook.python.ignoring", ll_warning, lc_context) << "Ignoring hook '" << *e << "' because"
                    << " Paludis was built using a dev-libs/boost version older than 1.34.0.";
            }
#else
            if (is_file_with_extension(*e, ".py", { }))
            {
                Log::get_instance()->message("hook.python.ignoring", ll_warning, lc_context) << "Ignoring hook '" << *e << "' because"
                    << " Paludis was built without Python support (also needs >=dev-libs/boost-1.34.0).";
            }
#endif
        }
    }

    DirectedGraph<std::string, int> hook_deps;
    {
        Context context_local("When determining hook dependencies for '" + hook.name() + "':");
        for (std::map<std::string, std::shared_ptr<HookFile> >::const_iterator f(hook_files.begin()), f_end(hook_files.end()) ;
                f != f_end ; ++f)
            hook_deps.add_node(f->first);

        for (std::map<std::string, std::shared_ptr<HookFile> >::const_iterator f(hook_files.begin()), f_end(hook_files.end()) ;
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
            Log::get_instance()->message("hook.cycles", ll_warning, lc_context) << "Could not resolve dependency order for hook '"
                << hook.name() << "' due to exception '" << e.message() << "' (" << e.what() << "'), skipping hooks '" <<
                join(e.remaining_nodes()->begin(), e.remaining_nodes()->end(), "', '") << "' and using hooks '" << join(ordered.begin(),
                        ordered.end(), "', '") << "' in that order";
        }
    }

    std::shared_ptr<Sequence<std::shared_ptr<HookFile> > > result(std::make_shared<Sequence<std::shared_ptr<HookFile> >>());
    for (std::list<std::string>::const_iterator o(ordered.begin()), o_end(ordered.end()) ;
            o != o_end ; ++o)
        result->push_back(hook_files.find(*o)->second);

    return result;
}

HookResult
Hooker::perform_hook(
        const Hook & hook,
        const std::shared_ptr<OutputManager> & optional_output_manager) const
{
    HookResult result(make_named_values<HookResult>(n::max_exit_status() = 0, n::output() = ""));

    Context context("When triggering hook '" + hook.name() + "':");
    Log::get_instance()->message("hook.starting", ll_debug, lc_no_context) << "Starting hook '" << hook.name() << "'";

    /* repo hooks first */

    do
    {
        switch (hook.output_dest)
        {
            case hod_stdout:
                for (auto r(_imp->env->begin_repositories()),
                        r_end(_imp->env->end_repositories()) ; r != r_end ; ++r)
                    result.max_exit_status() = std::max(result.max_exit_status(),
                            ((*r)->perform_hook(hook, optional_output_manager)).max_exit_status());
                continue;

            case hod_grab:
                for (auto r(_imp->env->begin_repositories()),
                        r_end(_imp->env->end_repositories()) ; r != r_end ; ++r)
                {
                    HookResult tmp((*r)->perform_hook(hook, optional_output_manager));
                    if (tmp.max_exit_status() > result.max_exit_status())
                        result = tmp;
                    else if (! tmp.output().empty())
                    {
                        if (hook.validate_value(tmp.output()))
                        {
                            if (result.max_exit_status() == 0)
                                return tmp;
                        }
                        else
                            Log::get_instance()->message("hook.bad_output", ll_warning, lc_context)
                                << "Hook returned invalid output: '" << tmp.output() << "'";
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

    std::unique_lock<std::recursive_mutex> l(_imp->hook_files_mutex);
    std::map<std::string, std::shared_ptr<Sequence<std::shared_ptr<HookFile> > > >::iterator h(_imp->hook_files.find(hook.name()));

    if (h == _imp->hook_files.end())
        h = _imp->hook_files.insert(std::make_pair(hook.name(), _find_hooks(hook))).first;

    if (! h->second->empty())
    {
        do
        {
            switch (hook.output_dest)
            {
                case hod_stdout:
                    for (Sequence<std::shared_ptr<HookFile> >::ConstIterator f(h->second->begin()),
                            f_end(h->second->end()) ; f != f_end ; ++f)
                        if ((*f)->file_name().stat().is_regular_file_or_symlink_to_regular_file())
                            result.max_exit_status() = std::max(result.max_exit_status(), (*f)->run(hook, optional_output_manager).max_exit_status());
                        else
                            Log::get_instance()->message("hook.not_regular_file", ll_warning, lc_context) << "Hook file '" <<
                                (*f)->file_name() << "' is not a regular file or has been removed";
                    continue;

                case hod_grab:
                    for (Sequence<std::shared_ptr<HookFile> >::ConstIterator f(h->second->begin()),
                            f_end(h->second->end()) ; f != f_end ; ++f)
                    {
                        if (! (*f)->file_name().stat().is_regular_file_or_symlink_to_regular_file())
                        {
                            Log::get_instance()->message("hook.not_regular_file", ll_warning, lc_context) << "Hook file '" <<
                                (*f)->file_name() << "' is not a regular file or has been removed";
                            continue;
                        }

                        HookResult tmp((*f)->run(hook, optional_output_manager));
                        if (tmp.max_exit_status() > result.max_exit_status())
                            result = tmp;
                        else if (! tmp.output().empty())
                        {
                            if (hook.validate_value(tmp.output()))
                            {
                                if (result.max_exit_status() == 0)
                                    return tmp;
                            }
                            else
                                Log::get_instance()->message("hook.bad_output", ll_warning, lc_context)
                                    << "Hook returned invalid output: '" << tmp.output() << "'";
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

namespace paludis
{
    template class Sequence<std::shared_ptr<HookFile> >;
}
