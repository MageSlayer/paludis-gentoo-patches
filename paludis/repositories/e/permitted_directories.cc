/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2011 Ciaran McCreesh
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

#include <paludis/repositories/e/permitted_directories.hh>
#include <paludis/util/pimp-impl.hh>

#include <list>
#include <string>

using namespace paludis;
using namespace paludis::erepository;

namespace paludis
{
    template <>
    struct Imp<PermittedDirectories>
    {
        std::list<std::pair<FSPath, bool> > rules;
    };
}

PermittedDirectories::PermittedDirectories() :
    _imp()
{
}

PermittedDirectories::~PermittedDirectories() = default;

void
PermittedDirectories::add(const FSPath & p, bool b)
{
    _imp->rules.push_back(std::make_pair(p, b));
}

bool
PermittedDirectories::permit(const FSPath & p) const
{
    bool result(true);

    for (auto r(_imp->rules.begin()), r_end(_imp->rules.end()) ;
            r != r_end ; ++r)
        if (p.starts_with(r->first))
            result = r->second;

    return result;
}

