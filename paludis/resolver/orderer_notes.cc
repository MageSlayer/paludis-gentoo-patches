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

#include <paludis/resolver/orderer_notes.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/serialise-impl.hh>

using namespace paludis;
using namespace paludis::resolver;

void
OrdererNotes::serialise(Serialiser & s) const
{
    s.object("OrdererNotes")
        .member(SerialiserFlags<>(), "cycle_breaking", cycle_breaking())
        ;
}

const std::shared_ptr<OrdererNotes>
OrdererNotes::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "OrdererNotes");

    return std::make_shared<OrdererNotes>(make_named_values<OrdererNotes>(
                    n::cycle_breaking() = v.member<std::string>("cycle_breaking")
                    ));
}

