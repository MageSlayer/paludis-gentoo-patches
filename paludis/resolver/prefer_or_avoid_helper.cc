/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010 Ciaran McCreesh
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

#include <paludis/resolver/prefer_or_avoid_helper.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/tribool.hh>
#include <paludis/name.hh>
#include <unordered_set>

using namespace paludis;
using namespace paludis::resolver;

namespace paludis
{
    template <>
    struct Imp<PreferOrAvoidHelper>
    {
        const Environment * const env;
        std::unordered_set<QualifiedPackageName, Hash<QualifiedPackageName> > prefer_names;
        std::unordered_set<QualifiedPackageName, Hash<QualifiedPackageName> > avoid_names;

        Imp(const Environment * const e) :
            env(e)
        {
        }
    };
}

PreferOrAvoidHelper::PreferOrAvoidHelper(const Environment * const e) :
    Pimp<PreferOrAvoidHelper>(e)
{
}

PreferOrAvoidHelper::~PreferOrAvoidHelper() = default;

void
PreferOrAvoidHelper::add_prefer_name(const QualifiedPackageName & name)
{
    _imp->prefer_names.insert(name);
}

void
PreferOrAvoidHelper::add_avoid_name(const QualifiedPackageName & name)
{
    _imp->avoid_names.insert(name);
}

Tribool
PreferOrAvoidHelper::operator() (const QualifiedPackageName & n) const
{
    if (_imp->prefer_names.end() != _imp->prefer_names.find(n))
        return true;

    if (_imp->avoid_names.end() != _imp->avoid_names.find(n))
        return false;

    return indeterminate;
}

template class Pimp<PreferOrAvoidHelper>;

