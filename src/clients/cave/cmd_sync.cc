/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009 Ciaran McCreesh
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

#include "cmd_sync.hh"
#include "exceptions.hh"
#include "formats.hh"
#include "format_general.hh"
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/package_database.hh>
#include <paludis/util/action_queue.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/named_value.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/condition_variable.hh>
#include <paludis/util/thread.hh>
#include <paludis/util/return_literal_function.hh>
#include <paludis/output_manager.hh>
#include <paludis/repository.hh>
#include <paludis/environment.hh>
#include <paludis/hook.hh>
#include <paludis/syncer.hh>
#include <paludis/metadata_key.hh>
#include <paludis/create_output_manager_info.hh>
#include <tr1/functional>
#include <cstdlib>
#include <iostream>
#include <list>
#include <map>
#include <set>

#include "command_command_line.hh"

using namespace paludis;
using namespace cave;
using std::cout;
using std::endl;

namespace
{
    typedef std::set<RepositoryName, RepositoryNameComparator> Repos;

    struct SyncCommandLine :
        CaveCommandCommandLine
    {
        args::ArgsGroup g_job_options;
        args::SwitchArg a_sequential;

        virtual std::string app_name() const
        {
            return "cave sync";
        }

        virtual std::string app_synopsis() const
        {
            return "Sync all or specified repositories.";
        }

        virtual std::string app_description() const
        {
            return "Syncs repositories. If any repository names are specified, these repositories "
                "are synced. Otherwise, all syncable repositories are synced.";
        }

        SyncCommandLine() :
            g_job_options(main_options_section(), "Job Options", "Job options."),
            a_sequential(&g_job_options, "sequential", '\0', "Only perform one sync at a time.", false)
        {
            add_usage_line("[ --sequential ] [repository ...]");
        }
    };

    std::string get_queue_name_func(
            const std::tr1::shared_ptr<const Environment> & env,
            const RepositoryName & r)
    {
        const std::tr1::shared_ptr<const Repository> repo(env->package_database()->fetch_repository(r));
        if (repo->sync_host_key())
            return repo->sync_host_key()->value();
        else
            return "(no host)";
    }

    struct Executive
    {
        virtual std::string queue_name() const = 0;
        virtual std::string unique_id() const = 0;
        virtual bool can_run() const = 0;

        virtual void pre_execute_exclusive() = 0;
        virtual void execute_threaded() = 0;
        virtual void post_execute_exclusive() = 0;
    };

    class Executor
    {
        private:
            int _pending;
            int _active;
            int _done;

            typedef std::multimap<std::string, std::tr1::shared_ptr<Executive> > Queues;
            typedef std::list<std::tr1::shared_ptr<Executive> > ReadyForPost;
            Queues _queues;
            ReadyForPost _ready_for_post;
            Mutex _mutex;
            ConditionVariable _condition;

            void _one(const std::tr1::shared_ptr<Executive> executive)
            {
                executive->execute_threaded();

                Lock lock(_mutex);
                _ready_for_post.push_back(executive);
                _condition.signal();
            }

        public:
            Executor() :
                _pending(0),
                _active(0),
                _done(0)
            {
            }

            int pending() const
            {
                return _pending;
            }

            int active() const
            {
                return _active;
            }

            int done() const
            {
                return _done;
            }

            void add(const std::tr1::shared_ptr<Executive> & x)
            {
                ++_pending;
                _queues.insert(std::make_pair(x->queue_name(), x));
            }

            void execute()
            {
                typedef std::map<std::string, std::tr1::shared_ptr<Thread> > Running;
                Running running;

                Lock lock(_mutex);
                while (true)
                {
                    bool any(false);
                    for (Queues::iterator q(_queues.begin()), q_end(_queues.end()) ;
                            q != q_end ; )
                    {
                        if ((running.end() != running.find(q->first)) || ! q->second->can_run())
                        {
                            ++q;
                            continue;
                        }

                        ++_active;
                        --_pending;
                        q->second->pre_execute_exclusive();
                        running.insert(std::make_pair(q->first, make_shared_ptr(new Thread(
                                            std::tr1::bind(&Executor::_one, this, q->second)))));
                        _queues.erase(q++);
                        any = true;
                    }

                    if ((! any) && running.empty())
                        break;

                    _condition.wait(_mutex);

                    for (ReadyForPost::iterator p(_ready_for_post.begin()), p_end(_ready_for_post.end()) ;
                            p != p_end ; ++p)
                    {
                        --_active;
                        ++_done;
                        running.erase((*p)->queue_name());
                        (*p)->post_execute_exclusive();
                    }

                    _ready_for_post.clear();
                }
            }
    };

    struct SyncExecutive :
        Executive
    {
        bool abort;
        bool success;
        bool skipped;
        std::string error;

        const std::tr1::shared_ptr<Environment> env;
        const SyncCommandLine & cmdline;
        const Executor * const executor;
        const RepositoryName name;

        std::tr1::shared_ptr<OutputManager> output_manager;

        SyncExecutive(
                const std::tr1::shared_ptr<Environment> & e,
                const SyncCommandLine & c,
                const Executor * const x,
                const RepositoryName & n) :
            abort(false),
            success(false),
            skipped(false),
            env(e),
            cmdline(c),
            executor(x),
            name(n)
        {
        }

        virtual std::string queue_name() const
        {
            const std::tr1::shared_ptr<const Repository> r(env->package_database()->fetch_repository(name));
            if (r->sync_host_key())
                return r->sync_host_key()->value();
            else
                return "";
        }

        virtual std::string unique_id() const
        {
            return stringify(name);
        }

        virtual bool can_run() const
        {
            return true;
        }

        virtual void pre_execute_exclusive()
        {
            try
            {
                cout << format_general_spad(f::sync_repo_starting(), stringify(name), executor->pending(),
                        executor->active(), executor->done());

                if (0 != env->perform_hook(Hook("sync_pre")
                            ("TARGET", stringify(name))
                            ("NUMBER_DONE", stringify(executor->done()))
                            ("NUMBER_ACTIVE", stringify(executor->active()))
                            ("NUMBER_PENDING", stringify(executor->pending()))
                            ).max_exit_status())
                    throw SyncFailedError("Sync aborted by hook");
            }
            catch (const Exception & e)
            {
                abort = true;
                error = e.message();
            }
            catch (...)
            {
                abort = true;
                error = "Caught unknown exception";
            }
        }

        virtual void execute_threaded()
        {
            if (abort)
                return;

            try
            {
                const std::tr1::shared_ptr<Repository> repo(env->package_database()->fetch_repository(name));
                output_manager = env->create_output_manager(
                        CreateOutputManagerForRepositorySyncInfo(*repo,
                            cmdline.a_sequential.specified() ? oe_exclusive : oe_with_others));

                if (! repo->sync(output_manager))
                    skipped = true;
                success = true;
            }
            catch (const SyncFailedError & e)
            {
                error = e.message();
                /* don't abort */
            }
            catch (const Exception & e)
            {
                error = e.message();
                abort = true;
            }
            catch (...)
            {
                error = "Caught unknown exception";
                abort = true;
            }
        }

        virtual void post_execute_exclusive()
        {
            if (abort)
                return;

            try
            {
                if (0 != env->perform_hook(Hook(success ? "sync_post" : "sync_fail")
                            ("TARGET", stringify(name))
                            ("NUMBER_DONE", stringify(executor->done()))
                            ("NUMBER_ACTIVE", stringify(executor->active()))
                            ("NUMBER_PENDING", stringify(executor->pending()))
                            ).max_exit_status())
                    throw SyncFailedError("Sync aborted by hook");

                if (skipped)
                    cout << format_general_spad(f::sync_repo_done_no_syncing_required(), stringify(name),
                            executor->pending(), executor->active(), executor->done());
                else if (! success)
                    cout << format_general_spad(f::sync_repo_done_failure(), stringify(name),
                            executor->pending(), executor->active(), executor->done());
                else
                    cout << format_general_spad(f::sync_repo_done_success(), stringify(name),
                            executor->pending(), executor->active(), executor->done());
            }
            catch (const Exception & e)
            {
                error = e.message();
                abort = true;
                success = false;
            }
            catch (...)
            {
                error = "Caught unknown exception";
                abort = true;
                success = false;
            }
        }
    };

    int sync_these(
            const std::tr1::shared_ptr<Environment> & env,
            const SyncCommandLine & cmdline,
            const Repos & repos)
    {
        std::list<std::tr1::shared_ptr<SyncExecutive> > executives;

        {
            Executor executor;

            for (Repos::const_iterator r(repos.begin()), r_end(repos.end()) ;
                    r != r_end ; ++r)
            {
                const std::tr1::shared_ptr<SyncExecutive> x(new SyncExecutive(env, cmdline, &executor, *r));
                executor.add(x);
                executives.push_back(x);
            }

            executor.execute();
        }

        int retcode(0);

        cout << format_general_s(f::sync_heading(), "Sync results");
        for (std::list<std::tr1::shared_ptr<SyncExecutive> >::const_iterator x(executives.begin()),
                x_end(executives.end()) ;
                x != x_end ; ++x)
        {
            if (! (*x)->success)
            {
                retcode |= 1;
                cout << format_general_kv(f::sync_message_failure(), stringify((*x)->name), "failed");
                cout << format_general_kv(f::sync_message_failure_message(), "error", (*x)->error);
            }
            else if ((*x)->skipped)
                cout << format_general_kv(f::sync_message_success(), stringify((*x)->name), "no syncing required");
            else
                cout << format_general_kv(f::sync_message_success(), stringify((*x)->name), "success");

            (*x)->output_manager.reset();
        }

        return retcode;
    }
}

bool
SyncCommand::important() const
{
    return true;
}

int
SyncCommand::run(
        const std::tr1::shared_ptr<Environment> & env,
        const std::tr1::shared_ptr<const Sequence<std::string > > & args
        )
{
    SyncCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_SYNC_OPTIONS", "CAVE_SYNC_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    int retcode(0);

    Repos repos;
    if (cmdline.begin_parameters() != cmdline.end_parameters())
        for (SyncCommandLine::ParametersConstIterator p(cmdline.begin_parameters()), p_end(cmdline.end_parameters()) ;
                p != p_end ; ++p)
        {
            RepositoryName n(*p);
            if (! env->package_database()->has_repository_named(n))
                throw NothingMatching(*p);
            repos.insert(n);
        }
    else
        for (PackageDatabase::RepositoryConstIterator p(env->package_database()->begin_repositories()),
                p_end(env->package_database()->end_repositories()) ;
                p != p_end ; ++p)
            repos.insert((*p)->name());

    cout << format_general_s(f::sync_heading(), "Starting sync");

    if (0 != env->perform_hook(Hook("sync_all_pre")
                ("TARGETS", join(repos.begin(), repos.end(), " ")
                )).max_exit_status())
        throw SyncFailedError("Sync aborted by hook");

    cout << format_general_s(f::sync_repos_title(), "");

    retcode |= sync_these(env, cmdline, repos);

    for (PackageDatabase::RepositoryConstIterator r(env->package_database()->begin_repositories()),
            r_end(env->package_database()->end_repositories()) ; r != r_end ; ++r)
    {
        (*r)->invalidate();
        (*r)->purge_invalid_cache();
    }

    if (0 != env->perform_hook(Hook("sync_all_post")
                ("TARGETS", join(repos.begin(), repos.end(), " ")
                )).max_exit_status())
        throw SyncFailedError("Sync aborted by hook");

    return retcode;
}

std::tr1::shared_ptr<args::ArgsHandler>
SyncCommand::make_doc_cmdline()
{
    return make_shared_ptr(new SyncCommandLine);
}

