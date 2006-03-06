/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

#include <paludis/qa/license_check.hh>
#include <paludis/dep_atom.hh>
#include <paludis/dep_parser.hh>
#include <paludis/util/tokeniser.hh>

using namespace paludis;
using namespace paludis::qa;

namespace
{
    struct Checker :
        DepAtomVisitorTypes::ConstVisitor
    {
        CheckResult & result;
        const Environment * const env;

        Checker(CheckResult & rr, const Environment * const e) :
            result(rr),
            env(e)
        {
        }

        void visit(const PlainTextDepAtom * const a)
        {
            if (! env->package_database()->fetch_repository(
                        env->package_database()->favourite_repository())->is_license(a->text()))
                result << Message(qal_major, "Item '" + a->text() + "' is not a licence");
        }

        void visit(const AllDepAtom * const a)
        {
            std::for_each(a->begin(), a->end(), accept_visitor(this));
        }

        void visit(const AnyDepAtom * const a)
        {
            std::for_each(a->begin(), a->end(), accept_visitor(this));
        }

        void visit(const UseDepAtom * const u)
        {
            std::for_each(u->begin(), u->end(), accept_visitor(this));
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
    CheckResult result(stringify(e.get<ecd_name>()) + "-" + stringify(e.get<ecd_version>()),
            identifier());

    try
    {
        do
        {
            PackageDatabaseEntry ee(e.get<ecd_name>(), e.get<ecd_version>(),
                    e.get<ecd_environment>()->package_database()->favourite_repository());
            VersionMetadata::ConstPointer metadata(
                    e.get<ecd_environment>()->package_database()->fetch_metadata(ee));

            std::string license(metadata->get(vmk_license));

            DepAtom::ConstPointer license_parts(0);
            try
            {
                license_parts = DepParser::parse(license,
                        DepParserPolicy<PlainTextDepAtom, true>::get_instance());

                Checker checker(result, e.get<ecd_environment>());
                license_parts->accept(&checker);
            }
            catch (const DepStringError & e)
            {
                result << Message(qal_major, "Invalid LICENSE: '" + e.message() + "' ("
                        + e.what() + ")");
                break;
            }
        } while (false);
    }
    catch (const InternalError &)
    {
        throw;
    }
    catch (const Exception & e)
    {
        result << Message(qal_fatal, "Caught Exception '" + e.message() + "' ("
                + e.what() + ")");
    }

    return result;
}

const std::string &
LicenseCheck::identifier()
{
    static const std::string id("license");
    return id;
}

