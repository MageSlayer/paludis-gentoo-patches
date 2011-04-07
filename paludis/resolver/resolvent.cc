/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010, 2011 Ciaran McCreesh
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
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/dep_spec.hh>
#include <paludis/filter.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/serialise-impl.hh>
#include <paludis/package_dep_spec_requirement.hh>

using namespace paludis;
using namespace paludis::resolver;

bool
paludis::resolver::operator< (const Resolvent & a, const Resolvent & b)
{
    if (a.package() < b.package())
        return true;
    if (a.package() > b.package())
        return false;

    /* unknown slot orders before no slot orders before slot */
    if (a.slot().name_or_null() && b.slot().name_or_null())
    {
        if (*a.slot().name_or_null() < *b.slot().name_or_null())
            return true;
        if (*a.slot().name_or_null() > *b.slot().name_or_null())
            return false;
    }
    else if (a.slot().name_or_null())
        return false;
    else if (b.slot().name_or_null())
        return true;
    else if (a.slot().null_means_unknown() && ! b.slot().null_means_unknown())
        return true;
    else if (! a.slot().null_means_unknown() && b.slot().null_means_unknown())
        return false;

    return a.destination_type() < b.destination_type();
}

bool
paludis::resolver::operator== (const Resolvent & a, const Resolvent & b)
{
    if (a.package() != b.package())
        return false;

    if (! (a.slot() == b.slot()))
        return false;

    if (a.destination_type() != b.destination_type())
        return false;

    return true;
}

Resolvent::Resolvent(const Resolvent &) = default;

Resolvent::Resolvent(
        const PackageDepSpec & spec,
        const SlotName & s,
        const DestinationType t) :
    destination_type(n::destination_type() = t),
    package(n::package() = spec.package_name_requirement()->name()),
    slot(make_named_values<SlotNameOrNull>(
                n::name_or_null() = std::make_shared<SlotName>(s),
                n::null_means_unknown() = false
                ))
{
}

Resolvent::Resolvent(
        const PackageDepSpec & spec,
        const SlotNameOrNull & s,
        const DestinationType t) :
    destination_type(n::destination_type() = t),
    package(n::package() = spec.package_name_requirement()->name()),
    slot(s)
{
}

Resolvent::Resolvent(
        const QualifiedPackageName & n,
        const SlotName & s,
        const DestinationType t) :
    destination_type(n::destination_type() = t),
    package(n::package() = n),
    slot(make_named_values<SlotNameOrNull>(
                n::name_or_null() = std::make_shared<SlotName>(s),
                n::null_means_unknown() = false
                ))
{
}

Resolvent::Resolvent(
        const QualifiedPackageName & n,
        const SlotNameOrNull & s,
        const DestinationType t) :
    destination_type(n::destination_type() = t),
    package(n::package() = n),
    slot(s)
{
}

Resolvent::Resolvent(
        const std::shared_ptr<const PackageID> & id,
        const DestinationType t) :
    destination_type(n::destination_type() = t),
    package(id->name()),
    slot(make_named_values<SlotNameOrNull>(
                n::name_or_null() = id->slot_key() ?
                    std::make_shared<SlotName>(id->slot_key()->value()) :
                    make_null_shared_ptr(),
                n::null_means_unknown() = false
                ))
{
}

void
Resolvent::serialise(Serialiser & s) const
{
    s.object("Resolvent")
        .member(SerialiserFlags<>(), "destination_type", stringify(destination_type()))
        .member(SerialiserFlags<>(), "package", stringify(package()))
        .member(SerialiserFlags<>(), "slot", slot())
        ;
}

const Resolvent
Resolvent::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "Resolvent");

    return Resolvent(
            QualifiedPackageName(v.member<std::string>("package")),
            v.member<SlotNameOrNull>("slot"),
            destringify<DestinationType>(v.member<std::string>("destination_type"))
            );
}

std::size_t
Resolvent::hash() const
{
    return
        static_cast<int>(destination_type()) ^
        Hash<QualifiedPackageName>()(package()) ^
        Hash<SlotNameOrNull>()(slot())
        ;
}

Filter
paludis::resolver::make_slot_filter(const Resolvent & r)
{
    if (r.slot().name_or_null())
        return filter::Slot(*r.slot().name_or_null());
    else if (r.slot().null_means_unknown())
        return filter::All();
    else
        return filter::NoSlot();
}

std::ostream &
paludis::resolver::operator<< (std::ostream & s, const Resolvent & r)
{
    s << r.package() << r.slot() << "::(" << r.destination_type() << ")";
    return s;
}

template class Sequence<Resolvent>;
template class WrappedForwardIterator<Resolvents::ConstIteratorTag, const Resolvent>;

