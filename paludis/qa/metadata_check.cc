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

#include "metadata_check.hh"
#include <paludis/pstream.hh>

using namespace paludis;
using namespace paludis::qa;

MetadataCheck::MetadataCheck()
{
}

CheckResult
MetadataCheck::operator() (const FSEntry & f) const
{
    CheckResult result(f, identifier());

    if (f.basename() != "metadata.xml")
        result << Message(qal_skip, "Not a metadata.xml file");
    else if (! f.is_regular_file())
        result << Message(qal_major, "Not a regular file");
    else
    {
        PStream xmllint("xmllint --noout --valid '" + stringify(f) + "'");
        if (0 != xmllint.exit_status())
            result << Message(qal_major, "XML validation failed");
    }

    return result;
}

const std::string &
MetadataCheck::identifier()
{
    static const std::string id("metadata");
    return id;
}

