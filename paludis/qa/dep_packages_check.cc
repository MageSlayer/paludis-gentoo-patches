/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <paludis/dep_spec.hh>
#include <paludis/package_database_entry.hh>
#include <paludis/environment.hh>
#include <paludis/portage_dep_parser.hh>
#include <paludis/qa/dep_packages_check.hh>
#include <paludis/config_file.hh>
#include <paludis/util/log.hh>
#include <paludis/util/system.hh>
#include <paludis/qa/qa_environment.hh>
#include <paludis/repositories/gentoo/portage_repository.hh>
#include <set>

using namespace paludis;
using namespace paludis::qa;

namespace
{
    struct Checker :
        DepSpecVisitorTypes::ConstVisitor,
        DepSpecVisitorTypes::ConstVisitor::VisitChildren<Checker, AllDepSpec>,
        DepSpecVisitorTypes::ConstVisitor::VisitChildren<Checker, AnyDepSpec>,
        DepSpecVisitorTypes::ConstVisitor::VisitChildren<Checker, UseDepSpec>
    {
        using DepSpecVisitorTypes::ConstVisitor::VisitChildren<Checker, AllDepSpec>::visit;
        using DepSpecVisitorTypes::ConstVisitor::VisitChildren<Checker, AnyDepSpec>::visit;
        using DepSpecVisitorTypes::ConstVisitor::VisitChildren<Checker, UseDepSpec>::visit;

        CheckResult & result;
        const std::string role;
        const std::set<QualifiedPackageName> & suspicious;

        Checker(CheckResult & rr, const std::string & r, const std::set<QualifiedPackageName> & s) :
            result(rr),
            role(r),
            suspicious(s)
        {
        }

        void visit(const PackageDepSpec * const p)
        {
            if (p->package_ptr())
                if (suspicious.end() != suspicious.find(*p->package_ptr()))
                    result << Message(qal_maybe, "Suspicious " + role + " entry '"
                            + stringify(*p->package_ptr()) + "'");
        }

        void visit(const PlainTextDepSpec * const)
        {
        }

        void visit(const BlockDepSpec * const)
        {
        }
    };
}

DepPackagesCheck::DepPackagesCheck()
{
}

CheckResult
DepPackagesCheck::operator() (const EbuildCheckData & e) const
{
    CheckResult result(stringify(e.name) + "-" + stringify(e.version),
            identifier());

    try
    {
        PackageDatabaseEntry ee(e.name, e.version,
                e.environment->portage_repository()->name());
        std::tr1::shared_ptr<const VersionMetadata> metadata(
                e.environment->package_database()->fetch_repository(ee.repository)->version_metadata(ee.name, ee.version));

        static std::set<QualifiedPackageName> suspicious_depend;
        if (suspicious_depend.empty())
        {
            suspicious_depend.insert(QualifiedPackageName("often/not-been-on-boats"));

            try
            {
                LineConfigFile file(FSEntry(getenv_with_default(
                            "PALUDIS_QA_DATA_DIR", DATADIR "/paludis/qa/")) / "suspicious_depend.txt");
                std::copy(file.begin(), file.end(), create_inserter<QualifiedPackageName>(std::inserter(
                                suspicious_depend, suspicious_depend.end())));
            }
            catch (const Exception & eee)
            {
                Log::get_instance()->message(ll_warning, lc_context,
                        "Cannot load suspicious DEPEND list from suspicious_depend.txt due to exception '"
                        + eee.message() + "' (" + eee.what() + ")");
            }
        }

        Checker depend_checker(result, "DEPEND", suspicious_depend);
        metadata->deps_interface->build_depend()->accept(&depend_checker);

        static std::set<QualifiedPackageName> suspicious_rdepend;
        if (suspicious_rdepend.empty())
        {
            suspicious_rdepend.insert(QualifiedPackageName("often/not-been-on-boats"));

            try
            {
                LineConfigFile file(FSEntry(getenv_with_default(
                            "PALUDIS_QA_DATA_DIR", DATADIR "/paludis/qa/")) / "suspicious_rdepend.txt");
                std::copy(file.begin(), file.end(), create_inserter<QualifiedPackageName>(std::inserter(
                                suspicious_rdepend, suspicious_rdepend.end())));
            }
            catch (const Exception & eee)
            {
                Log::get_instance()->message(ll_warning, lc_context,
                        "Cannot load suspicious RDEPEND list from suspicious_rdepend.txt due to exception '"
                        + eee.message() + "' (" + eee.what() + ")");
            }
        }

        Checker rdepend_checker(result, "RDEPEND", suspicious_rdepend);
        metadata->deps_interface->run_depend()->accept(&rdepend_checker);
    }
    catch (const InternalError &)
    {
        throw;
    }
    catch (const Exception & err)
    {
        result << Message(qal_fatal, "Caught Exception '" + err.message() + "' ("
                + err.what() + ")");
    }

    return result;
}

const std::string &
DepPackagesCheck::identifier()
{
    static const std::string id("depend_packages");
    return id;
}

