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

#include <paludis/elike_blocker.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/stringify.hh>
#include <istream>
#include <ostream>

using namespace paludis;

#include <paludis/elike_blocker-se.cc>

std::tuple<ELikeBlockerKind, std::string, std::string>
paludis::split_elike_blocker(const std::string & s)
{
    if ((! s.empty()) && '!' == s.at(0))
    {
        if (s.length() >= 2)
        {
            if ('!' == s.at(1))
                return std::make_tuple(ebk_double_bang, s.substr(0, 2), s.substr(2));
            if ('?' == s.at(1))
                return std::make_tuple(ebk_bang_question, s.substr(0, 2), s.substr(2));
        }

        return std::make_tuple(ebk_single_bang, s.substr(0, 1), s.substr(1));
    }
    else
        return std::make_tuple(ebk_no_block, std::string(""), s);
}

