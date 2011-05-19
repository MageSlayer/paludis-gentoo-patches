/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_ELIKE_SLOT_REQUIREMENT_HH
#define PALUDIS_GUARD_PALUDIS_ELIKE_SLOT_REQUIREMENT_HH 1

#include <paludis/elike_slot_requirement-fwd.hh>
#include <paludis/slot_requirement.hh>
#include <paludis/name.hh>

namespace paludis
{
    class PALUDIS_VISIBLE ELikeSlotExactRequirement :
        public SlotExactRequirement
    {
        private:
            const SlotName _s;
            const bool _e;

        public:
            ELikeSlotExactRequirement(const SlotName &, const bool equals);

            virtual const std::string as_string() const PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual const SlotName slot() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE ELikeSlotAnyUnlockedRequirement :
        public SlotAnyUnlockedRequirement
    {
        public:
            virtual const std::string as_string() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE ELikeSlotAnyLockedRequirement :
        public SlotAnyLockedRequirement
    {
        public:
            virtual const std::string as_string() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };
}

#endif
