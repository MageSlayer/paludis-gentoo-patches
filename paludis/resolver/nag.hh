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
#include <paludis/resolver/resolvent.hh>
#include <paludis/resolver/strongly_connected_component-fwd.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/named_value.hh>
#include <paludis/serialise-fwd.hh>
#include <memory>
#include <functional>

namespace paludis
{
    namespace n
    {
        typedef Name<struct always_name> always;
        typedef Name<struct build_name> build;
        typedef Name<struct build_all_met_name> build_all_met;
        typedef Name<struct resolvent_name> resolvent;
        typedef Name<struct role_name> role;
        typedef Name<struct run_name> run;
        typedef Name<struct run_all_met_name> run_all_met;
    }

    namespace resolver
    {
        struct NAGIndex
        {
            NamedValue<n::resolvent, Resolvent> resolvent;
            NamedValue<n::role, NAGIndexRole> role;

            std::size_t hash() const PALUDIS_ATTRIBUTE((warn_unused_result));

            void serialise(Serialiser &) const;
            static const NAGIndex deserialise(Deserialisation & d) PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        struct NAGEdgeProperties
        {
            NamedValue<n::always, bool> always;
            NamedValue<n::build, bool> build;
            NamedValue<n::build_all_met, bool> build_all_met;
            NamedValue<n::run, bool> run;
            NamedValue<n::run_all_met, bool> run_all_met;

            NAGEdgeProperties & operator|= (const NAGEdgeProperties &);

            void serialise(Serialiser &) const;
            static const NAGEdgeProperties deserialise(Deserialisation & d) PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class NAG :
            private Pimp<NAG>
        {
            public:
                NAG();
                ~NAG();

                void add_node(const NAGIndex &);
                void add_edge(const NAGIndex &, const NAGIndex &, const NAGEdgeProperties &);

                void verify_edges() const;

                const std::shared_ptr<const SortedStronglyConnectedComponents> sorted_strongly_connected_components(
                        const std::function<Tribool (const NAGIndex &)> & order_early_fn
                        ) const PALUDIS_ATTRIBUTE((warn_unused_result));

                struct EdgesFromConstIteratorTag;
                typedef WrappedForwardIterator<EdgesFromConstIteratorTag, const std::pair<const NAGIndex, NAGEdgeProperties> > EdgesFromConstIterator;
                EdgesFromConstIterator begin_edges_from(const NAGIndex &) const PALUDIS_ATTRIBUTE((warn_unused_result));
                EdgesFromConstIterator end_edges_from(const NAGIndex &) const PALUDIS_ATTRIBUTE((warn_unused_result));

                struct NodesConstIteratorTag;
                typedef WrappedForwardIterator<NodesConstIteratorTag, const NAGIndex> NodesConstIterator;
                NodesConstIterator begin_nodes() const PALUDIS_ATTRIBUTE((warn_unused_result));
                NodesConstIterator end_nodes() const PALUDIS_ATTRIBUTE((warn_unused_result));

                void serialise(Serialiser &) const;
                static const std::shared_ptr<NAG> deserialise(Deserialisation & d) PALUDIS_ATTRIBUTE((warn_unused_result));
        };
    }

    extern template class WrappedForwardIterator<resolver::NAG::EdgesFromConstIteratorTag,
           const std::pair<const resolver::NAGIndex, resolver::NAGEdgeProperties> >;
    extern template class WrappedForwardIterator<resolver::NAG::NodesConstIteratorTag, const resolver::NAGIndex>;
}

#endif
