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

#include <paludis/qa/description_check.hh>
#include <paludis/package_database_entry.hh>
#include <paludis/environment.hh>
#include <strings.h>

using namespace paludis;
using namespace paludis::qa;

DescriptionCheck::DescriptionCheck()
{
}

CheckResult
DescriptionCheck::operator() (const EbuildCheckData & e) const
{
    CheckResult result(stringify(e.name) + "-" + stringify(e.version),
            identifier());

    try
    {
        PackageDatabaseEntry ee(e.name, e.version,
                e.environment->package_database()->favourite_repository());
        VersionMetadata::ConstPointer metadata(
                e.environment->package_database()->fetch_repository(ee.repository)->version_metadata(ee.name, ee.version));

        const std::string::size_type length(metadata->description.length());
        if (0 == length)
            result << Message(qal_major, "DESCRIPTION unset or empty");
        else if (0 == strcasecmp(e.name.package.data().c_str(),
                    metadata->description.c_str()))
            result << Message(qal_major, "DESCRIPTION equal to $PN? You can do better than that.");
        else if (std::string::npos != metadata->description.find("Based on the") &&
                std::string::npos != metadata->description.find("eclass"))
            result << Message(qal_major, "DESCRIPTION is about as useful as a chocolate teapot");
        else if (length < 10)
            result << Message(qal_minor, "DESCRIPTION suspiciously short (" + stringify(length) + ")");
        else if (length > 300)
            result << Message(qal_minor, "DESCRIPTION written by Duncan? (" + stringify(length) + ")");
        else if (length > 120)
            result << Message(qal_minor, "DESCRIPTION too long (" + stringify(length) + ")");

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
DescriptionCheck::identifier()
{
    static const std::string id("description");
    return id;
}

