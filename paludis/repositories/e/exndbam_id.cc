/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#include <paludis/repositories/e/exndbam_id.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/contents.hh>
#include <paludis/ndbam.hh>
#include <functional>

using namespace paludis;
using namespace paludis::erepository;

ExndbamID::ExndbamID(const QualifiedPackageName & q, const VersionSpec & v,
        const Environment * const e,
        const RepositoryName & r,
        const FSPath & f, const NDBAM * const n) :
    EInstalledRepositoryID(q, v, e, r, f),
    _ndbam(n)
{
}

std::string
ExndbamID::fs_location_raw_name() const
{
    return "EXNDBAMDIR";
}

std::string
ExndbamID::fs_location_human_name() const
{
    return "Exndbam Directory";
}

std::string
ExndbamID::contents_filename() const
{
    return "contents";
}

const std::shared_ptr<const Contents>
ExndbamID::contents() const
{
    auto v(std::make_shared<Contents>());
    _ndbam->parse_contents(*this,
            std::bind(&Contents::add, v.get(), std::placeholders::_1),
            std::bind(&Contents::add, v.get(), std::placeholders::_1),
            std::bind(&Contents::add, v.get(), std::placeholders::_1)
            );
    return v;
}

