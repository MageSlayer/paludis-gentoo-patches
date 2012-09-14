/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2012 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_SLOT_HH
#define PALUDIS_GUARD_PALUDIS_SLOT_HH 1

#include <paludis/slot-fwd.hh>

#include <paludis/util/named_value.hh>
#include <paludis/name.hh>
#include <utility>

namespace paludis
{
    namespace n
    {
        typedef Name<struct name_match_values> match_values;
        typedef Name<struct name_parallel_value> parallel_value;
        typedef Name<struct name_raw_value> raw_value;
    }

    /**
     * Represents an ID's slot, possibly with sub-slot related information.
     *
     * \since 0.79
     */
    struct Slot
    {
        /**
         * First value is the slot, second the sub-slot. Both should be the same
         * when sub-slots are not being used. Matching is against the first
         * value for a :slot, and against both for a :first/second.
         */
        NamedValue<n::match_values, std::pair<SlotName, SlotName> > match_values;

        /**
         * Used to determine what can be installed in parallel.
         */
        NamedValue<n::parallel_value, SlotName> parallel_value;

        /**
         * Used for output purposes, contains whatever the native representation
         * is.
         */
        NamedValue<n::raw_value, std::string> raw_value;
    };
}

#endif
