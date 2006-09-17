/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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
#include <paludis/package_database_entry.hh>
#include <paludis/environment.hh>
#include <paludis/qa/src_uri_check.hh>
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
        bool fetch_restrict;
        const Environment * const env;

        Checker(CheckResult & rr, bool f, const Environment * const e) :
            result(rr),
            fetch_restrict(f),
            env(e)
        {
        }

        void visit(const PlainTextDepAtom * const a)
        {
            if (a->text().empty())
                return;

            std::string::size_type p(std::string::npos);
            if (std::string::npos == ((p = a->text().find("://"))) && ! fetch_restrict)
                result << Message(qal_major, "No protocol found for '" + a->text() +
                            "' and not fetch restricted");

            else if ((std::string::npos != p) &&
                    (("http" != a->text().substr(0, p)) &&
                     ("https" != a->text().substr(0, p)) &&
                     ("mirror" != a->text().substr(0, p)) &&
                     ("ftp" != a->text().substr(0, p))))
                result << Message(qal_major, "Unrecognised protocol for '" + a->text() + "'");

            else if ((std::string::npos != a->text().find("dev.gentoo.org")) ||
                    (std::string::npos != a->text().find("cvs.gentoo.org")) ||
                    (std::string::npos != a->text().find("toucan.gentoo.org")) ||
                    (std::string::npos != a->text().find("emu.gentoo.org")) ||
                    (std::string::npos != a->text().find("alpha.gnu.org")) ||
                    (std::string::npos != a->text().find("geocities.com")))
                result << Message(qal_major, "Unreliable host for '" + a->text() + "'");

            else
            {
                if (0 == a->text().compare(0, 9, "mirror://"))
                {
                    std::string mirror_host(a->text().substr(9));
                    std::string::size_type pos(mirror_host.find('/'));
                    if (std::string::npos == pos)
                        result << Message(qal_major, "Malformed SRC_URI component '" + a->text() + "'");
                    else
                    {
                        mirror_host.erase(pos);
                        RepositoryMirrorsInterface * m(env->package_database()->fetch_repository(
                                    env->package_database()->favourite_repository())->mirrors_interface);
                        if (! m)
                            result << Message(qal_major, "Mirror '" + a->text() + "' used, but repository '"
                                    + stringify(env->package_database()->favourite_repository())
                                    + "' defines no mirrors interface");
                        else if (! m->is_mirror(mirror_host))
                            result << Message(qal_major, "Unknown mirror '" + mirror_host
                                    + "' for '" + a->text() + "'");
                    }
                }
            }
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
            result << Message(qal_major, "Unexpected || dep block");
            std::for_each(u->begin(), u->end(), accept_visitor(this));
        }

        void visit(const BlockDepAtom * const) PALUDIS_ATTRIBUTE((noreturn));
        void visit(const PackageDepAtom * const) PALUDIS_ATTRIBUTE((noreturn));
    };

    void
    Checker::visit(const BlockDepAtom * const)
    {
        throw InternalError(PALUDIS_HERE, "Unexpected BlockDepAtom");
    }

    void
    Checker::visit(const PackageDepAtom * const)
    {
        throw InternalError(PALUDIS_HERE, "Unexpected PackageDepAtom");
    }
}

SrcUriCheck::SrcUriCheck()
{
}

CheckResult
SrcUriCheck::operator() (const EbuildCheckData & e) const
{
    CheckResult result(stringify(e.name) + "-" + stringify(e.version),
            identifier());

    try
    {
        do
        {
            PackageDatabaseEntry ee(e.name, e.version,
                    e.environment->package_database()->favourite_repository());
            VersionMetadata::ConstPointer metadata(
                    e.environment->package_database()->fetch_repository(ee.repository)->version_metadata(ee.name, ee.version));

            std::string src_uri(metadata->get_ebuild_interface()->src_uri);

            DepAtom::ConstPointer src_uri_parts(0);
            try
            {
                src_uri_parts = PortageDepParser::parse(src_uri,
                        PortageDepParserPolicy<PlainTextDepAtom, false>::get_instance());

                std::set<std::string> restricts;
                Tokeniser<delim_kind::AnyOfTag, delim_mode::DelimiterTag> tokeniser(" \t\n");
                tokeniser.tokenise(metadata->get_ebuild_interface()->restrict_string,
                        std::inserter(restricts, restricts.begin()));

                bool fetch_restrict(restricts.end() != restricts.find("fetch"));
                Checker checker(result, fetch_restrict, e.environment);
                src_uri_parts->accept(&checker);
            }
            catch (const DepStringError & err)
            {
                result << Message(qal_major, "Invalid SRC_URI: '" + err.message() + "' ("
                        + err.what() + ")");
                break;
            }
        } while (false);
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
SrcUriCheck::identifier()
{
    static const std::string id("src_uri");
    return id;
}

