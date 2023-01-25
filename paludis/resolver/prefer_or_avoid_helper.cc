/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010, 2011, 2014 Ciaran McCreesh
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
#include <paludis/util/options.hh>
#include <paludis/name.hh>
#include <paludis/dep_spec.hh>
#include <paludis/match_package.hh>
#include <unordered_set>
#include <list>

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
        std::list<std::shared_ptr<const PackageIDSequence> > prefer_matching;
        std::list<std::shared_ptr<const PackageIDSequence> > avoid_matching;

        Imp(const Environment * const e) :
            env(e)
        {
        }
    };
}

PreferOrAvoidHelper::PreferOrAvoidHelper(const Environment * const e) :
    _imp(e)
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

void
PreferOrAvoidHelper::add_prefer_matching(const std::shared_ptr<const PackageIDSequence> & s)
{
    _imp->prefer_matching.push_back(s);
}

void
PreferOrAvoidHelper::add_avoid_matching(const std::shared_ptr<const PackageIDSequence> & s)
{
    _imp->avoid_matching.push_back(s);
}

Tribool
PreferOrAvoidHelper::operator() (const PackageDepSpec & s, const std::shared_ptr<const PackageID> & id) const
{
    if (s.package_ptr())
    {
        if (_imp->prefer_names.end() != _imp->prefer_names.find(*s.package_ptr()))
            return true;

        if (_imp->avoid_names.end() != _imp->avoid_names.find(*s.package_ptr()))
            return false;
    }

    for (const auto & prefered_ids : _imp->prefer_matching)
    {
        bool all(true);
        for (const auto & prefered_id : *prefered_ids)
            if (! match_package(*_imp->env, s, prefered_id, id, { }))
            {
                all = false;
                break;
            }

        if (all)
            return true;
    }

    for (const auto & avoid_ids : _imp->avoid_matching)
    {
        for (const auto & avoid_id : *avoid_ids)
            if (match_package(*_imp->env, s, avoid_id, id, { }))
                return false;
    }

    return indeterminate;
}

namespace paludis
{
    template class Pimp<PreferOrAvoidHelper>;
}
