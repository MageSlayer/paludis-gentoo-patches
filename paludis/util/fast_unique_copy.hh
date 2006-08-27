/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_VIRTUALS_FAST_UNIQUE_COPY_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_VIRTUALS_FAST_UNIQUE_COPY_HH 1

#include <paludis/util/iterator.hh>
#include <functional>

namespace paludis
{
    /**
     * For use by fast_unique_copy only.
     */
    namespace fast_unique_copy_internals
    {
        /**
         * For use by fast_unique_copy only.
         */
        template <typename I_, typename O_, typename C_>
        void
        real_fast_unique_copy(const I_ & start, const I_ & end, const I_ & full_end, O_ out,
                const C_ & comp, const I_ & mbgt)
        {
            if (start != end)
            {
                // if our final item is less than or equal to mbgt, there're no
                // matches in this block
                if ((mbgt != full_end) && ((comp(*previous(end), *mbgt)) || (! comp(*mbgt, *previous(end)))))
                    return;

                // if our first item is equal to our last item, we have exactly
                // one unique item in this sequence
                if ((! comp(*start, *previous(end))) && (! comp(*previous(end), *start)))
                    *out++ = *start;
                else
                {
                    I_ mid = start + (std::distance(start, end) >> 1);
                    real_fast_unique_copy(start, mid, full_end, out, comp, mbgt);
                    real_fast_unique_copy(mid, end, full_end, out, comp, previous(mid));
                }
            }
        }
    }

    /**
     * Extract unique elements from a sorted range of random access iterators.
     */
    template <typename I_, typename O_>
    void
    fast_unique_copy(const I_ & start, const I_ & end, O_ out)
    {
        fast_unique_copy_internals::real_fast_unique_copy(start, end, end, out,
                std::less<typename std::iterator_traits<I_>::value_type>(), end);
    }

    /**
     * Extract unique elements from a sorted range of random access iterators.
     */
    template <typename I_, typename O_, typename C_>
    void
    fast_unique_copy(const I_ & start, const I_ & end, O_ out, const C_ & comp)
    {
        fast_unique_copy_internals::real_fast_unique_copy(start, end, end, out, comp, end);
    }
}

#endif
