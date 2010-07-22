/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_SELECTION_CACHE_HH
#define PALUDIS_GUARD_PALUDIS_SELECTION_CACHE_HH 1

#include <paludis/selection_cache-fwd.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/selection-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <memory>

namespace paludis
{
    class PALUDIS_VISIBLE SelectionCache :
        private PrivateImplementationPattern<SelectionCache>
    {
        public:
            SelectionCache();
            ~SelectionCache();

            const std::shared_ptr<PackageIDSequence> perform_select(
                    const Environment * const,
                    const Selection &) const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE ScopedSelectionCache :
        private PrivateImplementationPattern<ScopedSelectionCache>
    {
        public:
            ScopedSelectionCache(Environment * const);
            ~ScopedSelectionCache();
    };

    extern template class PrivateImplementationPattern<SelectionCache>;
    extern template class PrivateImplementationPattern<ScopedSelectionCache>;
}

#endif
