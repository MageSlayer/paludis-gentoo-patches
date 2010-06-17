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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_ORDERER_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_ORDERER_HH 1

#include <paludis/resolver/orderer-fwd.hh>
#include <paludis/resolver/orderer_notes-fwd.hh>
#include <paludis/resolver/resolved-fwd.hh>
#include <paludis/resolver/decision-fwd.hh>
#include <paludis/resolver/strongly_connected_component-fwd.hh>
#include <paludis/resolver/nag-fwd.hh>
#include <paludis/resolver/resolvent-fwd.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/environment-fwd.hh>

namespace paludis
{
    namespace resolver
    {
        class PALUDIS_VISIBLE Orderer :
            private PrivateImplementationPattern<Orderer>
        {
            private:
                void _schedule(
                        const Resolvent &,
                        const std::tr1::shared_ptr<const ChangeOrRemoveDecision> &,
                        const std::tr1::shared_ptr<const OrdererNotes> &);

                void _order_sub_ssccs(
                        const NAG &,
                        const StronglyConnectedComponent & top_scc,
                        const std::tr1::shared_ptr<const SortedStronglyConnectedComponents> & sub_ssccs,
                        const bool can_recurse);

            public:
                Orderer(
                        const Environment * const,
                        const std::tr1::shared_ptr<Resolved> &);
                ~Orderer();

                void resolve();
        };
    }
}

#endif
