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

#include <algorithm>
#include <iterator>
#include <paludis/qa/inherits_check.hh>
#include <paludis/util/join.hh>
#include <paludis/util/tokeniser.hh>
#include <set>

using namespace paludis;
using namespace paludis::qa;

InheritsCheck::InheritsCheck()
{
}

CheckResult
InheritsCheck::operator() (const EbuildCheckData & e) const
{
    CheckResult result(stringify(e.get<ecd_name>()) + "-" + stringify(e.get<ecd_version>()),
            identifier());

    try
    {
        PackageDatabaseEntry ee(e.get<ecd_name>(), e.get<ecd_version>(),
                e.get<ecd_environment>()->package_database()->favourite_repository());
        VersionMetadata::ConstPointer metadata(
                e.get<ecd_environment>()->package_database()->fetch_metadata(ee));

        std::set<std::string> inherits;
        Tokeniser<delim_kind::AnyOfTag, delim_mode::DelimiterTag> tokeniser(" \t\n");
        tokeniser.tokenise(metadata->get(vmk_inherited), std::inserter(inherits, inherits.begin()));

        static std::set<std::string> inherits_blacklist;
        if (inherits_blacklist.empty())
        {
            inherits_blacklist.insert("gcc");
            inherits_blacklist.insert("kmod");
            inherits_blacklist.insert("kernel-mod");
            inherits_blacklist.insert("gtk-engines");
            inherits_blacklist.insert("gtk-engines2");
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
    catch (const Exception & e)
    {
        result << Message(qal_fatal, "Caught Exception '" + e.message() + "' ("
                + e.what() + ")");
    }

    return result;
}

const std::string &
InheritsCheck::identifier()
{
    static const std::string id("inherits");
    return id;
}

