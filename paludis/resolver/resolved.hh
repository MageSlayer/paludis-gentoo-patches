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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_RESOLVED_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_RESOLVED_HH 1

#include <paludis/resolver/resolved-fwd.hh>
#include <paludis/resolver/decisions-fwd.hh>
#include <paludis/resolver/resolutions_by_resolvent-fwd.hh>
#include <paludis/resolver/decision-fwd.hh>
#include <paludis/resolver/job_lists-fwd.hh>
#include <paludis/resolver/nag-fwd.hh>
#include <paludis/util/named_value.hh>
#include <paludis/serialise-fwd.hh>
#include <tr1/memory>

namespace paludis
{
    namespace n
    {
        typedef Name<struct job_lists_name> job_lists;
        typedef Name<struct nag_name> nag;
        typedef Name<struct resolutions_by_resolvent_name> resolutions_by_resolvent;
        typedef Name<struct taken_change_or_remove_decisions_name> taken_change_or_remove_decisions;
        typedef Name<struct taken_unable_to_make_decisions_name> taken_unable_to_make_decisions;
        typedef Name<struct taken_unconfirmed_change_or_remove_decisions_name> taken_unconfirmed_change_or_remove_decisions;
        typedef Name<struct untaken_change_or_remove_decisions_name> untaken_change_or_remove_decisions;
        typedef Name<struct untaken_unable_to_make_decisions_name> untaken_unable_to_make_decisions;
    }

    namespace resolver
    {
        struct Resolved
        {
            NamedValue<n::job_lists, std::tr1::shared_ptr<JobLists> > job_lists;
            NamedValue<n::nag, std::tr1::shared_ptr<NAG> > nag;
            NamedValue<n::resolutions_by_resolvent, std::tr1::shared_ptr<ResolutionsByResolvent> > resolutions_by_resolvent;
            NamedValue<n::taken_change_or_remove_decisions, std::tr1::shared_ptr<OrderedChangeOrRemoveDecisions> > taken_change_or_remove_decisions;
            NamedValue<n::taken_unable_to_make_decisions, std::tr1::shared_ptr<Decisions<UnableToMakeDecision> > > taken_unable_to_make_decisions;
            NamedValue<n::taken_unconfirmed_change_or_remove_decisions, std::tr1::shared_ptr<Decisions<ChangeOrRemoveDecision> > > taken_unconfirmed_change_or_remove_decisions;
            NamedValue<n::untaken_change_or_remove_decisions, std::tr1::shared_ptr<Decisions<ChangeOrRemoveDecision> > > untaken_change_or_remove_decisions;
            NamedValue<n::untaken_unable_to_make_decisions, std::tr1::shared_ptr<Decisions<UnableToMakeDecision> > > untaken_unable_to_make_decisions;

            static const Resolved deserialise(Deserialisation &) PALUDIS_ATTRIBUTE((warn_unused_result));
            void serialise(Serialiser &) const;
        };
    }
}

#endif
