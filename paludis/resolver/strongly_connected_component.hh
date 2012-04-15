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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_STRONGLY_CONNECTED_COMPONENT_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_STRONGLY_CONNECTED_COMPONENT_HH 1

#include <paludis/resolver/strongly_connected_component-fwd.hh>
#include <paludis/resolver/resolvent-fwd.hh>
#include <paludis/resolver/nag-fwd.hh>
#include <paludis/util/named_value.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <memory>

namespace paludis
{
    namespace n
    {
        typedef Name<struct name_nodes> nodes;
        typedef Name<struct name_requirements> requirements;
    }

    namespace resolver
    {
        struct StronglyConnectedComponent
        {
            NamedValue<n::nodes, std::shared_ptr<Set<NAGIndex> > > nodes;
            NamedValue<n::requirements, std::shared_ptr<Set<NAGIndex> > > requirements;
        };
    }

    extern template class PALUDIS_VISIBLE Set<resolver::NAGIndex>;
    extern template class PALUDIS_VISIBLE WrappedForwardIterator<Set<resolver::NAGIndex>::ConstIteratorTag, const resolver::NAGIndex>;
    extern template class PALUDIS_VISIBLE WrappedOutputIterator<Set<resolver::NAGIndex>::InserterTag, resolver::NAGIndex>;

    extern template class PALUDIS_VISIBLE Sequence<resolver::StronglyConnectedComponent>;
    extern template class PALUDIS_VISIBLE WrappedForwardIterator<Sequence<resolver::StronglyConnectedComponent>::ConstIteratorTag, const resolver::StronglyConnectedComponent>;
}

#endif
