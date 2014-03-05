/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2010, 2011 Ciaran McCreesh
 * Copyright (c) 2014 Dimitry Ishenko
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

#ifndef PALUDIS_GUARD_PALUDIS_SELECTION_WITH_PROMOTION_HH
#define PALUDIS_GUARD_PALUDIS_SELECTION_WITH_PROMOTION_HH 1

#include <paludis/resolver/selection_with_promotion-fwd.hh>
#include <paludis/selection.hh>
#include <paludis/filtered_generator-fwd.hh>

namespace paludis
{
    namespace selection
    {
        /**
         * Return all versions with the specified properties, sorted from worst
         * to best, promoting binary packages over identical non-binary ones.
         *
         * \ingroup g_selections
         */
        class PALUDIS_VISIBLE AllVersionsSortedWithPromotion :
            public Selection
        {
            public:
                ///\name Basic operations
                ///\{

                AllVersionsSortedWithPromotion(const FilteredGenerator &);

                ///\}
        };
    }
}

#endif
