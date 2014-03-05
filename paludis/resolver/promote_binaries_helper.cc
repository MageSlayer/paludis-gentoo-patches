/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010, 2011 Ciaran McCreesh
 * Copyright (c) 2014 Dimitry Ishenko
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

#include <paludis/resolver/promote_binaries_helper.hh>
#include <paludis/resolver/promote_binaries.hh>
#include <paludis/resolver/selection_with_promotion.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/selection.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/environment.hh>

using namespace paludis;
using namespace paludis::resolver;

namespace paludis
{
    template <>
    struct Imp<PromoteBinariesHelper>
    {
        const Environment * const env;
        PromoteBinaries promote_binaries;

        Imp(const Environment * const e) :
            env(e),
            promote_binaries(pb_never)
        {
        }
    };
}

PromoteBinariesHelper::PromoteBinariesHelper(const Environment * const e) :
    _imp(e)
{
}

PromoteBinariesHelper::~PromoteBinariesHelper() = default;

void
PromoteBinariesHelper::set_promote_binaries(const PromoteBinaries v)
{
    _imp->promote_binaries = v;
}

Selection
PromoteBinariesHelper::operator() (const FilteredGenerator & fg) const
{
    switch(_imp->promote_binaries)
    {
    case pb_if_same:
        return selection::AllVersionsSortedWithPromotion(fg);

    default:
        return selection::AllVersionsSorted(fg);
    }
}

namespace paludis
{
    template class Pimp<PromoteBinariesHelper>;
}
