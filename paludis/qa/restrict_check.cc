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

#include <algorithm>
#include <iterator>
#include <paludis/qa/restrict_check.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/package_database_entry.hh>
#include <paludis/environment.hh>
#include <paludis/qa/qa_environment.hh>
#include <paludis/repositories/portage/portage_repository.hh>
#include <set>

using namespace paludis;
using namespace paludis::qa;

RestrictCheck::RestrictCheck()
{
}

CheckResult
RestrictCheck::operator() (const EbuildCheckData & e) const
{
    CheckResult result(stringify(e.name) + "-" + stringify(e.version),
            identifier());

    try
    {
        PackageDatabaseEntry ee(e.name, e.version,
                e.environment->portage_repository()->name());
        std::tr1::shared_ptr<const VersionMetadata> metadata(
                e.environment->package_database()->fetch_repository(ee.repository)->version_metadata(ee.name, ee.version));

        std::set<std::string> restricts;
        Tokeniser<delim_kind::AnyOfTag, delim_mode::DelimiterTag> tokeniser(" \t\n");
        tokeniser.tokenise(metadata->ebuild_interface->restrict_string,
                std::inserter(restricts, restricts.begin()));

        static std::set<std::string> allowed_restricts;
        if (allowed_restricts.empty())
        {
            allowed_restricts.insert("fetch");
            allowed_restricts.insert("mirror");
            allowed_restricts.insert("nomirror");
            allowed_restricts.insert("primaryuri");
            allowed_restricts.insert("nostrip");
            allowed_restricts.insert("strip");
            allowed_restricts.insert("sandbox");
            allowed_restricts.insert("userpriv");
            allowed_restricts.insert("test");
        }

        std::set<std::string> unknown;
        std::set_difference(restricts.begin(), restricts.end(),
                allowed_restricts.begin(), allowed_restricts.end(),
                std::inserter(unknown, unknown.begin()));

        if (! unknown.empty())
            result << Message(qal_major, "Unrecognised RESTRICT values '"
                    + join(unknown.begin(), unknown.end(), "', '") + "'");

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
RestrictCheck::identifier()
{
    static const std::string id("restrict");
    return id;
}



