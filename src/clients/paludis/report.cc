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
#include <src/output/colour.hh>
#include <src/output/mask_displayer.hh>
#include <paludis/tasks/report_task.hh>
#include <paludis/mask.hh>
#include <paludis/package_id.hh>
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
            const Environment * const env;

        public:
            OurReportTask(tr1::shared_ptr<Environment> e) :
                ReportTask(e.get()),
                _n_packages(0),
                _n_errors(0),
                env(e.get())
            {
            }

            virtual void on_report_all_pre();
            virtual void on_report_check_package_pre(const QualifiedPackageName & p);
            virtual void on_report_package_success(const tr1::shared_ptr<const PackageID> & id);
            virtual void on_report_package_failure_pre(const tr1::shared_ptr<const PackageID> & id);
            virtual void on_report_package_is_masked(const tr1::shared_ptr<const PackageID> & id, const tr1::shared_ptr<const PackageID> & origin);
            virtual void on_report_package_is_vulnerable_pre(const tr1::shared_ptr<const PackageID> & id);
            virtual void on_report_package_is_vulnerable(const tr1::shared_ptr<const PackageID> & id, const std::string & tag);
            virtual void on_report_package_is_vulnerable_post(const tr1::shared_ptr<const PackageID> & id);
            virtual void on_report_package_is_missing(const tr1::shared_ptr<const PackageID> & id, const RepositoryName & repo_name);
            virtual void on_report_package_is_unused(const tr1::shared_ptr<const PackageID> & id);
            virtual void on_report_package_failure_post(const tr1::shared_ptr<const PackageID> & id);
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
    OurReportTask::on_report_package_success(const tr1::shared_ptr<const PackageID> &)
    {
    }

    void
    OurReportTask::on_report_package_failure_pre(const tr1::shared_ptr<const PackageID> & pde)
    {
        cout << "* " << colour(cl_package_name, *pde) << " NOT OK";
    }

    void
    OurReportTask::on_report_package_is_masked(const tr1::shared_ptr<const PackageID> & id,
            const tr1::shared_ptr<const PackageID> & origin)
    {
        cout << endl << "    Masked by: ";

        bool comma(false);
        for (PackageID::MasksConstIterator m(origin->begin_masks()), m_end(origin->end_masks()) ;
                m != m_end ; ++m)
        {
            if (comma)
                cout << ", ";

            MaskDisplayer d(env, id, true);
            (*m)->accept(d);
            cout << d.result();

            comma = true;
        }
        cout << " in its original repository '" << origin->repository()->name() << "'";
        ++_n_errors;
    }

    void
    OurReportTask::on_report_package_is_vulnerable_pre(const tr1::shared_ptr<const PackageID> &)
    {
        cout << endl << "    Affected by:";
    }

    void
    OurReportTask::on_report_package_is_vulnerable(const tr1::shared_ptr<const PackageID> &, const std::string & tag)
    {
        cout << " " << colour(cl_tag, tag);
        ++_n_errors;
    }

    void
    OurReportTask::on_report_package_is_vulnerable_post(const tr1::shared_ptr<const PackageID> &)
    {
    }

    void
    OurReportTask::on_report_package_is_missing(const tr1::shared_ptr<const PackageID> &,
            const RepositoryName & repo_name)
    {
        cout << endl << "    No longer exists in its original repository '" << repo_name << "'";
        ++_n_errors;
    }

    void
    OurReportTask::on_report_package_is_unused(const tr1::shared_ptr<const PackageID> &)
    {
        cout << endl << "    Not used by any package in world";
        ++_n_errors;
    }

    void
    OurReportTask::on_report_package_failure_post(const tr1::shared_ptr<const PackageID> &)
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

int do_report(tr1::shared_ptr<Environment> env)
{
    Context context("When performing report action from command line:");

    OurReportTask task(env);
    task.execute();

    return task.return_code();
}

