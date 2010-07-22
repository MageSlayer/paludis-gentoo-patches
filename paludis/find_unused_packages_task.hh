/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Danny van Dyk
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

#ifndef PALUDIS_GUARD_PALUDIS_FIND_UNUSED_PACKAGES_TASK_HH
#define PALUDIS_GUARD_PALUDIS_FIND_UNUSED_PACKAGES_TASK_HH 1

#include <paludis/package_id-fwd.hh>
#include <paludis/name-fwd.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/repository-fwd.hh>

/** \file
 * Declarations for FindUnusedPackagesTask.
 *
 * \ingroup g_tasks
 *
 * \section Examples
 *
 * - None at this time.
 */

namespace paludis
{
    /**
     * Task to find unused package versions for a given package name.
     *
     * \ingroup g_tasks
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE FindUnusedPackagesTask
    {
        private:
            const Environment * const _env;
            const Repository * const _repo;

        public:
            ///\name Basic operations
            ///\{

            FindUnusedPackagesTask(const Environment * const env, const Repository * const repo) :
                _env(env),
                _repo(repo)
            {
            }

            virtual ~FindUnusedPackagesTask();

            FindUnusedPackagesTask(const FindUnusedPackagesTask &) = delete;
            FindUnusedPackagesTask & operator= (const FindUnusedPackagesTask &) = delete;

            ///\}

            /**
             * Run the task.
             */
            std::shared_ptr<const PackageIDSequence> execute(const QualifiedPackageName &);
    };
}

#endif
