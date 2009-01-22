/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009 Ciaran McCreesh
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

#include <paludis/util/tribool.hh>

using namespace paludis;

Tribool::Tribool() :
    _value(v_false)
{
}

Tribool::Tribool(const bool b) :
    _value(b ? v_true : v_false)
{
}

Tribool::Tribool(TriboolIndeterminateValueType) :
    _value(v_indeterminate)
{
}

bool
Tribool::is_true() const
{
    return v_true == _value;
}

bool
Tribool::is_false() const
{
    return v_false == _value;
}

bool
Tribool::is_indeterminate() const
{
    return v_indeterminate == _value;
}

NoType<0u> *
paludis::indeterminate(const NoType<0u> * const)
{
    return 0;
}

