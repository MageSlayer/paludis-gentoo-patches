/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Fernando J. Pereda <ferdy@gentoo.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_TASKS_REPORT_TASK_HH
#define PALUDIS_GUARD_PALUDIS_TASKS_REPORT_TASK_HH 1

#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/repository.hh>

namespace paludis
{
    class Environment;

    /**
     * Task to report the current state of the system.
     *
     * \ingroup grptasks
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE ReportTask :
        PrivateImplementationPattern<ReportTask>,
        InstantiationPolicy<ReportTask, instantiation_method::NonCopyableTag>
    {
        protected:
            ///\name Basic operations
            ///\{

            ReportTask(Environment * const env);

            ///\}

        public:
            ///\name Basic operations
            ///\{

            virtual ~ReportTask();

            ///\}

            ///\name Event callbacks
            ///\{

            virtual void on_report_all_pre() = 0;
            virtual void on_report_check_package_pre(const QualifiedPackageName & p) = 0;
            virtual void on_report_package_success(const tr1::shared_ptr<const PackageID> & id) = 0;
            virtual void on_report_package_failure_pre(const tr1::shared_ptr<const PackageID> & id) = 0;
            virtual void on_report_package_is_masked(const tr1::shared_ptr<const PackageID> & id,
                    const tr1::shared_ptr<const PackageID> & origin) = 0;
            virtual void on_report_package_is_vulnerable_pre(const tr1::shared_ptr<const PackageID> & id) = 0;
            virtual void on_report_package_is_vulnerable(const tr1::shared_ptr<const PackageID> & id, const std::string & tag) = 0;
            virtual void on_report_package_is_vulnerable_post(const tr1::shared_ptr<const PackageID> & id) = 0;
            virtual void on_report_package_is_missing(const tr1::shared_ptr<const PackageID> & id,
                    const RepositoryName & repo_name) = 0;
            virtual void on_report_package_is_unused(const tr1::shared_ptr<const PackageID> & id) = 0;
            virtual void on_report_package_failure_post(const tr1::shared_ptr<const PackageID> & id) = 0;
            virtual void on_report_check_package_post(const QualifiedPackageName & p) = 0;
            virtual void on_report_all_post() = 0;

            ///\}

            /**
             * Run the task.
             */
            void execute();
    };
}

#endif
