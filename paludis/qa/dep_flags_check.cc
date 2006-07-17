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
#include <paludis/qa/dep_flags_check.hh>
#include <paludis/util/tokeniser.hh>
#include <set>

using namespace paludis;
using namespace paludis::qa;

namespace
{
    struct Checker :
        DepAtomVisitorTypes::ConstVisitor
    {
        CheckResult & result;
        const std::string role;
        const Environment * const env;
        const std::set<UseFlagName> & iuse;

        Checker(CheckResult & rr, const std::string & r,
                const Environment * const e, const std::set<UseFlagName> & i) :
            result(rr),
            role(r),
            env(e),
            iuse(i)
        {
        }

        void visit(const PackageDepAtom * const)
        {
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
            Repository::ConstPointer r(env->package_database()->fetch_repository(env->package_database()->
                        favourite_repository()));

            if (! r->get_interface<repo_use>())
                throw InternalError(PALUDIS_HERE, "Confused: Repository does not have a UseInterface.");

            if (r->get_interface<repo_use>()->is_arch_flag(u->flag()))
            {
                if (role == "DEPEND" || role == "RDEPEND" || role == "PDEPEND")
                {
                    if (u->inverse())
                        result << Message(qal_maybe, "Inverse arch flag '" + stringify(u->flag()) +
                                "' in " + role);
                }
                else if (role != "SRC_URI")
                    result << Message(qal_major, "Arch flag '" + stringify(u->flag()) +
                            "' in " + role);
            }
            else if (r->get_interface<repo_use>()->is_expand_flag(u->flag()))
            {
            }
            else if (iuse.end() == iuse.find(u->flag()))
                result << Message(qal_major, "Conditional flag '" + stringify(u->flag()) +
                        "' in " + role + " not in IUSE");

            std::for_each(u->begin(), u->end(), accept_visitor(this));
        }

        void visit(const PlainTextDepAtom * const)
        {
        }

        void visit(const BlockDepAtom * const)
        {
        }
    };
}

DepFlagsCheck::DepFlagsCheck()
{
}

CheckResult
DepFlagsCheck::operator() (const EbuildCheckData & e) const
{
    CheckResult result(stringify(e.get<ecd_name>()) + "-" + stringify(e.get<ecd_version>()),
            identifier());

    try
    {
        PackageDatabaseEntry ee(e.get<ecd_name>(), e.get<ecd_version>(),
                e.get<ecd_environment>()->package_database()->favourite_repository());
        VersionMetadata::ConstPointer metadata(
                e.get<ecd_environment>()->package_database()->fetch_repository(ee.get<pde_repository>())->version_metadata(ee.get<pde_name>(), ee.get<pde_version>()));

        std::set<UseFlagName> iuse;
        WhitespaceTokeniser::get_instance()->tokenise(metadata->get_ebuild_interface()->
                get<evm_iuse>(), create_inserter<UseFlagName>(std::inserter(iuse, iuse.begin())));
        iuse.insert(UseFlagName("bootstrap"));
        iuse.insert(UseFlagName("build"));

        Checker depend_checker(result, "DEPEND", e.get<ecd_environment>(), iuse);
        std::string depend(metadata->get<vm_deps>().get<vmd_build_depend_string>());
        PortageDepParser::parse(depend)->accept(&depend_checker);

        Checker rdepend_checker(result, "RDEPEND", e.get<ecd_environment>(), iuse);
        std::string rdepend(metadata->get<vm_deps>().get<vmd_run_depend_string>());
        PortageDepParser::parse(rdepend)->accept(&rdepend_checker);

        Checker pdepend_checker(result, "PDEPEND", e.get<ecd_environment>(), iuse);
        std::string pdepend(metadata->get<vm_deps>().get<vmd_post_depend_string>());
        PortageDepParser::parse(pdepend)->accept(&pdepend_checker);

        Checker provide_checker(result, "PROVIDE", e.get<ecd_environment>(), iuse);
        std::string provide(metadata->get_ebuild_interface()->get<evm_provide>());
        PortageDepParser::parse(provide, PortageDepParserPolicy<PackageDepAtom, false>::get_instance())->accept(&provide_checker);

        Checker license_checker(result, "LICENSE", e.get<ecd_environment>(), iuse);
        std::string license(metadata->get<vm_license>());
        PortageDepParser::parse(license, PortageDepParserPolicy<PlainTextDepAtom, true>::get_instance())->accept(&license_checker);

        Checker src_uri_checker(result, "SRC_URI", e.get<ecd_environment>(), iuse);
        if (metadata->get_ebuild_interface() == 0)
            result << Message(qal_fatal, "Not an ebuild");

        std::string src_uri(metadata->get_ebuild_interface()->get<evm_src_uri>());

        PortageDepParser::parse(src_uri, PortageDepParserPolicy<PlainTextDepAtom, true>::get_instance())->accept(&src_uri_checker);
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
DepFlagsCheck::identifier()
{
    static const std::string id("dep_flags");
    return id;
}

