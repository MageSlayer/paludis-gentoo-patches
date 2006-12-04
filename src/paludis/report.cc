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

#include "report.hh"
#include "colour.hh"
#include <paludis/tasks/report_task.hh>
#include <paludis/environment/default/default_environment.hh>
#include <iostream>

/** \file
 * Handle the --report action for the main paludis program.
 */

using namespace paludis;
using std::cout;
using std::endl;

namespace
{
    class OurReportTask :
        public ReportTask
    {
        private:
            int _n_packages;
            int _n_errors;

        public:
            OurReportTask() :
                ReportTask(DefaultEnvironment::get_instance()),
                _n_packages(0),
                _n_errors(0)
            {
            }

            virtual void on_report_all_pre();
            virtual void on_report_check_package_pre(const QualifiedPackageName & p);
            virtual void on_report_package_success(const PackageDatabaseEntry & pde);
            virtual void on_report_package_failure_pre(const PackageDatabaseEntry & pde);
            virtual void on_report_package_is_masked(const PackageDatabaseEntry & pde, const MaskReasons & mr);
            virtual void on_report_package_is_vulnerable_pre(const PackageDatabaseEntry & pde);
            virtual void on_report_package_is_vulnerable(const PackageDatabaseEntry & pde, const std::string & tag);
            virtual void on_report_package_is_vulnerable_post(const PackageDatabaseEntry & pde);
            virtual void on_report_package_is_missing(const PackageDatabaseEntry & pde);
            virtual void on_report_package_is_unused(const PackageDatabaseEntry & pde);
            virtual void on_report_package_failure_post(const PackageDatabaseEntry & pde);
            virtual void on_report_check_package_post(const QualifiedPackageName & p);
            virtual void on_report_all_post();

            int return_code() const
            {
                return _n_errors ? 1 : 0;
            }
    };

    void
    OurReportTask::on_report_all_pre()
    {
        cout << colour(cl_heading, "Current state of the system") << endl << endl;
    }

    void
    OurReportTask::on_report_check_package_pre(const QualifiedPackageName &)
    {
    }

    void
    OurReportTask::on_report_package_success(const PackageDatabaseEntry &)
    {
    }

    void
    OurReportTask::on_report_package_failure_pre(const PackageDatabaseEntry & pde)
    {
        cout << "* " << colour(cl_package_name, pde) << " NOT OK";
    }

    void
    OurReportTask::on_report_package_is_masked(const PackageDatabaseEntry &, const MaskReasons & mr)
    {
        cout << endl << "    Masked by: ";

        bool comma(false);
        for (unsigned i(0), i_end(mr.size()); i != i_end; ++i)
            if (mr.test(i))
            {
                if (comma)
                    cout << ", ";
                cout << colour(cl_masked, MaskReason(i));
                comma = true;
            }
        ++_n_errors;
    }

    void
    OurReportTask::on_report_package_is_vulnerable_pre(const PackageDatabaseEntry &)
    {
        cout << endl << "    Affected by:";
    }

    void
    OurReportTask::on_report_package_is_vulnerable(const PackageDatabaseEntry &, const std::string & tag)
    {
        cout << " " << colour(cl_tag, tag);
        ++_n_errors;
    }

    void
    OurReportTask::on_report_package_is_vulnerable_post(const PackageDatabaseEntry &)
    {
    }

    void
    OurReportTask::on_report_package_is_missing(const PackageDatabaseEntry &)
    {
        cout << endl << "    No longer exists in its original repository";
        ++_n_errors;
    }

    void
    OurReportTask::on_report_package_is_unused(const PackageDatabaseEntry &)
    {
        cout << endl << "    Not used by any package in world";
        ++_n_errors;
    }

    void
    OurReportTask::on_report_package_failure_post(const PackageDatabaseEntry &)
    {
        cout << endl << endl;
    }

    void
    OurReportTask::on_report_check_package_post(const QualifiedPackageName &)
    {
        ++_n_packages;
    }

    void
    OurReportTask::on_report_all_post()
    {
        cout << "Finished processing " <<
            _n_packages << " " << (_n_packages != 1 ? "packages" : "package") << ". " <<
            _n_errors << " " << (_n_errors != 1 ? "errors" : "error") << "." << endl;
    }
}

int do_report()
{
    Context context("When performing report action from command line:");

    OurReportTask task;
    task.execute();

    return task.return_code();
}
