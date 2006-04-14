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

#include <ctime>
#include <fstream>
#include <paludis/qa/metadata_check.hh>
#include <paludis/util/pstream.hh>
#include <paludis/util/system.hh>

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
        FSEntry dtd(FSEntry(getenv_or_error("HOME")) / ".paludis");
        if (! dtd.exists())
            throw ConfigurationError("~/.paludis/ does not exist, please create it");

        dtd /= "cache";
        if (! dtd.exists())
            if (0 != mkdir(stringify(dtd).c_str(), 0755))
                throw ConfigurationError("~/.paludis/cache/ does not exist and cannot be created");

        dtd /= "metadata.dtd";
        if (! dtd.exists() || dtd.mtime() > (std::time(0) + (24 * 60 * 60)))
        {
            PStream wget("wget -O- http://www.gentoo.org/dtd/metadata.dtd");

            // see \ref EffSTL item 29
            std::string dtd_data((std::istreambuf_iterator<char>(wget)),
                    std::istreambuf_iterator<char>());
            if (0 != wget.exit_status())
                throw ConfigurationError("Couldn't get a new metadata.dtd");

            std::ofstream dtd_file(stringify(dtd).c_str());
            dtd_file << dtd_data;
            if (! dtd_file)
                throw ConfigurationError("Error writing to '" + stringify(dtd) +
                        "' -- you should remove this file manually before continuing");
        }

        PStream xmllint("xmllint --noout --nonet --dtdvalid '"
                + stringify(dtd) + "' '" + stringify(f) + "' 2>&1");

        std::string s;
        while (std::getline(xmllint, s))
            ;

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

