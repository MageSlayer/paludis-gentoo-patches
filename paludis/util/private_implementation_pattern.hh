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

#ifndef PALUDIS_GUARD_PALUDIS_PRIVATE_IMPLEMENTATION_PATTERN_HH
#define PALUDIS_GUARD_PALUDIS_PRIVATE_IMPLEMENTATION_PATTERN_HH 1

#include <paludis/util/counted_ptr.hh>
#include <paludis/util/instantiation_policy.hh>

namespace paludis
{
    /**
     * Private implementation data, to be specialised for any class that
     * uses PrivateImplementationPattern.
     */
    template <typename C_>
    struct Implementation;

    /**
     * A class descended from PrivateImplementationPattern has an associated
     * Implementation instance.
     */
    template <typename C_>
    class PrivateImplementationPattern :
        private InstantiationPolicy<PrivateImplementationPattern<C_>, instantiation_method::NonCopyableTag>
    {
        protected:
            /**
             * Pointer to our implementation data.
             */
            CountedPtr<Implementation<C_>, count_policy::InternalCountTag> _implementation;

        public:
            /**
             * Constructor.
             */
            explicit PrivateImplementationPattern(Implementation<C_> * i) :
                _implementation(i)
            {
            }
    };
}

#endif
