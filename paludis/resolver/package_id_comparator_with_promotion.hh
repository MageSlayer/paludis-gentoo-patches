/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_PACKAGE_ID_COMPARATOR_WITH_PROMOTION_HH
#define PALUDIS_GUARD_PALUDIS_PACKAGE_ID_COMPARATOR_WITH_PROMOTION_HH 1

#include <paludis/resolver/package_id_comparator_with_promotion-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/util/pimp.hh>

#include <memory>

namespace paludis
{
    namespace resolver
    {
        /**
         * A comparison functor that operates similar to the PackageIDComparator,
         * but promotes binary packages over identical non-binary ones.
         */
        class PALUDIS_VISIBLE PackageIDComparatorWithPromotion
        {
            private:
                Pimp<PackageIDComparatorWithPromotion> _imp;

            public:
                ///\name Standard library typedefs
                ///\{

                typedef bool result_type;

                ///\}

                ///\name Basic operations
                ///\{

                PackageIDComparatorWithPromotion(const Environment * const);
                PackageIDComparatorWithPromotion(const PackageIDComparatorWithPromotion &);
                ~PackageIDComparatorWithPromotion();

                ///\}

                /**
                 * Perform the less-than comparison.
                 */
                bool operator() (const std::shared_ptr<const PackageID> &,
                        const std::shared_ptr<const PackageID> &) const;
        };
    }
}

#endif
