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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_DECISIONS_FWD_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_DECISIONS_FWD_HH 1

#include <paludis/util/no_type.hh>
#include <paludis/resolver/orderer_notes-fwd.hh>
#include <paludis/resolver/decision-fwd.hh>
#include <memory>

namespace paludis
{
    namespace resolver
    {
        template <typename Decision_, typename Notes_ = NoType<0u> *>
        struct Decisions;

        typedef Decisions<ChangeOrRemoveDecision, std::shared_ptr<const OrdererNotes> > OrderedChangeOrRemoveDecisions;
    }
}

#endif
