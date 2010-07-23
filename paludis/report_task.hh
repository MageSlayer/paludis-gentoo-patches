/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Fernando J. Pereda
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

#ifndef PALUDIS_GUARD_PALUDIS_REPORT_TASK_HH
#define PALUDIS_GUARD_PALUDIS_REPORT_TASK_HH 1

#include <paludis/util/pimp.hh>
#include <paludis/repository.hh>
#include <paludis/dep_tag-fwd.hh>

/** \file
 * Declarations for ReportTask.
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

    /**
     * Task to report the current state of the system.
     *
     * \ingroup g_tasks
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE ReportTask :
        private Pimp<ReportTask>
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

            ReportTask(const ReportTask &) = delete;
            ReportTask & operator= (const ReportTask &) = delete;

            ///\}

            ///\name Event callbacks
            ///\{

            virtual void on_report_all_pre() = 0;
            virtual void on_report_check_package_pre(const QualifiedPackageName & p) = 0;
            virtual void on_report_package_success(const std::shared_ptr<const PackageID> & id) = 0;
            virtual void on_report_package_failure_pre(const std::shared_ptr<const PackageID> & id) = 0;
            virtual void on_report_package_is_masked(const std::shared_ptr<const PackageID> & id,
                    const std::shared_ptr<const PackageIDSequence> & origins) = 0;
            virtual void on_report_package_is_vulnerable_pre(const std::shared_ptr<const PackageID> & id) = 0;
            virtual void on_report_package_is_vulnerable(const std::shared_ptr<const PackageID> & id, const GLSADepTag & glsa_tag) = 0;
            virtual void on_report_package_is_vulnerable_post(const std::shared_ptr<const PackageID> & id) = 0;
            virtual void on_report_package_is_missing(const std::shared_ptr<const PackageID> & id) = 0;
            virtual void on_report_package_is_unused(const std::shared_ptr<const PackageID> & id) = 0;
            virtual void on_report_package_failure_post(const std::shared_ptr<const PackageID> & id) = 0;
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
