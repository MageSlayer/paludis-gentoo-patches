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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_RANDOM_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_RANDOM_HH 1

#include <cstdlib>
#include <inttypes.h>

namespace paludis
{
    /**
     * A basic random number generator class, which is not suitable for
     * cryptography but is fast and reasonably pseudorandom.
     *
     * See \ref TCppPL 22.7 for justification. See \ref TaoCP2 3.2.1 for the
     * basic algorithm and \ref AppCrypt 16.1 for the choice of numbers.
     *
     * \ingroup grprandom
     */
    class Random
    {
        private:
            static uint32_t global_seed;
            uint32_t local_seed;

            static const uint32_t _a = 2416;
            static const uint32_t _b = 374441;
            static const uint32_t _m = 1771875;

        public:
            ///\name Basic operations
            ///\{

            /// Constructor, with a seed.
            Random(uint32_t seed);

            /// Constructor, with a magic random seed.
            Random();

            ///\}

            /// Fetch a random number in (0, max]
            template <typename DiffType_>
            DiffType_ operator() (DiffType_ max)
            {
                local_seed = (_a * local_seed + _b) % _m;
                double t(static_cast<double>(local_seed) / static_cast<double>(_m));
                return static_cast<DiffType_>(t * max);
            }
    };
}

#endif
