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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_NAG_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_NAG_HH 1

#include <paludis/resolver/nag-fwd.hh>
#include <paludis/resolver/resolvent-fwd.hh>
#include <paludis/resolver/strongly_connected_component-fwd.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/named_value.hh>
#include <tr1/memory>

namespace paludis
{
    namespace n
    {
        typedef Name<struct build_name> build;
        typedef Name<struct run_name> run;
    }

    namespace resolver
    {
        struct NAGEdgeProperties
        {
            NamedValue<n::build, bool> build;
            NamedValue<n::run, bool> run;

            NAGEdgeProperties & operator|= (const NAGEdgeProperties &);
        };

        class NAG :
            private PrivateImplementationPattern<NAG>
        {
            public:
                NAG();
                ~NAG();

                void add_node(const Resolvent &);
                void add_edge(const Resolvent &, const Resolvent &, const NAGEdgeProperties &);

                void verify_edges() const;

                const std::tr1::shared_ptr<const SortedStronglyConnectedComponents>
                    sorted_strongly_connected_components() const PALUDIS_ATTRIBUTE((warn_unused_result));

                struct EdgesFromConstIteratorTag;
                typedef WrappedForwardIterator<EdgesFromConstIteratorTag, const std::pair<const Resolvent, NAGEdgeProperties> > EdgesFromConstIterator;
                EdgesFromConstIterator begin_edges_from(const Resolvent &) const PALUDIS_ATTRIBUTE((warn_unused_result));
                EdgesFromConstIterator end_edges_from(const Resolvent &) const PALUDIS_ATTRIBUTE((warn_unused_result));
        };
    }

    extern template class WrappedForwardIterator<resolver::NAG::EdgesFromConstIteratorTag,
           const std::pair<const resolver::Resolvent, resolver::NAGEdgeProperties> >;
}

#endif
