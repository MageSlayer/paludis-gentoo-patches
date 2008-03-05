/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_PACKAGE_DEP_SPEC_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_PACKAGE_DEP_SPEC_HH 1

#include <paludis/dep_spec-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/repositories/e/eapi-fwd.hh>
#include <paludis/slot_requirement.hh>
#include <paludis/name.hh>

namespace paludis
{
    namespace erepository
    {
        PackageDepSpec parse_e_package_dep_spec(const std::string &, const EAPI & eapi,
                const tr1::shared_ptr<const PackageID> & id) PALUDIS_VISIBLE;

        PartiallyMadePackageDepSpec partial_parse_e_package_dep_spec(const std::string &, const EAPI & eapi,
                const tr1::shared_ptr<const PackageID> & id) PALUDIS_VISIBLE;

        class ESlotExactRequirement :
            public SlotExactRequirement
        {
            private:
                const SlotName _s;
                const bool _e;

            public:
                ESlotExactRequirement(const SlotName &, const bool equals);

                virtual const std::string as_string() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual const SlotName slot() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class ESlotAnyUnlockedRequirement :
            public SlotAnyUnlockedRequirement
        {
            public:
                virtual const std::string as_string() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class ESlotAnyLockedRequirement :
            public SlotAnyLockedRequirement
        {
            public:
                virtual const std::string as_string() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };
    }
}

#endif
