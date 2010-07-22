/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2010 Ciaran McCreesh
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

using namespace paludis;
using namespace paludis::erepository;

VDBID::VDBID(const QualifiedPackageName & q, const VersionSpec & v,
        const Environment * const e,
        const std::shared_ptr<const Repository> & r,
        const FSEntry & f) :
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

std::shared_ptr<MetadataValueKey<std::shared_ptr<const Contents> > >
VDBID::make_contents_key() const
{
    return std::make_shared<EContentsKey>(shared_from_this(), "CONTENTS", "Contents",
                fs_location_key()->value() / "CONTENTS", mkt_internal);
}

