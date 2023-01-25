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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_RESOLVENT_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_RESOLVENT_HH 1

#include <paludis/resolver/resolvent-fwd.hh>
#include <paludis/resolver/destination_types-fwd.hh>
#include <paludis/resolver/slot_name_or_null.hh>
#include <paludis/util/named_value.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/serialise-fwd.hh>
#include <paludis/dep_spec-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/name.hh>
#include <memory>

namespace paludis
{
    namespace n
    {
        typedef Name<struct name_destination_type> destination_type;
        typedef Name<struct name_package> package;
        typedef Name<struct name_slot> slot;
    }

    namespace resolver
    {
        struct Resolvent
        {
            NamedValue<n::destination_type, DestinationType> destination_type;
            NamedValue<n::package, QualifiedPackageName> package;
            NamedValue<n::slot, SlotNameOrNull> slot;

            Resolvent(const Resolvent &) = default;
            Resolvent & operator=(const Resolvent &) = default;

            Resolvent(const QualifiedPackageName &, const SlotName &, const DestinationType);
            Resolvent(const QualifiedPackageName &, const SlotNameOrNull &, const DestinationType);

            Resolvent(const PackageDepSpec &, const SlotName &, const DestinationType);
            Resolvent(const PackageDepSpec &, const SlotNameOrNull &, const DestinationType);

            Resolvent(const std::shared_ptr<const PackageID> &, const DestinationType);

            void serialise(Serialiser &) const;
            static const Resolvent deserialise(Deserialisation &) PALUDIS_ATTRIBUTE((warn_unused_result));

            std::size_t hash() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };
    }

    extern template class PALUDIS_VISIBLE Sequence<resolver::Resolvent>;
    extern template class PALUDIS_VISIBLE WrappedForwardIterator<Sequence<resolver::Resolvent>::ConstIteratorTag, const resolver::Resolvent>;
}

#endif
