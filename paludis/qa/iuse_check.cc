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

#include <paludis/qa/iuse_check.hh>
#include <set>
#include <algorithm>
#include <paludis/package_database_entry.hh>
#include <paludis/environment.hh>
#include <paludis/util/join.hh>
#include <paludis/util/tokeniser.hh>

using namespace paludis;
using namespace paludis::qa;

IuseCheck::IuseCheck()
{
}

CheckResult
IuseCheck::operator() (const EbuildCheckData & e) const
{
    CheckResult result(stringify(e.name) + "-" + stringify(e.version),
            identifier());

    try
    {
        PackageDatabaseEntry ee(e.name, e.version,
                e.environment->package_database()->favourite_repository());
        VersionMetadata::ConstPointer metadata(
                e.environment->package_database()->fetch_repository(ee.repository)->version_metadata(ee.name, ee.version));

        try
        {
            std::set<UseFlagName> iuse;
            WhitespaceTokeniser::get_instance()->tokenise(metadata->get_ebuild_interface()->
                iuse, create_inserter<UseFlagName>(std::inserter(iuse, iuse.begin())));


            static std::set<UseFlagName> iuse_blacklist;
            if (iuse_blacklist.empty())
            {
                iuse_blacklist.insert(UseFlagName("gtk2"));
                iuse_blacklist.insert(UseFlagName("xml2"));
                iuse_blacklist.insert(UseFlagName("oggvorbis"));
            }

            std::set<UseFlagName> bad_iuse;
            std::set_intersection(iuse.begin(), iuse.end(),
                    iuse_blacklist.begin(), iuse_blacklist.end(),
                    std::inserter(bad_iuse, bad_iuse.begin()));

            if (! bad_iuse.empty())
                result << Message(qal_minor, "Deprecated IUSEs '" + join(bad_iuse.begin(),
                            bad_iuse.end(), "', '") + "'");
        }
        catch (const NameError & e)
        {
            result << Message(qal_fatal, "Bad IUSE entry name: " + e.message() + " ("
                    + e.what() + ")");
        }
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
IuseCheck::identifier()
{
    static const std::string id("iuse");
    return id;
}


