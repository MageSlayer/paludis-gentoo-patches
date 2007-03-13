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
#include <paludis/qa/dep_flags_check.hh>
#include <paludis/util/tokeniser.hh>
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
        DepSpecVisitorTypes::ConstVisitor::VisitChildren<Checker, AnyDepSpec>
    {
        using DepSpecVisitorTypes::ConstVisitor::VisitChildren<Checker, AllDepSpec>::visit;
        using DepSpecVisitorTypes::ConstVisitor::VisitChildren<Checker, AnyDepSpec>::visit;

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

        void visit(const PackageDepSpec * const)
        {
        }

        void visit(const UseDepSpec * const u)
        {
            std::tr1::shared_ptr<const Repository> r(env->package_database()->fetch_repository(env->package_database()->
                        favourite_repository()));

            if (! r->use_interface)
                throw InternalError(PALUDIS_HERE, "Confused: Repository does not have a UseInterface.");

            if (r->use_interface->arch_flags()->count(u->flag()))
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
            else
            {
                do
                {
                    bool found_match(false);
                    std::tr1::shared_ptr<const UseFlagNameCollection> c(r->use_interface->use_expand_prefixes());

                    for (UseFlagNameCollection::Iterator i(c->begin()), i_end(c->end()) ;
                            i != i_end ; ++i)
                    {
                        std::string prefix(stringify(*i)), flag(stringify(u->flag()));
                        if (0 == flag.compare(0, prefix.length(), prefix))
                        {
                            found_match = true;
                            break;
                        }
                    }

                    if (found_match)
                        break;

                    if (iuse.end() == iuse.find(u->flag()))
                        result << Message(qal_major, "Conditional flag '" + stringify(u->flag()) +
                                "' in " + role + " not in IUSE");
                } while (false);
            }

            std::for_each(u->begin(), u->end(), accept_visitor(this));
        }

        void visit(const PlainTextDepSpec * const)
        {
        }

        void visit(const BlockDepSpec * const)
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
                e.environment->portage_repository()->name());
        std::tr1::shared_ptr<const VersionMetadata> metadata(
                e.environment->package_database()->fetch_repository(ee.repository)->version_metadata(ee.name, ee.version));

        if (metadata->ebuild_interface == 0)
            result << Message(qal_fatal, "Not an ebuild");
        else
        {
            std::set<UseFlagName> iuse;
            WhitespaceTokeniser::get_instance()->tokenise(metadata->ebuild_interface->
                    iuse, create_inserter<UseFlagName>(std::inserter(iuse, iuse.begin())));
            iuse.insert(UseFlagName("bootstrap"));
            iuse.insert(UseFlagName("build"));

            Checker depend_checker(result, "DEPEND", e.environment, iuse);
            metadata->deps_interface->build_depend()->accept(&depend_checker);

            Checker rdepend_checker(result, "RDEPEND", e.environment, iuse);
            metadata->deps_interface->run_depend()->accept(&rdepend_checker);

            Checker pdepend_checker(result, "PDEPEND", e.environment, iuse);
            metadata->deps_interface->post_depend()->accept(&pdepend_checker);

            Checker provide_checker(result, "PROVIDE", e.environment, iuse);
            metadata->ebuild_interface->provide()->accept(&provide_checker);

            Checker license_checker(result, "LICENSE", e.environment, iuse);
            metadata->license_interface->license()->accept(&license_checker);

            Checker src_uri_checker(result, "SRC_URI", e.environment, iuse);
            metadata->ebuild_interface->src_uri()->accept(&src_uri_checker);
        }
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
DepFlagsCheck::identifier()
{
    static const std::string id("dep_flags");
    return id;
}

