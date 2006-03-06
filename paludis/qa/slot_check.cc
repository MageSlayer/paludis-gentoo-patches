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

#include <paludis/qa/slot_check.hh>

using namespace paludis;
using namespace paludis::qa;

SlotCheck::SlotCheck()
{
}

CheckResult
SlotCheck::operator() (const EbuildCheckData & e) const
{
    CheckResult result(stringify(e.get<ecd_name>()) + "-" + stringify(e.get<ecd_version>()),
            identifier());

    try
    {
        PackageDatabaseEntry ee(e.get<ecd_name>(), e.get<ecd_version>(),
                e.get<ecd_environment>()->package_database()->favourite_repository());
        VersionMetadata::ConstPointer metadata(
                e.get<ecd_environment>()->package_database()->fetch_metadata(ee));

        try
        {
            SlotName slot(metadata->get(vmk_slot));
        }
        catch (const NameError &)
        {
            result << Message(qal_fatal, "Invalid SLOT value");
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
SlotCheck::identifier()
{
    static const std::string id("slot");
    return id;
}


