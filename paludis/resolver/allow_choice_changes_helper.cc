/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010, 2011 Ciaran McCreesh
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

#include <paludis/resolver/allow_choice_changes_helper.hh>
#include <paludis/util/pimp-impl.hh>

using namespace paludis;
using namespace paludis::resolver;

namespace paludis
{
    template <>
    struct Imp<AllowChoiceChangesHelper>
    {
        bool allow_choice_changes;

        Imp() :
            allow_choice_changes(true)
        {
        }
    };
}

AllowChoiceChangesHelper::AllowChoiceChangesHelper(const Environment * const) :
    _imp()
{
}

AllowChoiceChangesHelper::~AllowChoiceChangesHelper() = default;

void
AllowChoiceChangesHelper::set_allow_choice_changes(const bool v)
{
    _imp->allow_choice_changes = v;
}

bool
AllowChoiceChangesHelper::operator() (const std::shared_ptr<const Resolution> &) const
{
    return _imp->allow_choice_changes;
}

namespace paludis
{
    template class Pimp<AllowChoiceChangesHelper>;
}
