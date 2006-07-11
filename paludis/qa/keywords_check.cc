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

#include <paludis/qa/keywords_check.hh>

using namespace paludis;
using namespace paludis::qa;

KeywordsCheck::KeywordsCheck()
{
}

CheckResult
KeywordsCheck::operator() (const EbuildCheckData & e) const
{
    CheckResult result(stringify(e.get<ecd_name>()) + "-" + stringify(e.get<ecd_version>()),
            identifier());

    try
    {
        PackageDatabaseEntry ee(e.get<ecd_name>(), e.get<ecd_version>(),
                e.get<ecd_environment>()->package_database()->favourite_repository());
        VersionMetadata::ConstPointer metadata(
                e.get<ecd_environment>()->package_database()->fetch_repository(ee.get<pde_repository>())->version_metadata(ee.get<pde_name>(), ee.get<pde_version>()));

        try
        {
            std::set<KeywordName> keywords(metadata->begin_keywords(), metadata->end_keywords());

            if (keywords.end() != keywords.find(KeywordName("-*")) &&
                    keywords.size() == 1)
                result << Message(qal_major, "-* abuse (use package.mask and keyword properly)");

            else if (keywords.empty())
                result << Message(qal_major, "KEYWORDS empty");

        }
        catch (const NameError &)
        {
            result << Message(qal_major, "Bad entries in KEYWORDS");
        }

        if (! metadata->get_ebuild_interface()->get<evm_keywords>().empty())
            result << Message(qal_major, "KEYWORDS was altered by an eclass");
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
KeywordsCheck::identifier()
{
    static const std::string id("keywords");
    return id;
}

