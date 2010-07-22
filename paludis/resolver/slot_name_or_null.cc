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

#include <paludis/resolver/slot_name_or_null.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/serialise-impl.hh>
#include <ostream>

using namespace paludis;
using namespace paludis::resolver;

bool
paludis::resolver::operator== (const SlotNameOrNull & a, const SlotNameOrNull & b)
{
    if ((!! a.name_or_null()) != (!! b.name_or_null()))
        return false;

    if ((a.name_or_null()) && (*a.name_or_null() != *b.name_or_null()))
        return false;

    if ((! a.name_or_null()) && (a.null_means_unknown() != b.null_means_unknown()))
        return false;

    return true;
}

std::ostream &
paludis::resolver::operator<< (std::ostream & s, const SlotNameOrNull & n)
{
    if (n.name_or_null())
        s << ":" << *n.name_or_null();
    else if (n.null_means_unknown())
        s << ":(unknown)";
    else
        s << ":(no slot)";
    return s;
}

void
SlotNameOrNull::serialise(Serialiser & s) const
{
    s.object("SlotNameOrNull")
        .member(SerialiserFlags<>(), "name_or_null", name_or_null() ? stringify(*name_or_null()) : "")
        .member(SerialiserFlags<>(), "null_means_unknown", null_means_unknown())
        ;
}

const SlotNameOrNull
SlotNameOrNull::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "SlotNameOrNull");

    std::string s(v.member<std::string>("name_or_null"));

    return make_named_values<SlotNameOrNull>(
            n::name_or_null() = s.empty() ? make_null_shared_ptr() : std::make_shared<SlotName>(s),
            n::null_means_unknown() = v.member<bool>("null_means_unknown")
            );
}

std::size_t
SlotNameOrNull::hash() const
{
    if (name_or_null())
        return Hash<SlotName>()(*name_or_null());
    else
        return 0xdeadbeef;
}

