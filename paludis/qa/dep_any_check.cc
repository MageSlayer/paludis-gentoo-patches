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

#include "dep_any_check.hh"

#include <paludis/dep_parser.hh>
#include <paludis/dep_atom_visitor.hh>
#include <paludis/any_dep_atom.hh>
#include <paludis/all_dep_atom.hh>
#include <paludis/package_dep_atom.hh>
#include <paludis/block_dep_atom.hh>
#include <paludis/use_dep_atom.hh>
#include <paludis/save.hh>

using namespace paludis;
using namespace paludis::qa;

namespace
{
    struct Checker :
        DepAtomVisitorTypes::ConstVisitor
    {
        CheckResult & result;
        const std::string role;
        bool in_any;

        Checker(CheckResult & rr, const std::string & r) :
            result(rr),
            role(r),
            in_any(false)
        {
        }

        void visit(const PackageDepAtom * const)
        {
        }

        void visit(const AllDepAtom * const a)
        {
            /* yes, the following line is correct. */
            Save<bool> in_any_save(&in_any, false);
            std::for_each(a->begin(), a->end(), accept_visitor(this));
        }

        void visit(const AnyDepAtom * const a)
        {
            Save<bool> in_any_save(&in_any, true);
            if (a->begin() == a->end())
                result << Message(qal_minor, "Empty || ( ) block in " + role);
            else
                std::for_each(a->begin(), a->end(), accept_visitor(this));
        }

        void visit(const UseDepAtom * const u)
        {
            if (in_any)
                result << Message(qal_maybe, "Conditional on '" + stringify(u->flag()) + 
                        "' inside || ( ) block in " + role);
            std::for_each(u->begin(), u->end(), accept_visitor(this));
        }

        void visit(const BlockDepAtom * const)
        {
        }
    };
}

DepAnyCheck::DepAnyCheck()
{
}

CheckResult
DepAnyCheck::operator() (const EbuildCheckData & e) const
{
    CheckResult result(stringify(e.get<ecd_name>()) + "-" + stringify(e.get<ecd_version>()),
            identifier());

    try
    {
        PackageDatabaseEntry ee(e.get<ecd_name>(), e.get<ecd_version>(),
                e.get<ecd_environment>()->package_database()->favourite_repository());
        VersionMetadata::ConstPointer metadata(
                e.get<ecd_environment>()->package_database()->fetch_metadata(ee));

        Checker depend_checker(result, "DEPEND");
        std::string depend(metadata->get(vmk_depend));
        DepParser::parse(depend)->accept(&depend_checker);

        Checker rdepend_checker(result, "RDEPEND");
        std::string rdepend(metadata->get(vmk_rdepend));
        DepParser::parse(rdepend)->accept(&rdepend_checker);

        Checker pdepend_checker(result, "PDEPEND");
        std::string pdepend(metadata->get(vmk_pdepend));
        DepParser::parse(pdepend)->accept(&pdepend_checker);
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
DepAnyCheck::identifier()
{
    static const std::string id("any deps");
    return id;
}

