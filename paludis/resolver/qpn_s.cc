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

#include <paludis/resolver/qpn_s.hh>
#include <paludis/resolver/serialise-impl.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/filter.hh>
#include <paludis/dep_spec.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <sstream>

using namespace paludis;
using namespace paludis::resolver;

namespace paludis
{
    template <>
    struct Implementation<QPN_S>
    {
        QualifiedPackageName package;
        std::tr1::shared_ptr<const SlotName> slot_name_or_null;

        Implementation(const QualifiedPackageName & q, const std::tr1::shared_ptr<const SlotName> & s) :
            package(q),
            slot_name_or_null(s)
        {
        }
    };
}

QPN_S::QPN_S(const QualifiedPackageName & q, const std::tr1::shared_ptr<const SlotName> & s) :
    PrivateImplementationPattern<QPN_S>(new Implementation<QPN_S>(q, s))
{
}

QPN_S::QPN_S(const PackageDepSpec & p, const std::tr1::shared_ptr<const SlotName> & s) :
    PrivateImplementationPattern<QPN_S>(new Implementation<QPN_S>(*p.package_ptr(), s))
{
}

QPN_S::QPN_S(const std::tr1::shared_ptr<const PackageID> & id) :
    PrivateImplementationPattern<QPN_S>(new Implementation<QPN_S>(id->name(),
                id->slot_key() ? make_shared_ptr(new SlotName(id->slot_key()->value())) : make_null_shared_ptr()))
{
}

QPN_S::QPN_S(const QPN_S & other) :
    PrivateImplementationPattern<QPN_S>(new Implementation<QPN_S>(other.package(), other.slot_name_or_null()))
{
}

QPN_S::~QPN_S()
{
}

const QualifiedPackageName
QPN_S::package() const
{
    return _imp->package;
}

const std::tr1::shared_ptr<const SlotName>
QPN_S::slot_name_or_null() const
{
    return _imp->slot_name_or_null;
}

bool
QPN_S::operator< (const QPN_S & other) const
{
    if (package() < other.package())
        return true;
    if (package() > other.package())
        return false;

    /* no slot orders before any slot */
    if (slot_name_or_null())
    {
        if (other.slot_name_or_null())
            return *slot_name_or_null() < *other.slot_name_or_null();
        else
            return false;
    }
    else
    {
        if (other.slot_name_or_null())
            return true;
        else
            return false;
    }
}

bool
QPN_S::operator== (const QPN_S & other) const
{
    if (! (package() == other.package()))
        return false;

    if (slot_name_or_null())
        return other.slot_name_or_null() && *slot_name_or_null() == *other.slot_name_or_null();
    else
        return ! other.slot_name_or_null();
}

std::ostream &
paludis::resolver::operator<< (std::ostream & s, const QPN_S & q)
{
    std::stringstream ss;
    ss << q.package();
    if (q.slot_name_or_null())
        ss << ":" << *q.slot_name_or_null();
    else
        ss << " (no slot)";

    s << ss.str();
    return s;
}

Filter
QPN_S::make_slot_filter() const
{
    if (slot_name_or_null())
        return filter::Slot(*slot_name_or_null());
    else
        return filter::NoSlot();
}

QPN_S &
QPN_S::operator= (const QPN_S & other)
{
    if (this != &other)
    {
        _imp->package = other._imp->package;
        _imp->slot_name_or_null = other._imp->slot_name_or_null;
    }
    return *this;
}

void
QPN_S::serialise(Serialiser & s) const
{
    s.object("QPN_S")
        .member(SerialiserFlags<>(), "package", stringify(package()))
        .member(SerialiserFlags<>(), "slot", slot_name_or_null() ? stringify(*slot_name_or_null()) : "")
        ;
}

QPN_S
QPN_S::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "QPN_S");
    std::string slot_str(v.member<std::string>("slot"));
    return QPN_S(
            QualifiedPackageName(v.member<std::string>("package")),
            ! slot_str.empty() ? make_shared_ptr(new SlotName(slot_str)) : make_null_shared_ptr());
}

template class PrivateImplementationPattern<QPN_S>;
template class Sequence<QPN_S>;
template class WrappedForwardIterator<QPN_S_Sequence::ConstIteratorTag, const QPN_S>;

