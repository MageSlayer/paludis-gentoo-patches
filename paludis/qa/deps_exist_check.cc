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

#include "deps_exist_check.hh"
#include <paludis/dep_parser.hh>
#include <paludis/dep_atom_visitor.hh>
#include <paludis/any_dep_atom.hh>
#include <paludis/all_dep_atom.hh>
#include <paludis/package_dep_atom.hh>
#include <paludis/block_dep_atom.hh>
#include <paludis/use_dep_atom.hh>

using namespace paludis;
using namespace paludis::qa;

namespace
{
    struct Checker :
        DepAtomVisitorTypes::ConstVisitor
    {
        CheckResult & result;
        const std::string role;
        const Environment * env;

        Checker(CheckResult & rr, const std::string & r, const Environment * e) :
            result(rr),
            role(r),
            env(e)
        {
        }

        void visit(const PackageDepAtom * const p)
        {
            if (env->package_database()->query(p)->empty())
                result << Message(qal_major, "No match for " + role + " entry '"
                        + stringify(*p) + "'");
        }

        void visit(const AllDepAtom * const a)
        {
            std::for_each(a->begin(), a->end(), accept_visitor(this));
        }

        void visit(const AnyDepAtom * const a)
        {
            /// \todo VV make this smarter
            std::for_each(a->begin(), a->end(), accept_visitor(this));
        }

        void visit(const UseDepAtom * const u)
        {
            std::for_each(u->begin(), u->end(), accept_visitor(this));
        }

        void visit(const BlockDepAtom * const b)
        {
            if (env->package_database()->query(b->blocked_atom())->empty())
                result << Message(qal_maybe, "No match for " + role + " block '!"
                        + stringify(*b->blocked_atom()) + "'");
        }
    };
}

DepsExistCheck::DepsExistCheck()
{
}

CheckResult
DepsExistCheck::operator() (const EbuildCheckData & e) const
{
    CheckResult result(stringify(e.get<ecd_name>()) + "-" + stringify(e.get<ecd_version>()),
            identifier());

    try
    {
        PackageDatabaseEntry ee(e.get<ecd_name>(), e.get<ecd_version>(),
                e.get<ecd_environment>()->package_database()->favourite_repository());
        VersionMetadata::ConstPointer metadata(
                e.get<ecd_environment>()->package_database()->fetch_metadata(ee));

        Checker depend_checker(result, "DEPEND", e.get<ecd_environment>());
        std::string depend(metadata->get(vmk_depend));
        DepParser::parse(depend)->accept(&depend_checker);

        Checker rdepend_checker(result, "RDEPEND", e.get<ecd_environment>());
        std::string rdepend(metadata->get(vmk_rdepend));
        DepParser::parse(rdepend)->accept(&rdepend_checker);

        Checker pdepend_checker(result, "PDEPEND", e.get<ecd_environment>());
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
DepsExistCheck::identifier()
{
    static const std::string id("deps exist");
    return id;
}

