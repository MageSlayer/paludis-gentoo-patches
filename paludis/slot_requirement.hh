/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_SLOT_REQUIREMENT_HH
#define PALUDIS_GUARD_PALUDIS_SLOT_REQUIREMENT_HH 1

#include <paludis/slot_requirement-fwd.hh>
#include <paludis/name-fwd.hh>
#include <paludis/util/visitor.hh>
#include <paludis/util/type_list.hh>

namespace paludis
{
    class PALUDIS_VISIBLE SlotRequirement :
        public virtual DeclareAbstractAcceptMethods<SlotRequirement, MakeTypeList<
            SlotExactRequirement, SlotAnyLockedRequirement, SlotAnyUnlockedRequirement>::Type>
    {
        public:
            virtual const std::string as_string() const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    class PALUDIS_VISIBLE SlotExactRequirement :
        public SlotRequirement,
        public ImplementAcceptMethods<SlotRequirement, SlotExactRequirement>
    {
        public:
            virtual const SlotName slot() const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    class PALUDIS_VISIBLE SlotAnyLockedRequirement :
        public SlotRequirement,
        public ImplementAcceptMethods<SlotRequirement, SlotAnyLockedRequirement>
    {
    };

    class PALUDIS_VISIBLE SlotAnyUnlockedRequirement :
        public SlotRequirement,
        public ImplementAcceptMethods<SlotRequirement, SlotAnyUnlockedRequirement>
    {
    };
}

#endif
