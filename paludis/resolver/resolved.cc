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

#include <paludis/resolver/resolved.hh>
#include <paludis/resolver/decisions.hh>
#include <paludis/resolver/decision.hh>
#include <paludis/resolver/resolutions_by_resolvent.hh>
#include <paludis/resolver/work_lists.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/serialise-impl.hh>

using namespace paludis;
using namespace paludis::resolver;

void
Resolved::serialise(Serialiser & s) const
{
    s.object("Resolved")
        .member(SerialiserFlags<serialise::might_be_null>(), "resolutions_by_resolvent", resolutions_by_resolvent())
        .member(SerialiserFlags<serialise::might_be_null>(), "taken_change_or_remove_decisions", taken_change_or_remove_decisions())
        .member(SerialiserFlags<serialise::might_be_null>(), "taken_unable_to_make_decisions", taken_unable_to_make_decisions())
        .member(SerialiserFlags<serialise::might_be_null>(), "taken_unconfirmed_change_or_remove_decisions", taken_unconfirmed_change_or_remove_decisions())
        .member(SerialiserFlags<serialise::might_be_null>(), "untaken_change_or_remove_decisions", untaken_change_or_remove_decisions())
        .member(SerialiserFlags<serialise::might_be_null>(), "untaken_unable_to_make_decisions", untaken_unable_to_make_decisions())
        .member(SerialiserFlags<serialise::might_be_null>(), "work_lists", work_lists())
        ;
}

const Resolved
Resolved::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "Resolved");

    return make_named_values<Resolved>(
            n::resolutions_by_resolvent() =
                v.member<std::tr1::shared_ptr<ResolutionsByResolvent> >("resolutions_by_resolvent"),
            n::taken_change_or_remove_decisions() =
                v.member<std::tr1::shared_ptr<ChangeOrRemoveDecisionsWithNotes> >("taken_change_or_remove_decisions"),
            n::taken_unable_to_make_decisions() =
                v.member<std::tr1::shared_ptr<Decisions<UnableToMakeDecision> > >("taken_unable_to_make_decisions"),
            n::taken_unconfirmed_change_or_remove_decisions() =
                v.member<std::tr1::shared_ptr<Decisions<ChangeOrRemoveDecision> > >("taken_unconfirmed_change_or_remove_decisions"),
            n::untaken_change_or_remove_decisions() =
                v.member<std::tr1::shared_ptr<Decisions<ChangeOrRemoveDecision> > >("untaken_change_or_remove_decisions"),
            n::untaken_unable_to_make_decisions() =
                v.member<std::tr1::shared_ptr<Decisions<UnableToMakeDecision> > >("untaken_unable_to_make_decisions"),
            n::work_lists() =
                v.member<std::tr1::shared_ptr<WorkLists> >("work_lists")
            );
}

