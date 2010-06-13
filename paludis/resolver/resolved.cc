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
#include <paludis/resolver/resolutions.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/serialise-impl.hh>

using namespace paludis;
using namespace paludis::resolver;

void
Resolved::serialise(Serialiser & s) const
{
    s.object("Resolved")
        .member(SerialiserFlags<serialise::container, serialise::might_be_null>(), "display_change_or_remove_decisions", display_change_or_remove_decisions())
        .member(SerialiserFlags<serialise::might_be_null>(), "resolutions", resolutions())
        .member(SerialiserFlags<serialise::container, serialise::might_be_null>(), "taken_unable_to_make_decisions", taken_unable_to_make_decisions())
        .member(SerialiserFlags<serialise::container, serialise::might_be_null>(), "untaken_change_or_remove_decisions", untaken_change_or_remove_decisions())
        .member(SerialiserFlags<serialise::container, serialise::might_be_null>(), "untaken_unable_to_make_decisions", untaken_unable_to_make_decisions())
        ;
}

const Resolved
Resolved::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "Resolved");

    std::tr1::shared_ptr<Decisions<ChangeOrRemoveDecision> > display_change_or_remove_decisions(new Decisions<ChangeOrRemoveDecision>);
    {
        Deserialisator vv(*v.find_remove_member("display_change_or_remove_decisions"), "c");
        for (int n(1), n_end(vv.member<int>("count") + 1) ; n != n_end ; ++n)
            display_change_or_remove_decisions->push_back(vv.member<std::tr1::shared_ptr<ChangeOrRemoveDecision> >(stringify(n)));
    }

    std::tr1::shared_ptr<Decisions<UnableToMakeDecision> > taken_unable_to_make_decisions(new Decisions<UnableToMakeDecision>);
    {
        Deserialisator vv(*v.find_remove_member("taken_unable_to_make_decisions"), "c");
        for (int n(1), n_end(vv.member<int>("count") + 1) ; n != n_end ; ++n)
            taken_unable_to_make_decisions->push_back(vv.member<std::tr1::shared_ptr<UnableToMakeDecision> >(stringify(n)));
    }

    std::tr1::shared_ptr<Decisions<ChangeOrRemoveDecision> > untaken_change_or_remove_decisions(new Decisions<ChangeOrRemoveDecision>);
    {
        Deserialisator vv(*v.find_remove_member("untaken_change_or_remove_decisions"), "c");
        for (int n(1), n_end(vv.member<int>("count") + 1) ; n != n_end ; ++n)
            untaken_change_or_remove_decisions->push_back(vv.member<std::tr1::shared_ptr<ChangeOrRemoveDecision> >(stringify(n)));
    }

    std::tr1::shared_ptr<Decisions<UnableToMakeDecision> > untaken_unable_to_make_decisions(new Decisions<UnableToMakeDecision>);
    {
        Deserialisator vv(*v.find_remove_member("untaken_unable_to_make_decisions"), "c");
        for (int n(1), n_end(vv.member<int>("count") + 1) ; n != n_end ; ++n)
            untaken_unable_to_make_decisions->push_back(vv.member<std::tr1::shared_ptr<UnableToMakeDecision> >(stringify(n)));
    }

    return make_named_values<Resolved>(
            n::display_change_or_remove_decisions() = display_change_or_remove_decisions,
            n::resolutions() = v.member<std::tr1::shared_ptr<Resolutions> >("resolutions"),
            n::taken_unable_to_make_decisions() = taken_unable_to_make_decisions,
            n::untaken_change_or_remove_decisions() = untaken_change_or_remove_decisions,
            n::untaken_unable_to_make_decisions() = untaken_unable_to_make_decisions
            );
}

