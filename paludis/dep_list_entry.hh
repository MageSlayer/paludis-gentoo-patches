/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_DEP_LIST_ENTRY_HH
#define PALUDIS_GUARD_PALUDIS_DEP_LIST_ENTRY_HH 1

#include <paludis/smart_record.hh>
#include <paludis/qualified_package_name.hh>
#include <paludis/version_spec.hh>
#include <paludis/slot_name.hh>
#include <paludis/repository_name.hh>
#include <ostream>

namespace paludis
{
    /**
     * Keys for a DepListEntry.
     */
    enum DepListEntryKeys
    {
        dle_name,          ///< Package name
        dle_version,       ///< Package version
        dle_slot,          ///< Package SLOT
        dle_repository     ///< Repository name
    };

    /**
     * Tag for a DepListEntry.
     */
    struct DepListEntryTag :
        SmartRecordTag<comparison_mode::FullComparisonTag, comparison_method::SmartRecordCompareByAllTag>,
        SmartRecordKeys<DepListEntryKeys, 4>,
        SmartRecordKey<dle_name, QualifiedPackageName>,
        SmartRecordKey<dle_version, VersionSpec>,
        SmartRecordKey<dle_slot, SlotName>,
        SmartRecordKey<dle_repository, RepositoryName>
    {
    };

    /**
     * A DepListEntry represents an entry in a DepList.
     */
    typedef MakeSmartRecord<DepListEntryTag>::Type DepListEntry;

    /**
     * A DepListEntry can be written to a stream.
     */
    std::ostream & operator<< (std::ostream &, const DepListEntry &);
}

#endif
