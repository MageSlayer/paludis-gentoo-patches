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
#include <paludis/package_database_entry.hh>
#include <paludis/environment.hh>
#include <paludis/qa/inherits_check.hh>
#include <paludis/util/join.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/config_file.hh>
#include <paludis/util/log.hh>
#include <paludis/util/system.hh>
#include <set>

using namespace paludis;
using namespace paludis::qa;

InheritsCheck::InheritsCheck()
{
}

CheckResult
InheritsCheck::operator() (const EbuildCheckData & e) const
{
    CheckResult result(stringify(e.name) + "-" + stringify(e.version),
            identifier());

    try
    {
        PackageDatabaseEntry ee(e.name, e.version,
                e.environment->package_database()->favourite_repository());
        std::tr1::shared_ptr<const VersionMetadata> metadata(
                e.environment->package_database()->fetch_repository(ee.repository)->version_metadata(ee.name, ee.version));

        std::set<std::string> inherits;
        Tokeniser<delim_kind::AnyOfTag, delim_mode::DelimiterTag> tokeniser(" \t\n");
        tokeniser.tokenise(metadata->ebuild_interface->inherited, std::inserter(inherits, inherits.begin()));

        static std::set<std::string> inherits_blacklist;
        if (inherits_blacklist.empty())
        {
            inherits_blacklist.insert("often-not-been-on-boats");

            try
            {
                LineConfigFile file(FSEntry(getenv_with_default(
                            "PALUDIS_QA_DATA_DIR", DATADIR "/paludis/qa/")) / "inherits_blacklist.txt");
                std::copy(file.begin(), file.end(), std::inserter(
                                inherits_blacklist, inherits_blacklist.end()));
            }
            catch (const Exception & eee)
            {
                Log::get_instance()->message(ll_warning, lc_context,
                        "Cannot load inherits blacklist from inherits_blacklist.txt due to exception '"
                        + eee.message() + "' (" + eee.what() + ")");
            }
        }

        std::set<std::string> bad_inherits;
        std::set_intersection(inherits.begin(), inherits.end(),
                inherits_blacklist.begin(), inherits_blacklist.end(),
                std::inserter(bad_inherits, bad_inherits.begin()));

        if (! bad_inherits.empty())
            result << Message(qal_major, "Deprecated inherits '" + join(bad_inherits.begin(),
                        bad_inherits.end(), "', '") + "'");
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
InheritsCheck::identifier()
{
    static const std::string id("inherits");
    return id;
}

