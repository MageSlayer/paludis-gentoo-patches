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
#include <paludis/output_manager.hh>
#include <paludis/repository.hh>
#include <paludis/environment.hh>
#include <paludis/hook.hh>
#include <paludis/syncer.hh>
#include <cstdlib>
#include <iostream>
#include <map>
#include <set>

#include "command_command_line.hh"

using namespace paludis;
using namespace cave;
using std::cout;
using std::endl;

namespace paludis
{
    namespace n
    {
        struct output_manager;
        struct success;
        struct summary;
    }
}

namespace
{
    struct TailAndLogOutputManager :
        OutputManager
    {
        TailAndLogOutputManager(const std::string &)
        {
        }

        virtual std::ostream & stdout_stream()
        {
            return std::cout;
        }

        virtual std::ostream & stderr_stream()
        {
            return std::cerr;
        }

        std::tr1::shared_ptr<const Sequence<std::string> > tail(const bool) const
        {
            return make_shared_ptr(new Sequence<std::string>);
        }

        void discard_log()
        {
        }

        const FSEntry log_file_name() const
        {
            return FSEntry("/dev/null");
        }

        void succeeded()
        {
        }

        virtual void message(const MessageType, const std::string &)
        {
        }
    };

    struct SyncCommandLine :
        CaveCommandCommandLine
    {
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

        SyncCommandLine()
        {
            add_usage_line("[repository ...]");
        }
    };

    struct Message
    {
        NamedValue<n::output_manager, std::tr1::shared_ptr<TailAndLogOutputManager> > output_manager;
        NamedValue<n::success, bool> success;
        NamedValue<n::summary, std::string> summary;
    };

    typedef std::map<std::string, Message> Messages;

    void do_one_sync_notifier(const RepositoryName & r, Mutex & notifier_mutex,
            Mutex & count_mutex, ConditionVariable & notifier_condition, int & np, int & na, int & nd,
            bool & finished, TailAndLogOutputManager & output_manager)
    {
        bool first(true);
        while (true)
        {
            {
                Lock lock(count_mutex);
                if (finished)
                    return;

                if (! first)
                {
                    cout << format_general_spad(f::sync_repo_active(), stringify(r), np, na, nd);
                    std::tr1::shared_ptr<const Sequence<std::string> > tail(output_manager.tail(true));
                    if (tail && tail->begin() != tail->end())
                    {
                        for (Sequence<std::string>::ConstIterator t(tail->begin()), t_end(tail->end()) ;
                                t != t_end ; ++t)
                            cout << format_general_s(f::sync_repo_tail(), *t);
                    }
                }
            }

            Lock lock(notifier_mutex);
            notifier_condition.timed_wait(notifier_mutex, 10);
            first = false;
        }
    }

    void do_one_sync(const std::tr1::shared_ptr<Environment> & env, const RepositoryName & r, Mutex & mutex,
            Messages & messages, int & retcode, int & np, int & na, int & nd)
    {
        std::tr1::shared_ptr<TailAndLogOutputManager> output_manager(
                new TailAndLogOutputManager("sync-" + stringify(r)));

        bool done_decrement(false);

        try
        {
            {
                Lock lock(mutex);
                ++na;
                --np;
                cout << format_general_spad(f::sync_repo_starting(), stringify(r), np, na, nd);
                if (0 !=
                        env->perform_hook(Hook("sync_pre")
                            ("TARGET", stringify(r))
                            ("NUMBER_DONE", stringify(nd))
                            ("NUMBER_ACTIVE", stringify(na))
                            ("NUMBER_PENDING", stringify(np))
                            ).max_exit_status())
                    throw SyncFailedError("Sync of '" + stringify(r) + "' aborted by hook");
            }

            const std::tr1::shared_ptr<const Repository> repo(env->package_database()->fetch_repository(r));
            bool result(false);
            {
                Mutex notifier_mutex;
                ConditionVariable notifier_condition;
                bool finished(false);
                Thread notifier_thread(std::tr1::bind(&do_one_sync_notifier, r,
                            std::tr1::ref(notifier_mutex), std::tr1::ref(mutex),
                            std::tr1::ref(notifier_condition),
                            std::tr1::ref(np), std::tr1::ref(na), std::tr1::ref(nd),
                            std::tr1::ref(finished), std::tr1::ref(*output_manager)));

                try
                {
                    result = repo->sync(output_manager);

                    {
                        Lock lock(mutex);
                        finished = true;
                    }
                }
                catch (...)
                {
                    {
                        Lock lock(mutex);
                        finished = true;
                    }
                    notifier_condition.acquire_then_signal(notifier_mutex);
                    throw;
                }
                notifier_condition.acquire_then_signal(notifier_mutex);
            }

            {
                Lock lock(mutex);
                ++nd;
                --na;
                done_decrement = true;

                if (0 !=
                        env->perform_hook(Hook("sync_post")
                            ("TARGET", stringify(r))
                            ("NUMBER_DONE", stringify(nd))
                            ("NUMBER_ACTIVE", stringify(na))
                            ("NUMBER_PENDING", stringify(np))
                            ).max_exit_status())
                    throw SyncFailedError("Sync of '" + stringify(r) + "' aborted by hook");
            }

            if (result)
            {
                Lock lock(mutex);
                messages.insert(make_pair(stringify(r), make_named_values<Message>(
                                value_for<n::output_manager>(output_manager),
                                value_for<n::success>(true),
                                value_for<n::summary>("success")
                                )));
                cout << format_general_spad(f::sync_repo_done_success(), stringify(r), np, na, nd);
            }
            else
            {
                Lock lock(mutex);
                messages.insert(make_pair(stringify(r), make_named_values<Message>(
                                value_for<n::output_manager>(output_manager),
                                value_for<n::success>(true),
                                value_for<n::summary>("no syncing required")
                                )));
                cout << format_general_spad(f::sync_repo_done_no_syncing_required(), stringify(r), np, na, nd);
            }
        }
        catch (const Exception & e)
        {
            Lock lock(mutex);

            if (! done_decrement)
            {
                --na;
                ++nd;
                done_decrement = true;
            }

            retcode |= 1;
            messages.insert(make_pair(stringify(r), make_named_values<Message>(
                            value_for<n::output_manager>(output_manager),
                            value_for<n::success>(false),
                            value_for<n::summary>(e.message() + " (" + e.what() + ")")
                            )));

            cout << format_general_spad(f::sync_repo_done_failure(), stringify(r), np, na, nd);
            std::tr1::shared_ptr<const Sequence<std::string> > tail(output_manager->tail(true));
            if (tail && tail->begin() != tail->end())
            {
                for (Sequence<std::string>::ConstIterator t(tail->begin()), t_end(tail->end()) ;
                        t != t_end ; ++t)
                    cout << format_general_s(f::sync_repo_tail(), *t);
            }

            int PALUDIS_ATTRIBUTE((unused)) ignore(env->perform_hook(Hook("sync_fail")
                        ("TARGET", stringify(r))
                        ("NUMBER_DONE", stringify(nd))
                        ("NUMBER_ACTIVE", stringify(na))
                        ("NUMBER_PENDING", stringify(np))
                        ).max_exit_status());
        }
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
    Messages messages;

    std::set<RepositoryName, RepositoryNameComparator> repos;
    if (cmdline.begin_parameters() != cmdline.end_parameters())
        for (SyncCommandLine::ParametersConstIterator p(cmdline.begin_parameters()), p_end(cmdline.end_parameters()) ;
                p != p_end ; ++p)
            repos.insert(RepositoryName(*p));
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

    {
        Mutex mutex;
        int active(0), done(0), pending(repos.size());

        ActionQueue actions(5);
        for (std::set<RepositoryName, RepositoryNameComparator>::const_iterator r(repos.begin()), r_end(repos.end()) ;
                r != r_end ; ++r)
            actions.enqueue(std::tr1::bind(&do_one_sync, env, *r, std::tr1::ref(mutex),
                        std::tr1::ref(messages), std::tr1::ref(retcode), std::tr1::ref(pending),
                        std::tr1::ref(active), std::tr1::ref(done)));
    }

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

    cout << endl << format_general_s(f::sync_heading(), "Sync results");

    for (Messages::const_iterator m(messages.begin()), m_end(messages.end()) ;
            m != m_end ; ++m)
    {
        if (m->second.success())
        {
            cout << format_general_kv(f::sync_message_success(), m->first, m->second.summary());
            m->second.output_manager()->discard_log();
        }
        else
        {
            cout << format_general_kv(f::sync_message_failure(), m->first, m->second.summary());
            cout << format_general_kv(f::sync_message_failure_message(), "Log file", stringify(m->second.output_manager()->log_file_name()));
        }
    }
    cout << endl;

    return retcode;
}

std::tr1::shared_ptr<args::ArgsHandler>
SyncCommand::make_doc_cmdline()
{
    return make_shared_ptr(new SyncCommandLine);
}

