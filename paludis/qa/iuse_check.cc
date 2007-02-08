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

#include <paludis/qa/iuse_check.hh>
#include <set>
#include <algorithm>
#include <paludis/package_database_entry.hh>
#include <paludis/environment.hh>
#include <paludis/config_file.hh>
#include <paludis/util/join.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/log.hh>
#include <paludis/util/system.hh>

using namespace paludis;
using namespace paludis::qa;

IuseCheck::IuseCheck()
{
}

CheckResult
IuseCheck::operator() (const EbuildCheckData & e) const
{
    Context context("When performing iuse check on '" + stringify(e.name) +
            "-" + stringify(e.version) + "'");

    CheckResult result(stringify(e.name) + "-" + stringify(e.version),
            identifier());

    try
    {
        PackageDatabaseEntry ee(e.name, e.version,
                e.environment->package_database()->favourite_repository());
        std::tr1::shared_ptr<const VersionMetadata> metadata(
                e.environment->package_database()->fetch_repository(ee.repository)->
                version_metadata(ee.name, ee.version));

        try
        {
            std::set<UseFlagName> iuse;
            WhitespaceTokeniser::get_instance()->tokenise(metadata->ebuild_interface->
                iuse, create_inserter<UseFlagName>(std::inserter(iuse, iuse.begin())));


            static std::set<UseFlagName> iuse_blacklist;
            if (iuse_blacklist.empty())
            {
                iuse_blacklist.insert(UseFlagName("OFTEN_NOT_BEEN_ON_BOATS"));

                try
                {
                    LineConfigFile iuse_blacklist_file(FSEntry(getenv_with_default(
                                "PALUDIS_QA_DATA_DIR", DATADIR "/paludis/qa/")) / "iuse_blacklist.txt");
                    std::copy(iuse_blacklist_file.begin(), iuse_blacklist_file.end(),
                            create_inserter<UseFlagName>(std::inserter(iuse_blacklist, iuse_blacklist.end())));
                }
                catch (const Exception & eee)
                {
                    Log::get_instance()->message(ll_warning, lc_context,
                            "Cannot load IUSE blacklist from iuse_check.txt due to exception '"
                            + eee.message() + "' (" + eee.what() + ")");
                }
            }

            std::set<UseFlagName> bad_iuse;
            std::set_intersection(iuse.begin(), iuse.end(),
                    iuse_blacklist.begin(), iuse_blacklist.end(),
                    std::inserter(bad_iuse, bad_iuse.begin()));

            if (! bad_iuse.empty())
                result << Message(qal_minor, "Deprecated IUSEs '" + join(bad_iuse.begin(),
                            bad_iuse.end(), "', '") + "'");

            for (std::set<UseFlagName>::iterator i(iuse.begin()), i_end(iuse.end()) ; i != i_end ; ++i)
                if ("" == e.environment->package_database()->fetch_repository(ee.repository)->use_interface->
                        describe_use_flag(*i, &ee))
                    result << Message(qal_minor, "Use flag '" + stringify(*i) + "' has no description");
        }
        catch (const NameError & err)
        {
            result << Message(qal_fatal, "Bad IUSE entry name: " + err.message() + " ("
                    + err.what() + ")");
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
IuseCheck::identifier()
{
    static const std::string id("iuse");
    return id;
}


