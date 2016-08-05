/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2013 Saleem Abdulrasool <compnerd@compnerd.org>
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

#include <paludis/name.hh>
#include <paludis/partitioning.hh>
#include <paludis/util/pimp-impl.hh>

#include <map>
#include <utility>
#include <algorithm>

using namespace paludis;

namespace paludis
{
    template <>
    struct Imp<Partitioning>
    {
        std::vector<std::pair<FSPath, PartName>> parts;
    };
}

Partitioning::Partitioning() :
    _imp()
{
}

Partitioning::~Partitioning() = default;

void
Partitioning::mark(const std::vector<FSPath> & paths, const PartName & name)
{
    for (const auto & path : paths)
        _imp->parts.push_back(std::make_pair(path, name));
}

PartName
Partitioning::classify(const FSPath & path) const
{
    for (auto part = _imp->parts.rbegin(), end = _imp->parts.rend();
         part != end; ++part)
        if (path.starts_with(part->first))
            return part->second;

    return PartName("");
}

bool
Partitioning::is_partitioned(const FSPath & path) const
{
    for (auto part = _imp->parts.rbegin(), end = _imp->parts.rend();
            part != end; ++part)
        if (part->first.starts_with(path) && ! part->second.value().empty())
            return true;
    return false;
}

