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

#include <paludis/resolver/why_changed_choices.hh>
#include <paludis/resolver/reason.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/serialise-impl.hh>
#include <paludis/changed_choices.hh>

using namespace paludis;
using namespace paludis::resolver;

void
WhyChangedChoices::serialise(Serialiser & s) const
{
    auto ss(s.object("WhyChangedChoices"));

    ss.member(SerialiserFlags<serialise::might_be_null>(), "changed_choices", changed_choices());

    int n(0);
    for (auto o(reasons()->begin()), o_end(reasons()->end()) ;
            o != o_end ; ++o)
        ss.member(SerialiserFlags<>(), stringify(++n), **o);
    ss.member(SerialiserFlags<>(), "reasons_count", stringify(n));
}

const std::shared_ptr<WhyChangedChoices>
WhyChangedChoices::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "WhyChangedChoices");
    auto result(std::make_shared<WhyChangedChoices>(make_named_values<WhyChangedChoices>(
                    n::changed_choices() = v.member<std::shared_ptr<ChangedChoices> >("changed_choices"),
                    n::reasons() = std::make_shared<Reasons>()
                    )));

    for (int n(1), n_end(v.member<int>("reasons_count") + 1) ; n != n_end ; ++n)
        result->reasons()->push_back(v.member<std::shared_ptr<Reason> >(stringify(n)));

    return result;
}

