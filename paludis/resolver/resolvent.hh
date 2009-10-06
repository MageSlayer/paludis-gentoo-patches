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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_RESOLVENT_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_RESOLVENT_HH 1

#include <paludis/resolver/resolvent-fwd.hh>
#include <paludis/resolver/destination_types-fwd.hh>
#include <paludis/serialise-fwd.hh>
#include <paludis/util/named_value.hh>
#include <paludis/name.hh>
#include <paludis/dep_spec-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <tr1/memory>

namespace paludis
{
    namespace n
    {
        struct destination_type;
        struct name_or_null;
        struct null_means_unknown;
        struct package;
        struct slot;
    }

    namespace resolver
    {
        struct SlotNameOrNull
        {
            NamedValue<n::name_or_null, std::tr1::shared_ptr<const SlotName> > name_or_null;
            NamedValue<n::null_means_unknown, bool> null_means_unknown;

            void serialise(Serialiser &) const;
            static const SlotNameOrNull deserialise(Deserialisation &) PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        struct Resolvent
        {
            NamedValue<n::destination_type, DestinationType> destination_type;
            NamedValue<n::package, QualifiedPackageName> package;
            NamedValue<n::slot, SlotNameOrNull> slot;

            Resolvent(const Resolvent &);

            Resolvent(const QualifiedPackageName &, const SlotName &, const DestinationType);
            Resolvent(const QualifiedPackageName &, const SlotNameOrNull &, const DestinationType);

            Resolvent(const PackageDepSpec &, const bool, const DestinationType);
            Resolvent(const PackageDepSpec &, const SlotName &, const DestinationType);

            Resolvent(const std::tr1::shared_ptr<const PackageID> &, const DestinationType);

            void serialise(Serialiser &) const;
            static const Resolvent deserialise(Deserialisation &) PALUDIS_ATTRIBUTE((warn_unused_result));
        };
    }
}

#endif
