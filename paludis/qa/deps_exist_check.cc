/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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
#include <paludis/qa/deps_exist_check.hh>
#include <paludis/util/save.hh>

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
        bool in_any;

        Checker(CheckResult & rr, const std::string & r, const Environment * e) :
            result(rr),
            role(r),
            env(e),
            in_any(false)
        {
        }

        void visit(const PackageDepAtom * const p)
        {
            if (env->package_database()->query(p, is_either)->empty())
            {
                if (in_any)
                    result << Message(qal_maybe, "No match for " + role + " entry '"
                            + stringify(*p) + "' inside || ( ) block");
                else
                    result << Message(qal_major, "No match for " + role + " entry '"
                            + stringify(*p) + "'");
            }
        }

        void visit(const AllDepAtom * const a)
        {
            std::for_each(a->begin(), a->end(), accept_visitor(this));
        }

        void visit(const AnyDepAtom * const a)
        {
            /// \todo VV make this smarter
            Save<bool> save_in_any(&in_any, true);
            std::for_each(a->begin(), a->end(), accept_visitor(this));
        }

        void visit(const UseDepAtom * const u)
        {
            std::for_each(u->begin(), u->end(), accept_visitor(this));
        }

        void visit(const BlockDepAtom * const b)
        {
            if (env->package_database()->query(b->blocked_atom(), is_either)->empty())
                result << Message(qal_maybe, "No match for " + role + " block '!"
                        + stringify(*b->blocked_atom()) + "'");
        }

        void visit(const PlainTextDepAtom * const)
        {
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
                e.get<ecd_environment>()->package_database()->fetch_repository(
                        ee.get<pde_repository>())->version_metadata(ee.get<pde_name>(), ee.get<pde_version>()));

        Checker depend_checker(result, "DEPEND", e.get<ecd_environment>());
        std::string depend(metadata->get<vm_deps>().get<vmd_build_depend_string>());
        PortageDepParser::parse(depend)->accept(&depend_checker);

        Checker rdepend_checker(result, "RDEPEND", e.get<ecd_environment>());
        std::string rdepend(metadata->get<vm_deps>().get<vmd_run_depend_string>());
        PortageDepParser::parse(rdepend)->accept(&rdepend_checker);

        Checker pdepend_checker(result, "PDEPEND", e.get<ecd_environment>());
        std::string pdepend(metadata->get<vm_deps>().get<vmd_post_depend_string>());
        PortageDepParser::parse(pdepend)->accept(&pdepend_checker);
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

