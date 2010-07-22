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

#include <paludis/repositories/e/iuse.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/tribool.hh>
#include <paludis/choice.hh>
#include <paludis/name.hh>
#include <istream>
#include <ostream>

using namespace paludis;
using namespace paludis::erepository;

#include <paludis/repositories/e/iuse-se.cc>

std::pair<ChoiceNameWithPrefix, Tribool>
paludis::erepository::parse_iuse(const std::shared_ptr<const EAPI> &, const std::string & s)
{
    if (s.empty())
        return std::make_pair(ChoiceNameWithPrefix(s), indeterminate);

    if ('-' == s.at(0))
        return std::make_pair(ChoiceNameWithPrefix(s.substr(1)), false);
    if ('+' == s.at(0))
        return std::make_pair(ChoiceNameWithPrefix(s.substr(1)), true);
    return std::make_pair(ChoiceNameWithPrefix(s), indeterminate);
}
