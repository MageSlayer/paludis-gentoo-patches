/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2010, 2011 Ciaran McCreesh
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

#include <paludis/repositories/e/vdb_id.hh>
#include <paludis/repositories/e/e_key.hh>
#include <paludis/repositories/e/vdb_contents_tokeniser.hh>
#include <paludis/util/fs_path.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/log.hh>
#include <paludis/util/safe_ifstream.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/timestamp.hh>
#include <paludis/contents.hh>
#include <paludis/literal_metadata_key.hh>

#include <vector>

using namespace paludis;
using namespace paludis::erepository;

VDBID::VDBID(const QualifiedPackageName & q, const VersionSpec & v,
        const Environment * const e,
        const RepositoryName & r,
        const FSPath & f) :
    EInstalledRepositoryID(q, v, e, r, f)
{
}

std::string
VDBID::fs_location_raw_name() const
{
    return "VDBDIR";
}

std::string
VDBID::fs_location_human_name() const
{
    return "VDB Directory";
}

std::string
VDBID::contents_filename() const
{
    return "CONTENTS";
}

const std::shared_ptr<const Contents>
VDBID::contents() const
{
    // NOTE(compnerd) VDB does not support parts
    static const std::string kNoPart = "";

    FSPath contents_location(fs_location_key()->parse_value() / "CONTENTS");
    Context context("When creating contents from '" + stringify(contents_location) + "':");

    auto value(std::make_shared<Contents>());

    if (! contents_location.stat().is_regular_file_or_symlink_to_regular_file())
    {
        Log::get_instance()->message("e.contents.not_a_file", ll_warning, lc_context) << "Could not read CONTENTS file '" <<
            contents_location << "'";
        return value;
    }

    SafeIFStream ff(contents_location);

    std::string line;
    unsigned line_number(0);
    while (std::getline(ff, line))
    {
        ++line_number;

        std::vector<std::string> tokens;
        if (! VDBContentsTokeniser::tokenise(line, std::back_inserter(tokens)))
        {
            Log::get_instance()->message("e.contents.broken", ll_warning, lc_context) << "CONTENTS has broken line '" <<
                line_number << "', skipping";
            continue;
        }

        if ("obj" == tokens.at(0))
        {
            auto e(std::make_shared<ContentsFileEntry>(FSPath(tokens.at(1)),
                                                       kNoPart));
            e->add_metadata_key(std::make_shared<LiteralMetadataTimeKey>("mtime", "mtime", mkt_normal,
                            Timestamp(destringify<time_t>(tokens.at(3)), 0)));
            e->add_metadata_key(std::make_shared<LiteralMetadataValueKey<std::string>>("md5", "md5", mkt_normal, tokens.at(2)));
            value->add(e);
        }
        else if ("dir" == tokens.at(0))
        {
            std::shared_ptr<ContentsEntry> e(std::make_shared<ContentsDirEntry>(FSPath(tokens.at(1))));
            value->add(e);
        }
        else if ("sym" == tokens.at(0))
        {
            auto e(std::make_shared<ContentsSymEntry>(FSPath(tokens.at(1)),
                                                      tokens.at(2), kNoPart));
            e->add_metadata_key(std::make_shared<LiteralMetadataTimeKey>("mtime", "mtime", mkt_normal,
                            Timestamp(destringify<time_t>(tokens.at(3)), 0)));
            value->add(e);
        }
        else if ("misc" == tokens.at(0) || "fif" == tokens.at(0) || "dev" == tokens.at(0))
            value->add(std::shared_ptr<ContentsEntry>(std::make_shared<ContentsOtherEntry>(FSPath(tokens.at(1)))));
        else
            Log::get_instance()->message("e.contents.unknown", ll_warning, lc_context) << "CONTENTS has unsupported entry type '" <<
                tokens.at(0) << "', skipping";
    }

    return value;
}

