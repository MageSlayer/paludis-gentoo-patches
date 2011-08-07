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

#include <paludis/resolver/remove_hidden_helper.hh>

#include <paludis/util/pimp-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/util/options.hh>

#include <paludis/dep_spec.hh>
#include <paludis/package_dep_spec_collection.hh>

#include <list>

using namespace paludis;
using namespace paludis::resolver;

namespace paludis
{
    template <>
    struct Imp<RemoveHiddenHelper>
    {
        const Environment * const env;
        PackageDepSpecCollection hide_specs;

        Imp(const Environment * const e) :
            env(e),
            hide_specs(make_null_shared_ptr())
        {
        }
    };
}

RemoveHiddenHelper::RemoveHiddenHelper(const Environment * const e) :
    _imp(e)
{
}

RemoveHiddenHelper::~RemoveHiddenHelper() = default;

void
RemoveHiddenHelper::add_hide_spec(const PackageDepSpec & spec)
{
    _imp->hide_specs.insert(spec);
}

const std::shared_ptr<const PackageIDSequence>
RemoveHiddenHelper::operator() (
        const std::shared_ptr<const PackageIDSequence> & candidates) const
{
    auto result(std::make_shared<PackageIDSequence>());

    for (auto c(candidates->begin()), c_end(candidates->end()) ;
            c != c_end ; ++c)
        if (! _imp->hide_specs.match_any(_imp->env, *c, { }))
            result->push_back(*c);

    return result;
}

namespace paludis
{
    template class Pimp<RemoveHiddenHelper>;
}
