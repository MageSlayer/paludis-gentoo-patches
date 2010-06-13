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

#include <paludis/resolver/decisions.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>

using namespace paludis;
using namespace paludis::resolver;

namespace paludis
{
#ifdef PALUDIS_NO_DOUBLE_TEMPLATE
    template <>
#endif
    template <typename D_>
    struct Implementation<Decisions<D_> >
    {
    };
}

template <typename D_>
Decisions<D_>::Decisions() :
    PrivateImplementationPattern<Decisions<D_> >(new Implementation<Decisions<D_> >)
{
}

template <typename D_>
Decisions<D_>::~Decisions()
{
}

template class Decisions<UnableToMakeDecision>;
template class Decisions<ChangesToMakeDecision>;
template class Decisions<ChangeOrRemoveDecision>;

