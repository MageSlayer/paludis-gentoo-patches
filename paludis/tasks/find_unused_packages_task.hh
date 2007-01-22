/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Danny van Dyk <kugelfang@gentoo.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_TASKS_FIND_UNUSED_PACKAGES_TASK_HH
#define PALUDIS_GUARD_PALUDIS_TASKS_FIND_UNUSED_PACKAGES_TASK_HH 1

#include <paludis/util/instantiation_policy.hh>
#include <paludis/package_database.hh>

namespace paludis
{
    class Environment;
    class Repository;

    /**
     * Task to find unused package versions for a given package name.
     *
     * \ingroup grptasks
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE FindUnusedPackagesTask :
        InstantiationPolicy<FindUnusedPackagesTask, instantiation_method::NonCopyableTag>
    {
        private:
            /// Our environment
            const Environment * const _env;

            /// Our repository
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

            ///\}

            /**
             * Run the task.
             */
            PackageDatabaseEntryCollection::ConstPointer execute(const QualifiedPackageName &);
    };
}

#endif
