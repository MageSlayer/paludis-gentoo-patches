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
#include <paludis/package_database_entry.hh>
#include <paludis/environment.hh>
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

            if (! r->use_interface)
                throw InternalError(PALUDIS_HERE, "Confused: Repository does not have a UseInterface.");

            if (r->use_interface->is_arch_flag(u->flag()))
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
            else if (r->use_interface->is_expand_flag(u->flag()))
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
    CheckResult result(stringify(e.name) + "-" + stringify(e.version),
            identifier());

    try
    {
        PackageDatabaseEntry ee(e.name, e.version,
                e.environment->package_database()->favourite_repository());
        VersionMetadata::ConstPointer metadata(
                e.environment->package_database()->fetch_repository(ee.repository)->version_metadata(ee.name, ee.version));

        std::set<UseFlagName> iuse;
        WhitespaceTokeniser::get_instance()->tokenise(metadata->get_ebuild_interface()->
                iuse, create_inserter<UseFlagName>(std::inserter(iuse, iuse.begin())));
        iuse.insert(UseFlagName("bootstrap"));
        iuse.insert(UseFlagName("build"));

        Checker depend_checker(result, "DEPEND", e.environment, iuse);
        std::string depend(metadata->deps.build_depend_string);
        PortageDepParser::parse(depend)->accept(&depend_checker);

        Checker rdepend_checker(result, "RDEPEND", e.environment, iuse);
        std::string rdepend(metadata->deps.run_depend_string);
        PortageDepParser::parse(rdepend)->accept(&rdepend_checker);

        Checker pdepend_checker(result, "PDEPEND", e.environment, iuse);
        std::string pdepend(metadata->deps.post_depend_string);
        PortageDepParser::parse(pdepend)->accept(&pdepend_checker);

        Checker provide_checker(result, "PROVIDE", e.environment, iuse);
        std::string provide(metadata->get_ebuild_interface()->provide_string);
        PortageDepParser::parse(provide, PortageDepParserPolicy<PackageDepAtom, false>::get_instance())->accept(&provide_checker);

        Checker license_checker(result, "LICENSE", e.environment, iuse);
        std::string license(metadata->license_string);
        PortageDepParser::parse(license, PortageDepParserPolicy<PlainTextDepAtom, true>::get_instance())->accept(&license_checker);

        Checker src_uri_checker(result, "SRC_URI", e.environment, iuse);
        if (metadata->get_ebuild_interface() == 0)
            result << Message(qal_fatal, "Not an ebuild");

        std::string src_uri(metadata->get_ebuild_interface()->src_uri);

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

