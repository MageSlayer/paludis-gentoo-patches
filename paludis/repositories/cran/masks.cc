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

#include "masks.hh"
#include <paludis/util/pimp-impl.hh>

using namespace paludis;
using namespace paludis::cranrepository;

namespace paludis
{
    template <>
    struct Imp<BrokenMask>
    {
        const char key;
        const std::string description;
        const std::string explanation;

        Imp(const char k, const std::string & d, const std::string & e) :
            key(k),
            description(d),
            explanation(e)
        {
        }
    };
}

BrokenMask::BrokenMask(const char c, const std::string & d, const std::string & e) :
    _imp(c, d, e)
{
}

BrokenMask::~BrokenMask()
{
}

char
BrokenMask::key() const
{
    return _imp->key;
}

const std::string
BrokenMask::description() const
{
    return _imp->description;
}

const std::string
BrokenMask::explanation() const
{
    return _imp->explanation;
}

