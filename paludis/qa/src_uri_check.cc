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

#include "src_uri_check.hh"
#include <paludis/nest_atom.hh>
#include <paludis/nest_parser.hh>
#include <paludis/tokeniser.hh>

using namespace paludis;
using namespace paludis::qa;

namespace
{
    struct Checker :
        NestAtomVisitorTypes::ConstVisitor
    {
        CheckResult & result;
        bool fetch_restrict;

        Checker(CheckResult & rr, bool f) :
            result(rr),
            fetch_restrict(f)
        {
        }

        void visit(const TextNestAtom * const a)
        {
            if (a->text().empty())
                return;

            std::string::size_type p(std::string::npos);
            if ((! fetch_restrict) && std::string::npos == ((p = a->text().find("://"))))
                result << Message(qal_major, "No protocol found for '" + a->text() +
                            "' and not fetch restricted");

            else if (("http" != a->text().substr(0, p)) &&
                    ("https" != a->text().substr(0, p)) &&
                    ("mirror" != a->text().substr(0, p)) &&
                    ("ftp" != a->text().substr(0, p)))
                result << Message(qal_major, "Unrecognised protocol for '" + a->text() + "'");

            else if ((std::string::npos != a->text().find("dev.gentoo.org")) ||
                    (std::string::npos != a->text().find("cvs.gentoo.org")) ||
                    (std::string::npos != a->text().find("toucan.gentoo.org")) ||
                    (std::string::npos != a->text().find("emu.gentoo.org")) ||
                    (std::string::npos != a->text().find("geocities.com")))
                result << Message(qal_major, "Unreliable host for '" + a->text() + "'");
        }

        void visit(const AllNestAtom * const a)
        {
            std::for_each(a->begin(), a->end(), accept_visitor(this));
        }

        void visit(const UseNestAtom * const u)
        {
            std::for_each(u->begin(), u->end(), accept_visitor(this));
        }
    };
}

SrcUriCheck::SrcUriCheck()
{
}

CheckResult
SrcUriCheck::operator() (const EbuildCheckData & e) const
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

            std::string src_uri(metadata->get(vmk_src_uri));

            NestAtom::ConstPointer src_uri_parts(0);
            try
            {
                src_uri_parts = NestParser::parse(src_uri);

                std::set<std::string> restricts;
                Tokeniser<delim_kind::AnyOfTag, delim_mode::DelimiterTag> tokeniser(" \t\n");
                tokeniser.tokenise(metadata->get(vmk_restrict),
                        std::inserter(restricts, restricts.begin()));

                bool fetch_restrict(restricts.end() != restricts.find("fetch"));
                Checker checker(result, fetch_restrict);
                src_uri_parts->accept(&checker);
            }
            catch (const DepStringError & e)
            {
                result << Message(qal_major, "Invalid SRC_URI: '" + e.message() + "' ("
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
SrcUriCheck::identifier()
{
    static const std::string id("src_uri");
    return id;
}


