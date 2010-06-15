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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_STRONGLY_CONNECTED_COMPONENT_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_STRONGLY_CONNECTED_COMPONENT_HH 1

#include <paludis/resolver/strongly_connected_component-fwd.hh>
#include <paludis/resolver/resolvent-fwd.hh>
#include <paludis/util/named_value.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <tr1/memory>

namespace paludis
{
    namespace n
    {
        typedef Name<struct nodes_name> nodes;
        typedef Name<struct requirements_name> requirements;
    }

    namespace resolver
    {
        struct StronglyConnectedComponent
        {
            NamedValue<n::nodes, std::tr1::shared_ptr<Set<Resolvent> > > nodes;
            NamedValue<n::requirements, std::tr1::shared_ptr<Set<Resolvent> > > requirements;
        };
    }

#ifdef PALUDIS_HAVE_EXTERN_TEMPLATE
    extern template class Set<resolver::Resolvent>;
    extern template class WrappedForwardIterator<Set<resolver::Resolvent>::ConstIteratorTag, const resolver::Resolvent>;
    extern template class WrappedOutputIterator<Set<resolver::Resolvent>::InserterTag, resolver::Resolvent>;

    extern template class Sequence<resolver::StronglyConnectedComponent>;
    extern template class WrappedForwardIterator<Sequence<resolver::StronglyConnectedComponent>::ConstIteratorTag, const resolver::StronglyConnectedComponent>;
#endif
}

#endif
