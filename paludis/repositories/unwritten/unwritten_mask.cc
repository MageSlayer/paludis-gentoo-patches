/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2010 Ciaran McCreesh
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

#include <paludis/repositories/unwritten/unwritten_mask.hh>

using namespace paludis;
using namespace paludis::unwritten_repository;

char
UnwrittenMask::key() const
{
    return 'X';
}

const std::string
UnwrittenMask::description() const
{
    return "unwritten";
}

const std::string
UnwrittenMask::explanation() const
{
    return "Package has not been written yet";
}

char
GraveyardMask::key() const
{
    return 'G';
}

const std::string
GraveyardMask::description() const
{
    return "graveyard";
}

const std::string
GraveyardMask::explanation() const
{
    return "Package has been deleted";
}

