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

#include "rdepend_packages_check.hh"
#include <paludis/dep_parser.hh>
#include <paludis/dep_atom.hh>

using namespace paludis;
using namespace paludis::qa;

namespace
{
    struct Checker :
        DepAtomVisitorTypes::ConstVisitor
    {
        CheckResult & result;
        const std::string role;
        const std::set<QualifiedPackageName> & suspicious;

        Checker(CheckResult & rr, const std::string & r, const std::set<QualifiedPackageName> & s) :
            result(rr),
            role(r),
            suspicious(s)
        {
        }

        void visit(const PackageDepAtom * const p)
        {
            if (suspicious.end() != suspicious.find(p->package()))
                result << Message(qal_maybe, "Suspicious " + role + " entry '"
                        + stringify(p->package()) + "'");
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

        void visit(const BlockDepAtom * const)
        {
        }
    };
}

RdependPackagesCheck::RdependPackagesCheck()
{
}

CheckResult
RdependPackagesCheck::operator() (const EbuildCheckData & e) const
{
    CheckResult result(stringify(e.get<ecd_name>()) + "-" + stringify(e.get<ecd_version>()),
            identifier());

    try
    {
        PackageDatabaseEntry ee(e.get<ecd_name>(), e.get<ecd_version>(),
                e.get<ecd_environment>()->package_database()->favourite_repository());
        VersionMetadata::ConstPointer metadata(
                e.get<ecd_environment>()->package_database()->fetch_metadata(ee));

        static std::set<QualifiedPackageName> suspicious;
        if (suspicious.empty())
        {
            suspicious.insert(QualifiedPackageName("app-arch/rpm2targz"));
            suspicious.insert(QualifiedPackageName("app-arch/unzip"));
            suspicious.insert(QualifiedPackageName("dev-util/pkgconfig"));
            suspicious.insert(QualifiedPackageName("sys-devel/autoconf"));
            suspicious.insert(QualifiedPackageName("sys-devel/automake"));
            suspicious.insert(QualifiedPackageName("sys-devel/flex"));
            suspicious.insert(QualifiedPackageName("sys-devel/gettext"));
            suspicious.insert(QualifiedPackageName("sys-devel/libtool"));
            suspicious.insert(QualifiedPackageName("sys-devel/patch"));
            suspicious.insert(QualifiedPackageName("app-doc/doxygen"));
        }

        Checker rdepend_checker(result, "RDEPEND", suspicious);
        std::string rdepend(metadata->get(vmk_rdepend));
        DepParser::parse(rdepend)->accept(&rdepend_checker);
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
RdependPackagesCheck::identifier()
{
    static const std::string id("rdepend packages");
    return id;
}

