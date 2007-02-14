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

#include <paludis/dep_atom.hh>
#include <paludis/portage_dep_parser.hh>
#include <paludis/package_database_entry.hh>
#include <paludis/environment.hh>
#include <paludis/qa/license_check.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/qa/qa_environment.hh>
#include <paludis/repositories/gentoo/portage_repository.hh>

using namespace paludis;
using namespace paludis::qa;

namespace
{
    struct Checker :
        DepAtomVisitorTypes::ConstVisitor,
        DepAtomVisitorTypes::ConstVisitor::VisitChildren<Checker, AnyDepAtom>,
        DepAtomVisitorTypes::ConstVisitor::VisitChildren<Checker, UseDepAtom>,
        DepAtomVisitorTypes::ConstVisitor::VisitChildren<Checker, AllDepAtom>
    {
        using DepAtomVisitorTypes::ConstVisitor::VisitChildren<Checker, UseDepAtom>::visit;
        using DepAtomVisitorTypes::ConstVisitor::VisitChildren<Checker, AllDepAtom>::visit;
        using DepAtomVisitorTypes::ConstVisitor::VisitChildren<Checker, AnyDepAtom>::visit;

        CheckResult & result;
        const QAEnvironment * const env;

        Checker(CheckResult & rr, const QAEnvironment * const e) :
            result(rr),
            env(e)
        {
        }

        void visit(const PlainTextDepAtom * const a)
        {
            RepositoryLicensesInterface *li(env->package_database()->fetch_repository(
                        env->portage_repository()->name())->licenses_interface);

            if (li && ! li->license_exists(a->text()))
                result << Message(qal_major, "Item '" + a->text() + "' is not a licence");
        }

        void visit(const BlockDepAtom * const) PALUDIS_ATTRIBUTE((noreturn));
        void visit(const PackageDepAtom * const) PALUDIS_ATTRIBUTE((noreturn));
    };

    void
    Checker::visit(const BlockDepAtom * const)
    {
        throw InternalError(PALUDIS_HERE, "Unexpected BlockDepAtom");
    }

    void
    Checker::visit(const PackageDepAtom * const)
    {
        throw InternalError(PALUDIS_HERE, "Unexpected PackageDepAtom");
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
                    e.environment->portage_repository()->name());
            std::tr1::shared_ptr<const VersionMetadata> metadata(
                    e.environment->package_database()->fetch_repository(ee.repository)->version_metadata(ee.name, ee.version));


            std::string license(metadata->license_interface->license_string);

            std::tr1::shared_ptr<const DepAtom> license_parts;
            try
            {
                license_parts = metadata->license_interface->license();

                Checker checker(result, e.environment);
                license_parts->accept(&checker);
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

