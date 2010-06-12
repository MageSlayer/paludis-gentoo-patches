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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_SLOT_NAME_OR_NULL_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_SLOT_NAME_OR_NULL_HH 1

#include <paludis/resolver/slot_name_or_null-fwd.hh>
#include <paludis/util/named_value.hh>
#include <paludis/serialise-fwd.hh>
#include <paludis/name.hh>

namespace paludis
{
    namespace n
    {
        typedef Name<struct name_or_null_name> name_or_null;
        typedef Name<struct null_means_unknown_name> null_means_unknown;
    }

    namespace resolver
    {
        struct SlotNameOrNull
        {
            NamedValue<n::name_or_null, std::tr1::shared_ptr<const SlotName> > name_or_null;
            NamedValue<n::null_means_unknown, bool> null_means_unknown;

            void serialise(Serialiser &) const;
            static const SlotNameOrNull deserialise(Deserialisation &) PALUDIS_ATTRIBUTE((warn_unused_result));

            std::size_t hash() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };
    }
}

#endif
