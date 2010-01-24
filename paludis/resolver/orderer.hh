/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010 Ciaran McCreesh
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
#include <paludis/resolver/resolvent-fwd.hh>
#include <paludis/resolver/resolution-fwd.hh>
#include <paludis/resolver/resolutions-fwd.hh>
#include <paludis/resolver/constraint-fwd.hh>
#include <paludis/resolver/reason-fwd.hh>
#include <paludis/resolver/resolver_functions-fwd.hh>
#include <paludis/resolver/sanitised_dependencies-fwd.hh>
#include <paludis/resolver/decider-fwd.hh>
#include <paludis/resolver/resolver-fwd.hh>
#include <paludis/resolver/job_id-fwd.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/environment.hh>
#include <tr1/memory>

namespace paludis
{
    namespace resolver
    {
        class PALUDIS_VISIBLE Orderer :
            private PrivateImplementationPattern<Orderer>
        {
            private:
                void _resolve_jobs();
                void _resolve_jobs_dep_arrows();
                void _resolve_order();

                bool _can_order(const JobID &, const bool desperate) const PALUDIS_ATTRIBUTE((warn_unused_result));
                void _mark_already_ordered(const JobID &);
                void _order_this(const JobID &, const bool inside_something_else);

                bool _order_some(const bool desperate, const bool installs_only) PALUDIS_ATTRIBUTE((warn_unused_result));
                bool _remove_usable_cycles() PALUDIS_ATTRIBUTE((warn_unused_result));
                void _cycle_error() PALUDIS_ATTRIBUTE((noreturn));

            public:
                Orderer(
                        const Environment * const,
                        const std::tr1::shared_ptr<const Decider> &,
                        const std::tr1::shared_ptr<ResolverLists> &);
                ~Orderer();

                void resolve();
        };
    }
}

#endif
