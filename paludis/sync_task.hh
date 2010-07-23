/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_SYNC_TASK_HH
#define PALUDIS_GUARD_PALUDIS_SYNC_TASK_HH 1

#include <paludis/util/pimp.hh>
#include <paludis/util/wrapped_forward_iterator-fwd.hh>
#include <paludis/dep_list.hh>

/** \file
 * Declarations for SyncTask.
 *
 * \ingroup g_tasks
 *
 * \section Examples
 *
 * - None at this time.
 */

namespace paludis
{
    class Environment;
    class SyncFailedError;

    /**
     * Task to handle syncing some or all repositories.
     *
     * \ingroup g_tasks
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE SyncTask :
        private Pimp<SyncTask>
    {
        protected:
            ///\name Basic operations
            ///\{

            SyncTask(Environment * const env, const bool parallel);

            ///\}

        public:
            ///\name Basic operations
            ///\{

            virtual ~SyncTask();

            SyncTask(const SyncTask &) = delete;
            SyncTask & operator= (const SyncTask &) = delete;

            ///\}

            ///\name Add targets
            ///\{

            void add_target(const std::string &);

            ///\}

            ///\name Event callbacks
            ///\{

            virtual void on_sync_all_pre() = 0;
            virtual void on_sync_pre(const RepositoryName &) = 0;
            virtual void on_sync_post(const RepositoryName &) = 0;
            virtual void on_sync_skip(const RepositoryName &) = 0;
            virtual void on_sync_fail(const RepositoryName &, const SyncFailedError &) = 0;
            virtual void on_sync_succeed(const RepositoryName &) = 0;
            virtual void on_sync_all_post() = 0;

            virtual void on_sync_status(const int x, const int y, const int a) = 0;

            ///\}

            ///\name Target iteration
            ///\{

            struct TargetsConstIteratorTag;
            typedef WrappedForwardIterator<TargetsConstIteratorTag, const RepositoryName> TargetsConstIterator;
            TargetsConstIterator begin_targets() const;
            TargetsConstIterator end_targets() const;

            ///\}

            /**
             * Run the task.
             */
            virtual void execute();
    };
}

#endif
