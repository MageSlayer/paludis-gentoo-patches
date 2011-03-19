/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#include <paludis/mask.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/exception.hh>
#include <istream>
#include <ostream>

using namespace paludis;

#include <paludis/mask-se.cc>

Mask::~Mask()
{
}

const std::string
UserMask::token() const
{
    return "user";
}

namespace
{
    struct TokenGetter
    {
        std::string visit(const UserMask & m) const
        {
            return m.token();
        }

        std::string visit(const RepositoryMask & m) const
        {
            return m.token();
        }

        std::string visit(const UnacceptedMask &) const
        {
            return "";
        }

        std::string visit(const UnsupportedMask &) const
        {
            return "";
        }

        std::string visit(const AssociationMask &) const
        {
            return "";
        }
    };
}

const std::string
paludis::get_mask_token(const Mask & m)
{
    return m.accept_returning<std::string>(TokenGetter{});
}

