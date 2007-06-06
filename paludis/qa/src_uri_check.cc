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
#include <paludis/portage_dep_parser.hh>
#include <paludis/package_database_entry.hh>
#include <paludis/environment.hh>
#include <paludis/qa/src_uri_check.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/qa/qa_environment.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <set>
#include <algorithm>

using namespace paludis;
using namespace paludis::qa;

namespace
{
    struct Checker :
        ConstVisitor<URISpecTree>,
        ConstVisitor<URISpecTree>::VisitConstSequence<Checker, UseDepSpec>,
        ConstVisitor<URISpecTree>::VisitConstSequence<Checker, AllDepSpec>
    {
        using ConstVisitor<URISpecTree>::VisitConstSequence<Checker, UseDepSpec>::visit_sequence;
        using ConstVisitor<URISpecTree>::VisitConstSequence<Checker, AllDepSpec>::visit_sequence;

        CheckResult & result;
        const QAEnvironment * const env;

        Checker(CheckResult & rr, const QAEnvironment * const e) :
            result(rr),
            env(e)
        {
        }

        void visit_leaf(const URIDepSpec & a)
        {
            if (a.original_url().empty())
                return;

            std::string::size_type p(std::string::npos);
            if ((std::string::npos != p) &&
                    (("http" != a.original_url().substr(0, p)) &&
                     ("https" != a.original_url().substr(0, p)) &&
                     ("mirror" != a.original_url().substr(0, p)) &&
                     ("ftp" != a.original_url().substr(0, p))))
                result << Message(qal_major, "Unrecognised protocol for '" + a.original_url() + "'");

            else if ((std::string::npos != a.original_url().find("dev.gentoo.org")) ||
                    (std::string::npos != a.original_url().find("cvs.gentoo.org")) ||
                    (std::string::npos != a.original_url().find("toucan.gentoo.org")) ||
                    (std::string::npos != a.original_url().find("emu.gentoo.org")) ||
                    (std::string::npos != a.original_url().find("alpha.gnu.org")) ||
                    (std::string::npos != a.original_url().find("geocities.com")))
                result << Message(qal_major, "Unreliable host for '" + a.text() + "'");

            else
            {
                if (0 == a.original_url().compare(0, 9, "mirror://"))
                {
                    std::string mirror_host(a.original_url().substr(9));
                    std::string::size_type pos(mirror_host.find('/'));
                    if (std::string::npos == pos)
                        result << Message(qal_major, "Malformed SRC_URI component '" + a.text() + "'");
                    else
                    {
                        mirror_host.erase(pos);
                        RepositoryMirrorsInterface * m(env->package_database()->fetch_repository(
                                    env->main_repository()->name())->mirrors_interface);
                        if (! m)
                            result << Message(qal_major, "Mirror '" + a.text() + "' used, but repository '"
                                    + stringify(env->main_repository()->name())
                                    + "' defines no mirrors interface");
                        else if (! m->is_mirror(mirror_host))
                            result << Message(qal_major, "Unknown mirror '" + mirror_host
                                    + "' for '" + a.text() + "'");
                    }
                }
            }
        }
    };
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
                    e.environment->main_repository()->name());
            tr1::shared_ptr<const VersionMetadata> metadata(
                    e.environment->package_database()->fetch_repository(ee.repository)->version_metadata(ee.name, ee.version));

            try
            {
                Checker checker(result, e.environment);
                metadata->ebuild_interface->src_uri()->accept(checker);
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

