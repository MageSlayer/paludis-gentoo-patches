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

#include "report.hh"
#include <src/output/colour.hh>
#include <src/output/mask_displayer.hh>
#include <paludis/legacy/report_task.hh>
#include <paludis/mask.hh>
#include <paludis/dep_tag.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/util/join.hh>
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
            OurReportTask(std::shared_ptr<Environment> e) :
                ReportTask(e.get()),
                _n_packages(0),
                _n_errors(0),
                env(e.get())
            {
            }

            virtual void on_report_all_pre();
            virtual void on_report_check_package_pre(const QualifiedPackageName & p);
            virtual void on_report_package_success(const std::shared_ptr<const PackageID> & id);
            virtual void on_report_package_failure_pre(const std::shared_ptr<const PackageID> & id);
            virtual void on_report_package_is_masked(const std::shared_ptr<const PackageID> & id, const std::shared_ptr<const PackageIDSequence> & origin);
            virtual void on_report_package_is_vulnerable_pre(const std::shared_ptr<const PackageID> & id);
            virtual void on_report_package_is_vulnerable(const std::shared_ptr<const PackageID> & id, const GLSADepTag & glsa_tag);
            virtual void on_report_package_is_vulnerable_post(const std::shared_ptr<const PackageID> & id);
            virtual void on_report_package_is_missing(const std::shared_ptr<const PackageID> & id);
            virtual void on_report_package_is_unused(const std::shared_ptr<const PackageID> & id);
            virtual void on_report_package_failure_post(const std::shared_ptr<const PackageID> & id);
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
    OurReportTask::on_report_package_success(const std::shared_ptr<const PackageID> &)
    {
    }

    void
    OurReportTask::on_report_package_failure_pre(const std::shared_ptr<const PackageID> & pde)
    {
        cout << "* " << colour(cl_package_name, *pde) << " NOT OK";
    }

    void
    OurReportTask::on_report_package_is_masked(const std::shared_ptr<const PackageID> & id,
            const std::shared_ptr<const PackageIDSequence> & origins)
    {
        cout << endl << "    Masked by: ";

        bool outer_comma(false);
        for (PackageIDSequence::ConstIterator o(origins->begin()), o_end(origins->end()) ;
                o != o_end ; ++o)
        {
            if (outer_comma)
                cout << "; ";

            bool comma(false);
            for (PackageID::MasksConstIterator m((*o)->begin_masks()), m_end((*o)->end_masks()) ;
                    m != m_end ; ++m)
            {
                if (comma)
                    cout << ", ";

                MaskDisplayer d(env, id, true);
                (*m)->accept(d);
                cout << d.result();

                comma = true;
            }
            cout << " in '" << (*o)->repository_name() << "'";
            outer_comma = true;
        }
        ++_n_errors;
    }

    void
    OurReportTask::on_report_package_is_vulnerable_pre(const std::shared_ptr<const PackageID> &)
    {
        cout << endl << "    This package has following security issues:";
    }

    void
    OurReportTask::on_report_package_is_vulnerable(const std::shared_ptr<const PackageID> &, const GLSADepTag & glsa_tag)
    {
        cout << endl << "    " << colour(cl_error, glsa_tag.short_text() + ": \"" + glsa_tag.glsa_title() +"\"")
                    << endl << colour(cl_error, "        -> " + stringify(glsa_tag.glsa_file()));
        ++_n_errors;
    }

    void
    OurReportTask::on_report_package_is_vulnerable_post(const std::shared_ptr<const PackageID> &)
    {
    }

    void
    OurReportTask::on_report_package_is_missing(const std::shared_ptr<const PackageID> & id)
    {
        cout << endl << "    No longer exists in original repositories '" <<
            join(id->from_repositories_key()->value()->begin(), id->from_repositories_key()->value()->end(),
                    "', '") << "'";
        ++_n_errors;
    }

    void
    OurReportTask::on_report_package_is_unused(const std::shared_ptr<const PackageID> &)
    {
        cout << endl << "    Not used by any package in world";
        ++_n_errors;
    }

    void
    OurReportTask::on_report_package_failure_post(const std::shared_ptr<const PackageID> &)
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

int do_report(const std::shared_ptr<Environment> & env)
{
    Context context("When performing report action from command line:");

    OurReportTask task(env);
    task.execute();

    return task.return_code();
}

