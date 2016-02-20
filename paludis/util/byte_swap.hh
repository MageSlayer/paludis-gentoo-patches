/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008 David Leverton
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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_BYTE_SWAP_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_BYTE_SWAP_HH

#include <cstddef>

namespace paludis
{
    namespace byte_swap_internals
    {
        template <std::size_t, typename> struct ByteSwap;

        template <typename T_>
        struct ByteSwap<1, T_>
        {
            static T_ swap(T_ x)
            {
                return x;
            }
        };

        template <typename T_>
        struct ByteSwap<2, T_>
        {
            static T_ swap(T_ x)
            {
                return ((x << 8) & (T_(0xFF) << 8)) | ((x >> 8) & T_(0xFF));
            }
        };

        template <typename T_>
        struct ByteSwap<4, T_>
        {
            static T_ swap(T_ x)
            {
                return ((x << 24) & (T_(0xFF) << 24)) | ((x <<  8) & (T_(0xFF) << 16))
                     | ((x >>  8) & (T_(0xFF) <<  8)) | ((x >> 24) &  T_(0xFF)       );
            }
        };

        template <typename T_>
        struct ByteSwap<8, T_>
        {
            static T_ swap(T_ x)
            {
                return ((x << 56) & (T_(0xFF) << 56)) | ((x << 40) & (T_(0xFF) << 48))
                     | ((x << 24) & (T_(0xFF) << 40)) | ((x <<  8) & (T_(0xFF) << 32))
                     | ((x >>  8) & (T_(0xFF) << 24)) | ((x >> 24) & (T_(0xFF) << 16))
                     | ((x >> 40) & (T_(0xFF) <<  8)) | ((x >> 56) &  T_(0xFF)       );
            }
        };
    }

    template <typename T_>
    inline T_
    byte_swap(T_ x)
    {
        return byte_swap_internals::ByteSwap<sizeof(T_), T_>::swap(x);
    }

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    template <typename T_>
    inline T_
    from_bigendian(T_ x)
    {
        return x;
    }

    template <typename T_>
    inline T_
    to_bigendian(T_ x)
    {
        return x;
    }
#else
    template <typename T_>
    inline T_
    from_bigendian(T_ x)
    {
        return byte_swap(x);
    }

    template <typename T_>
    inline T_
    to_bigendian(T_ x)
    {
        return byte_swap(x);
    }
#endif
}

#endif
