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
#include <paludis/resolver/decision.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/stringify.hh>
#include <list>

using namespace paludis;
using namespace paludis::resolver;

namespace paludis
{
#ifdef PALUDIS_NO_DOUBLE_TEMPLATE
    template <>
#endif
    template <typename Decision_>
    struct Implementation<Decisions<Decision_> >
    {
        std::list<std::tr1::shared_ptr<const Decision_> > values;
    };
}

template <typename Decision_>
Decisions<Decision_>::Decisions() :
    PrivateImplementationPattern<Decisions<Decision_> >(new Implementation<Decisions<Decision_> >)
{
}

template <typename Decision_>
Decisions<Decision_>::~Decisions()
{
}

template <typename Decision_>
void
Decisions<Decision_>::push_back(const std::tr1::shared_ptr<const Decision_> & d)
{
    _imp->values.push_back(d);
}

template <typename Decision_>
void
Decisions<Decision_>::cast_push_back(const std::tr1::shared_ptr<const Decision> & d)
{
    if (! simple_visitor_cast<const Decision_>(*d))
        throw InternalError(PALUDIS_HERE, "Wrong Decision type");
    push_back(std::tr1::static_pointer_cast<const Decision_>(d));
}

template class Decisions<UnableToMakeDecision>;
template class Decisions<ChangesToMakeDecision>;
template class Decisions<ChangeOrRemoveDecision>;

