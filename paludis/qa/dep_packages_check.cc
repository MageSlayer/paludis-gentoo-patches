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
#include <paludis/dep_parser.hh>
#include <paludis/qa/dep_packages_check.hh>

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

        void visit(const PlainTextDepAtom * const)
        {
        }

        void visit(const BlockDepAtom * const)
        {
        }
    };
}

DepPackagesCheck::DepPackagesCheck()
{
}

CheckResult
DepPackagesCheck::operator() (const EbuildCheckData & e) const
{
    CheckResult result(stringify(e.get<ecd_name>()) + "-" + stringify(e.get<ecd_version>()),
            identifier());

    try
    {
        PackageDatabaseEntry ee(e.get<ecd_name>(), e.get<ecd_version>(),
                e.get<ecd_environment>()->package_database()->favourite_repository());
        VersionMetadata::ConstPointer metadata(
                e.get<ecd_environment>()->package_database()->fetch_metadata(ee));

        static std::set<QualifiedPackageName> suspicious_depend;
        if (suspicious_depend.empty())
        {
            suspicious_depend.insert(QualifiedPackageName("virtual/libc"));
        }

        Checker depend_checker(result, "DEPEND", suspicious_depend);
        std::string depend(metadata->get(vmk_depend));
        DepParser::parse(depend)->accept(&depend_checker);

        static std::set<QualifiedPackageName> suspicious_rdepend;
        if (suspicious_rdepend.empty())
        {
            suspicious_rdepend.insert(QualifiedPackageName("app-arch/rpm2targz"));
            suspicious_rdepend.insert(QualifiedPackageName("app-arch/unzip"));
            suspicious_rdepend.insert(QualifiedPackageName("dev-util/pkgconfig"));
            suspicious_rdepend.insert(QualifiedPackageName("sys-devel/autoconf"));
            suspicious_rdepend.insert(QualifiedPackageName("sys-devel/automake"));
            suspicious_rdepend.insert(QualifiedPackageName("sys-devel/flex"));
            suspicious_rdepend.insert(QualifiedPackageName("sys-devel/bison"));
            suspicious_rdepend.insert(QualifiedPackageName("dev-util/yacc"));
            suspicious_rdepend.insert(QualifiedPackageName("sys-devel/gettext"));
            suspicious_rdepend.insert(QualifiedPackageName("sys-devel/libtool"));
            suspicious_rdepend.insert(QualifiedPackageName("sys-devel/patch"));
            suspicious_rdepend.insert(QualifiedPackageName("app-doc/doxygen"));
            suspicious_rdepend.insert(QualifiedPackageName("x11-misc/imake"));
            suspicious_rdepend.insert(QualifiedPackageName("media-gfx/ebdftopcf"));
            suspicious_rdepend.insert(QualifiedPackageName("x11-apps/bdftopcf"));
            suspicious_rdepend.insert(QualifiedPackageName("app-arch/cabextract"));
        }

        Checker rdepend_checker(result, "RDEPEND", suspicious_rdepend);
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
DepPackagesCheck::identifier()
{
    static const std::string id("depend packages");
    return id;
}

