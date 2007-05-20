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
#include <paludis/portage_dep_parser.hh>
#include <paludis/package_database_entry.hh>
#include <paludis/environment.hh>
#include <paludis/qa/license_check.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/qa/qa_environment.hh>
#include <paludis/util/visitor-impl.hh>

using namespace paludis;
using namespace paludis::qa;

namespace
{
    struct Checker :
        DepSpecVisitorTypes::ConstVisitor,
        DepSpecVisitorTypes::ConstVisitor::VisitChildren<Checker, AnyDepSpec>,
        DepSpecVisitorTypes::ConstVisitor::VisitChildren<Checker, UseDepSpec>,
        DepSpecVisitorTypes::ConstVisitor::VisitChildren<Checker, AllDepSpec>
    {
        using DepSpecVisitorTypes::ConstVisitor::VisitChildren<Checker, UseDepSpec>::visit;
        using DepSpecVisitorTypes::ConstVisitor::VisitChildren<Checker, AllDepSpec>::visit;
        using DepSpecVisitorTypes::ConstVisitor::VisitChildren<Checker, AnyDepSpec>::visit;

        CheckResult & result;
        const QAEnvironment * const env;

        Checker(CheckResult & rr, const QAEnvironment * const e) :
            result(rr),
            env(e)
        {
        }

        void visit(const PlainTextDepSpec * const a)
        {
            RepositoryLicensesInterface *li(env->package_database()->fetch_repository(
                        env->main_repository()->name())->licenses_interface);

            if (li && ! li->license_exists(a->text()))
                result << Message(qal_major, "Item '" + a->text() + "' is not a licence");
        }

        void visit(const BlockDepSpec * const) PALUDIS_ATTRIBUTE((noreturn));
        void visit(const PackageDepSpec * const) PALUDIS_ATTRIBUTE((noreturn));
    };

    void
    Checker::visit(const BlockDepSpec * const)
    {
        throw InternalError(PALUDIS_HERE, "Unexpected BlockDepSpec");
    }

    void
    Checker::visit(const PackageDepSpec * const)
    {
        throw InternalError(PALUDIS_HERE, "Unexpected PackageDepSpec");
    }
}

LicenseCheck::LicenseCheck()
{
}

CheckResult
LicenseCheck::operator() (const EbuildCheckData & e) const
{
    CheckResult result(stringify(e.name) + "-" + stringify(e.version),
            identifier());

    try
    {
        do
        {
            PackageDatabaseEntry ee(e.name, e.version,
                    e.environment->main_repository()->name());
            tr1::shared_ptr<const VersionMetadata> metadata(
                    e.environment->package_database()->fetch_repository(ee.repository)->version_metadata(ee.name, ee.version));


            try
            {
                Checker checker(result, e.environment);
                metadata->license_interface->license()->accept(&checker);
            }
            catch (const DepStringError & err)
            {
                result << Message(qal_major, "Invalid LICENSE: '" + err.message() + "' ("
                        + err.what() + ")");
                break;
            }
        } while (false);
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
LicenseCheck::identifier()
{
    static const std::string id("license");
    return id;
}

