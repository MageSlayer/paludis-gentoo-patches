/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013 Ciaran McCreesh
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
#include "colours.hh"
#include "format_user_config.hh"
#include <paludis/util/named_value.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/return_literal_function.hh>
#include <paludis/util/executor.hh>
#include <paludis/util/timestamp.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/join.hh>
#include <paludis/output_manager.hh>
#include <paludis/standard_output_manager.hh>
#include <paludis/repository.hh>
#include <paludis/environment.hh>
#include <paludis/hook.hh>
#include <paludis/syncer.hh>
#include <paludis/metadata_key.hh>
#include <paludis/create_output_manager_info.hh>
#include <functional>
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
#include "cmd_sync-fmt.hh"

    typedef std::set<RepositoryName> Repos;

    struct SyncCommandLine :
        CaveCommandCommandLine
    {
        args::ArgsGroup g_job_options;
        args::SwitchArg a_sequential;

        args::ArgsGroup g_sync_options;
        args::StringArg a_source;
        args::AliasArg a_suffix;
        args::StringArg a_revision;

        std::string app_name() const override
        {
            return "cave sync";
        }

        std::string app_synopsis() const override
        {
            return "Sync all or specified repositories.";
        }

        std::string app_description() const override
        {
            return "Syncs repositories. If any repository names are specified, these repositories "
                "are synced. Otherwise, all syncable repositories are synced.";
        }

        SyncCommandLine() :
            g_job_options(main_options_section(), "Job Options", "Job options."),
            a_sequential(&g_job_options, "sequential", '\0', "Only perform one sync at a time.", false),

            g_sync_options(main_options_section(), "Sync Options", "Sync options."),
            a_source(&g_sync_options, "source", 's', "Use the specified source for syncing."),
            a_suffix(&a_source, "suffix", true),
            a_revision(&g_sync_options, "revision", 'r', "Sync to the specified revision. Not supported by all "
                    "syncers. Probably doesn't make sense when not specified with a repository parameter.")
        {
            add_usage_line("[ --sequential ] [repository ...]");
        }
    };

    struct SyncExecutive :
        Executive
    {
        bool abort;
        bool success;
        bool skipped;
        std::string error;

        const std::shared_ptr<Environment> env;
        const SyncCommandLine & cmdline;
        const Executor * const executor;
        const RepositoryName name;

        Timestamp last_flushed, last_output;

        std::shared_ptr<OutputManager> output_manager;

        SyncExecutive(
                const std::shared_ptr<Environment> & e,
                const SyncCommandLine & c,
                const Executor * const x,
                const RepositoryName & n) :
            abort(false),
            success(false),
            skipped(false),
            env(e),
            cmdline(c),
            executor(x),
            name(n),
            last_flushed(Timestamp::now()),
            last_output(last_flushed)
        {
        }

        std::string queue_name() const override
        {
            /* if we're sequential, there's just one queue */
            if (cmdline.a_sequential.specified())
                return "";

            const std::shared_ptr<const Repository> r(env->fetch_repository(name));

            if (r->sync_host_key())
            {
                auto sync_host(r->sync_host_key()->parse_value());
                if (sync_host->end() != sync_host->find(cmdline.a_source.argument()))
                    return sync_host->find(cmdline.a_source.argument())->second;
            }

            return "";
        }

        std::string unique_id() const override
        {
            return stringify(name);
        }

        bool can_run() const override
        {
            return true;
        }

        void pre_execute_exclusive() override
        {
            try
            {
                cout << fuc(fs_repo_starting(), fv<'s'>(stringify(name)), fv<'p'>(stringify(executor->pending())),
                        fv<'a'>(stringify(executor->active())), fv<'d'>(stringify(executor->done())));
                output_manager = std::make_shared<StandardOutputManager>();

                if (0 != env->perform_hook(Hook("sync_pre")
                            ("TARGET", stringify(name))
                            ("NUMBER_DONE", stringify(executor->done()))
                            ("NUMBER_ACTIVE", stringify(executor->active()))
                            ("NUMBER_PENDING", stringify(executor->pending())),
                            nullptr).max_exit_status())
                    throw SyncFailedError("Sync aborted by hook");

                const std::shared_ptr<Repository> repo(env->fetch_repository(name));
                CreateOutputManagerForRepositorySyncInfo info(repo->name(),
                        cmdline.a_sequential.specified() ? oe_exclusive : oe_with_others,
                        ClientOutputFeatures() + cof_summary_at_end);
                output_manager = env->create_output_manager(info);

                last_flushed = Timestamp::now();
                last_output = last_flushed;
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

        void execute_threaded() override
        {
            if (abort)
                return;

            try
            {
                const std::shared_ptr<Repository> repo(env->fetch_repository(name));

                if (! repo->sync(cmdline.a_source.argument(), cmdline.a_revision.argument(), output_manager))
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

        void display_active()
        {
            if (output_manager)
            {
                cout << fuc(fs_repo_active(), fv<'s'>(stringify(name)), fv<'p'>(stringify(executor->pending())),
                        fv<'a'>(stringify(executor->active())), fv<'d'>(stringify(executor->done())));
                if (output_manager->want_to_flush())
                {
                    cout << endl;
                    output_manager->flush();
                    cout << endl;
                    last_output = Timestamp::now();
                }
                else if (! cmdline.a_sequential.specified())
                    cout << fuc(fs_repo_active_quiet(),
                            fv<'s'>(stringify(Timestamp::now().seconds() - last_output.seconds())));

                last_flushed = Timestamp::now();
            }
        }

        void flush_threaded() override
        {
            if (output_manager->want_to_flush())
                display_active();
            else
            {

                Timestamp now(Timestamp::now());
                if (now.seconds() - last_flushed.seconds() >= 10)
                    display_active();
            }
        }

        void post_execute_exclusive() override
        {
            try
            {
                if (output_manager->want_to_flush())
                    display_active();

                if (! abort)
                {
                    if (0 != env->perform_hook(Hook(success ? "sync_post" : "sync_fail")
                                ("TARGET", stringify(name))
                                ("NUMBER_DONE", stringify(executor->done()))
                                ("NUMBER_ACTIVE", stringify(executor->active()))
                                ("NUMBER_PENDING", stringify(executor->pending())),
                                nullptr).max_exit_status())
                        throw SyncFailedError("Sync aborted by hook");
                }

                output_manager->nothing_more_to_come();

                if (skipped)
                    cout << fuc(fs_repo_done_no_syncing_required(), fv<'s'>(stringify(name)), fv<'p'>(stringify(executor->pending())),
                            fv<'a'>(stringify(executor->active())), fv<'d'>(stringify(executor->done())));
                else if (! success)
                    cout << fuc(fs_repo_done_failure(), fv<'s'>(stringify(name)), fv<'p'>(stringify(executor->pending())),
                            fv<'a'>(stringify(executor->active())), fv<'d'>(stringify(executor->done())));
                else
                    cout << fuc(fs_repo_done_success(), fv<'s'>(stringify(name)), fv<'p'>(stringify(executor->pending())),
                            fv<'a'>(stringify(executor->active())), fv<'d'>(stringify(executor->done())));
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
            const std::shared_ptr<Environment> & env,
            const SyncCommandLine & cmdline,
            const Repos & repos)
    {
        std::list<std::shared_ptr<SyncExecutive> > executives;

        {
            Executor executor;

            for (const auto & repo : repos)
            {
                const std::shared_ptr<SyncExecutive> x(std::make_shared<SyncExecutive>(env, cmdline, &executor, repo));
                executor.add(x);
                executives.push_back(x);
            }

            executor.execute();
        }

        int retcode(0);

        cout << fuc(fs_heading(), fv<'s'>("Sync results"));
        for (const auto & executive : executives)
        {
            if (! executive->success)
            {
                retcode |= 1;
                cout << fuc(fs_message_failure(), fv<'k'>(stringify(executive->name)), fv<'v'>("failed"));
                cout << fuc(fs_message_failure_message(), fv<'s'>(executive->error));
            }
            else
            {
                executive->output_manager->succeeded();
                if (executive->skipped)
                    cout << fuc(fs_message_success(), fv<'k'>(stringify(executive->name)), fv<'v'>("no syncing required"));
                else
                    cout << fuc(fs_message_success(), fv<'k'>(stringify(executive->name)), fv<'v'>("success"));
            }

            executive->output_manager.reset();
        }

        return retcode;
    }
}

int
SyncCommand::run(
        const std::shared_ptr<Environment> & env,
        const std::shared_ptr<const Sequence<std::string > > & args
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
        for (const auto & param : cmdline.parameters())
        {
            RepositoryName n(param);
            if (! env->has_repository_named(n))
                throw NothingMatching(param);
            repos.insert(n);
        }
    else
        for (const auto & repository : env->repositories())
            repos.insert(repository->name());

    if (1 == repos.size())
        cmdline.a_sequential.set_specified(args::aos_weak);

    cout << fuc(fs_heading(), fv<'s'>("Starting sync"));

    if (0 != env->perform_hook(Hook("sync_all_pre")
                ("TARGETS", join(repos.begin(), repos.end(), " ")),
                nullptr).max_exit_status())
        throw SyncFailedError("Sync aborted by hook");

    cout << fuc(fs_repos_title());

    retcode |= sync_these(env, cmdline, repos);

    for (const auto & repository : env->repositories())
    {
        repository->invalidate();
        repository->purge_invalid_cache();
    }

    if (0 != env->perform_hook(Hook("sync_all_post")
                ("TARGETS", join(repos.begin(), repos.end(), " ")),
                nullptr).max_exit_status())
        throw SyncFailedError("Sync aborted by hook");

    return retcode;
}

std::shared_ptr<args::ArgsHandler>
SyncCommand::make_doc_cmdline()
{
    return std::make_shared<SyncCommandLine>();
}

CommandImportance
SyncCommand::importance() const
{
    return ci_core;
}

