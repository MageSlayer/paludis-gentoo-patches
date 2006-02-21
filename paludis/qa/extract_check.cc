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

#include "extract_check.hh"
#include <paludis/dep_atom.hh>
#include <paludis/dep_parser.hh>
#include <paludis/tokeniser.hh>

using namespace paludis;
using namespace paludis::qa;

namespace
{
    struct Checker :
        DepAtomVisitorTypes::ConstVisitor
    {
        bool need_zip;
        bool have_zip;

        Checker() :
            need_zip(false),
            have_zip(false)
        {
        }

        void visit(const PlainTextDepAtom * const a)
        {
            if (a->text().length() >= 4)
                if (a->text().substr(a->text().length() - 4) == ".zip")
                    need_zip = true;
        }

        void visit(const AllDepAtom * const a)
        {
            std::for_each(a->begin(), a->end(), accept_visitor(this));
        }

        void visit(const UseDepAtom * const u)
        {
            std::for_each(u->begin(), u->end(), accept_visitor(this));
        }

        void visit(const AnyDepAtom * const u)
        {
            std::for_each(u->begin(), u->end(), accept_visitor(this));
        }

        void visit(const BlockDepAtom * const)
        {
        }

        void visit(const PackageDepAtom * const p)
        {
            if (p->package() == QualifiedPackageName("app-arch/unzip"))
                have_zip = true;
        }
    };
}

ExtractCheck::ExtractCheck()
{
}

CheckResult
ExtractCheck::operator() (const EbuildCheckData & e) const
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

            Checker checker;
            DepParser::parse(metadata->get(vmk_src_uri),
                    DepParserPolicy<PlainTextDepAtom, false>::get_instance())->accept(&checker);
            DepParser::parse(metadata->get(vmk_depend))->accept(&checker);

            if (checker.need_zip && ! checker.have_zip)
                result << Message(qal_major, "Found .zip in SRC_URI but app-arch/unzip is not in DEPEND");

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
ExtractCheck::identifier()
{
    static const std::string id("extract");
    return id;
}



