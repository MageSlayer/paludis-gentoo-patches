/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009 Ciaran McCreesh
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

#include <paludis/resolver/resolvent.hh>
#include <paludis/resolver/serialise-impl.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/destringify.hh>
#include <paludis/dep_spec.hh>
#include <paludis/filter.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>

using namespace paludis;
using namespace paludis::resolver;

bool
paludis::resolver::operator< (const Resolvent & a, const Resolvent & b)
{
    if (a.package() < b.package())
        return true;
    if (a.package() > b.package())
        return false;

    /* no slot orders before slot */
    if (a.slot_name_or_null() && b.slot_name_or_null())
    {
        if (*a.slot_name_or_null() < *b.slot_name_or_null())
            return true;
        if (*a.slot_name_or_null() > *b.slot_name_or_null())
            return false;
    }
    else if (a.slot_name_or_null())
        return false;
    else if (b.slot_name_or_null())
        return true;

    return a.destination_type() < b.destination_type();
}

bool
paludis::resolver::operator== (const Resolvent & a, const Resolvent & b)
{
    if (a.package() != b.package())
        return false;

    if (a.slot_name_or_null() != b.slot_name_or_null())
        return false;

    if (a.slot_name_or_null() && *a.slot_name_or_null() != *b.slot_name_or_null())
        return false;

    if (a.destination_type() != b.destination_type())
        return false;

    return true;
}

#ifdef PALUDIS_HAVE_DEFAULT_DELETED

Resolvent::Resolvent(const Resolvent &) = default;

#else

Resolvent::Resolvent(const Resolvent & other) :
    destination_type(other.destination_type()),
    package(other.package()),
    slot_name_or_null(other.slot_name_or_null())
{
}

#endif

Resolvent::Resolvent(
        const PackageDepSpec & spec,
        const std::tr1::shared_ptr<const SlotName> & s,
        const DestinationType t) :
    destination_type(value_for<n::destination_type>(t)),
    package(value_for<n::package>(*spec.package_ptr())),
    slot_name_or_null(s)
{
}

Resolvent::Resolvent(
        const QualifiedPackageName & n,
        const std::tr1::shared_ptr<const SlotName> & s,
        const DestinationType t) :
    destination_type(value_for<n::destination_type>(t)),
    package(value_for<n::package>(n)),
    slot_name_or_null(s)
{
}

Resolvent::Resolvent(
        const std::tr1::shared_ptr<const PackageID> & id,
        const DestinationType t) :
    destination_type(value_for<n::destination_type>(t)),
    package(id->name()),
    slot_name_or_null(id->slot_key() ? make_shared_ptr(new SlotName(id->slot_key()->value())) : make_null_shared_ptr())
{
}

void
Resolvent::serialise(Serialiser & s) const
{
    s.object("Resolvent")
        .member(SerialiserFlags<>(), "destination_type", stringify(destination_type()))
        .member(SerialiserFlags<>(), "package", stringify(package()))
        .member(SerialiserFlags<>(), "slot", slot_name_or_null() ? stringify(*slot_name_or_null()) : "")
        ;
}

const Resolvent
Resolvent::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "Resolvent");

    std::string s(v.member<std::string>("slot"));

    return Resolvent(
            QualifiedPackageName(v.member<std::string>("package")),
            s.empty() ? make_null_shared_ptr() : make_shared_ptr(new SlotName(s)),
            destringify<DestinationType>(v.member<std::string>("destination_type"))
            );
}

Filter
paludis::resolver::make_slot_filter(const Resolvent & r)
{
    if (r.slot_name_or_null())
        return filter::Slot(*r.slot_name_or_null());
    else
        return filter::NoSlot();
}

std::ostream &
paludis::resolver::operator<< (std::ostream & s, const Resolvent & r)
{
    s << r.package();
    if (r.slot_name_or_null())
        s << ":" << *r.slot_name_or_null();
    else
        s << ":(no slot)";
    s << " -> " << r.destination_type();
    return s;
}

template class Sequence<Resolvent>;
template class WrappedForwardIterator<Resolvents::ConstIteratorTag, Resolvent>;

