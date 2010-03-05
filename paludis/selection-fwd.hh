/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_SELECTION_FWD_HH
#define PALUDIS_GUARD_PALUDIS_SELECTION_FWD_HH 1

#include <iosfwd>
#include <paludis/util/attributes.hh>

/** \file
 * Forward declarations for paludis/selection.hh .
 *
 * \ingroup g_selections
 */

namespace paludis
{
    class Selection;

    namespace selection
    {
        class SomeArbitraryVersion;
        class BestVersionOnly;
        class BestVersionInEachSlot;
        class AllVersionsSorted;
        class AllVersionsGroupedBySlot;
        class AllVersionsUnsorted;
        class RequireExactlyOne;
    }

    class DidNotGetExactlyOneError;

    /**
     * A Selection can be represented as a string, can be written to a stream.
     *
     * \ingroup g_selections
     */
    std::ostream & operator<< (std::ostream &, const Selection &) PALUDIS_VISIBLE;
}

#endif
