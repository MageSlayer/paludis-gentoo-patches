/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 David Leverton
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

#include "config.h"

#include <paludis/util/sha1.hh>
#include <paludis/util/byte_swap.hh>
#include <sstream>
#include <istream>
#include <iomanip>

using namespace paludis;

/*
 * Implemented based upon the description in RFC3174.
 */

namespace
{
#if WORDS_BIGENDIAN
    inline uint32_t from_bigendian(uint32_t x)
    {
        return x;
    }

    inline uint32_t to_bigendian(uint32_t x)
    {
        return x;
    }
#else
    inline uint32_t from_bigendian(uint32_t x)
    {
        return byte_swap(x);
    }

    inline uint32_t to_bigendian(uint32_t x)
    {
        return byte_swap(x);
    }
#endif

    template <int n_>
    inline uint32_t s(uint32_t x)
    {
        return (x << n_) | (x >> (32 - n_));
    }

    template <int> uint32_t f(uint32_t, uint32_t, uint32_t);
    template <int> uint32_t k();

    template<> inline uint32_t f<0>(uint32_t b, uint32_t c, uint32_t d)
    {
        return (b & c) | (~b & d);
    }
    template<> inline uint32_t k<0>()
    {
        return 0x5A827999U;
    }

    template<> inline uint32_t f<20>(uint32_t b, uint32_t c, uint32_t d)
    {
        return b ^ c ^ d;
    }
    template<> inline uint32_t k<20>()
    {
        return 0x6ED9EBA1U;
    }

    template<> inline uint32_t f<40>(uint32_t b, uint32_t c, uint32_t d)
    {
        return (b & c) | (b & d) | (c & d);
    }
    template<> inline uint32_t k<40>()
    {
        return 0x8F1BBCDCU;
    }

    template<> inline uint32_t f<60>(uint32_t b, uint32_t c, uint32_t d)
    {
        return b ^ c ^ d;
    }
    template<> inline uint32_t k<60>()
    {
        return 0xCA62C1D6U;
    }

    template <int t_>
    inline void
    process_chunk(uint32_t * w, uint32_t & a, uint32_t & b, uint32_t & c, uint32_t & d, uint32_t & e)
    {
        for (int t = t_; t < t_ + 20; ++t)
        {
            uint32_t temp = s<5>(a) + f<t_>(b, c, d) + e + w[t] + k<t_>();
            e = d;
            d = c;
            c = s<30>(b);
            b = a;
            a = temp;
        }
    }
}

void
SHA1::process_block(uint32_t * w)
{
    uint32_t a, b, c, d, e;

    std::transform(&w[0], &w[16], &w[0], from_bigendian);

    for (int t = 16; t < 80; ++t)
        w[t] = s<1>(w[t - 3] ^ w[t - 8] ^ w[t - 14] ^ w[t - 16]);

    a = h0;
    b = h1;
    c = h2;
    d = h3;
    e = h4;

    process_chunk<0>(w, a, b, c, d, e);
    process_chunk<20>(w, a, b, c, d, e);
    process_chunk<40>(w, a, b, c, d, e);
    process_chunk<60>(w, a, b, c, d, e);

    h0 += a;
    h1 += b;
    h2 += c;
    h3 += d;
    h4 += e;
}


SHA1::SHA1(std::istream & s) :
    h0(0x67452301U),
    h1(0xEFCDAB89U),
    h2(0x98BADCFEU),
    h3(0x10325476U),
    h4(0xC3D2E1F0U)
{
    std::streambuf * buf(s.rdbuf());
    uint64_t size(0);

    union
    {
        uint8_t  m[64];
        uint32_t w[80];
    } block;
    std::streamsize block_size;

    do
    {
        block_size = buf->sgetn(reinterpret_cast<char *>(&block.m[0]), 64);
        size += block_size * 8;

        if (64 != block_size)
        {
            block.m[block_size++] = 0x80U;

            if (56 < block_size)
            {
                std::fill(&block.m[block_size], &block.m[64], 0);
                process_block(block.w);
                block_size = 0;
            }

            std::fill(&block.m[block_size], &block.m[56], 0);
            block.w[14] = to_bigendian((size >> 32) & 0xFFFFFFFFU);
            block.w[15] = to_bigendian(size & 0xFFFFFFFFU);
        }

        process_block(block.w);
    } while (64 == block_size);
}

std::string
SHA1::hexsum() const
{
    std::stringstream result;
    result << std::hex << std::right << std::setfill('0');
    result << std::setw(8) << h0;
    result << std::setw(8) << h1;
    result << std::setw(8) << h2;
    result << std::setw(8) << h3;
    result << std::setw(8) << h4;
    return result.str();
}

