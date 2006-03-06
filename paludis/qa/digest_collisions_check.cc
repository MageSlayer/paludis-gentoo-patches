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

#include <paludis/qa/digest_collisions_check.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/smart_record.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/hashed_containers.hh>
#include <map>
#include <fstream>

using namespace paludis;
using namespace paludis::qa;

namespace
{
    enum DigestDataKeys
    {
        de_md5,
        de_owner
    };

    struct DigestDataTag :
        SmartRecordTag<comparison_mode::NoComparisonTag, void>,
        SmartRecordKeys<DigestDataKeys, 2>,
        SmartRecordKey<de_md5, std::string>,
        SmartRecordKey<de_owner, FSEntry>
    {
    };

    typedef MakeSmartRecord<DigestDataTag>::Type DigestData;

    static MakeHashedMap<std::string, DigestData>::Type digests;
}

DigestCollisionsCheck::DigestCollisionsCheck()
{
}

CheckResult
DigestCollisionsCheck::operator() (const FSEntry & d) const
{
    CheckResult result(d, identifier());

    for (DirIterator i(d / "files") ; i != DirIterator() ; ++i)
    {
        if (! IsFileWithExtension("digest-", "")(*i))
            continue;

        std::fstream f(stringify(*i).c_str());
        std::string line;
        Tokeniser<delim_kind::AnyOfTag, delim_mode::DelimiterTag> tokeniser(" \t\n");
        while (std::getline(f, line))
        {
            std::vector<std::string> entries;
            tokeniser.tokenise(line, std::back_inserter(entries));

            if (entries.size() != 4)
                continue;

            if ("MD5" != entries.at(0))
                continue;

            MakeHashedMap<std::string, DigestData>::Type::iterator existing(
                    digests.find(entries.at(2)));
            if (digests.end() != existing)
            {
                if (entries.at(1) != existing->second.get<de_md5>())
                    result << Message(qal_major, "Digest conflict on '" + entries.at(2) +
                            "' in '" + stringify(*i) +
                            "' (original was '" + stringify(existing->second.get<de_owner>()) + "')");
            }
            else
                digests.insert(std::make_pair(entries.at(2), DigestData(entries.at(1), *i)));
        }
    }

    return result;
}

const std::string &
DigestCollisionsCheck::identifier()
{
    static const std::string id("digest collisions");
    return id;
}


