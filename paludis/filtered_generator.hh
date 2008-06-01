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

#ifndef PALUDIS_GUARD_PALUDIS_FILTERED_GENERATOR_HH
#define PALUDIS_GUARD_PALUDIS_FILTERED_GENERATOR_HH 1

#include <paludis/filtered_generator-fwd.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/filter-fwd.hh>
#include <paludis/generator-fwd.hh>

namespace paludis
{
    class PALUDIS_VISIBLE FilteredGenerator :
        private PrivateImplementationPattern<FilteredGenerator>
    {
        public:
            FilteredGenerator(const FilteredGenerator &);
            FilteredGenerator(const Generator &, const Filter &);
            FilteredGenerator(const FilteredGenerator &, const Filter &);
            FilteredGenerator & operator= (const FilteredGenerator &);
            ~FilteredGenerator();

            const Generator & generator() const PALUDIS_ATTRIBUTE((warn_unused_result));
            const Filter & filter() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

#ifdef PALUDIS_HAVE_EXTERN_TEMPLATE
    extern template class PrivateImplementationPattern<FilteredGenerator>;
#endif
}

#endif
