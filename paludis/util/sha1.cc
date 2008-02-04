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

    inline uint32_t s(int n, uint32_t x)
    {
        return (x << n) | (x >> (32 - n));
    }

    inline uint32_t f_0_19(uint32_t b, uint32_t c, uint32_t d)
    {
        return (b & c) | (~b & d);
    }

    inline uint32_t f_20_39(uint32_t b, uint32_t c, uint32_t d)
    {
        return b ^ c ^ d;
    }

    inline uint32_t f_40_59(uint32_t b, uint32_t c, uint32_t d)
    {
        return (b & c) | (b & d) | (c & d);
    }

    inline uint32_t f_60_79(uint32_t b, uint32_t c, uint32_t d)
    {
        return b ^ c ^ d;
    }

    const uint32_t k_0_19 = 0x5A827999U;
    const uint32_t k_20_39 = 0x6ED9EBA1U;
    const uint32_t k_40_59 = 0x8F1BBCDCU;
    const uint32_t k_60_79 = 0xCA62C1D6U;
}

void
SHA1::process_block(uint32_t * w)
{
    uint32_t a, b, c, d, e, temp;

    std::transform(&w[0], &w[16], &w[0], from_bigendian);

    for (int t = 16; t < 80; ++t)
        w[t] = s(1, w[t - 3] ^ w[t - 8] ^ w[t - 14] ^ w[t - 16]);

    a = h0;
    b = h1;
    c = h2;
    d = h3;
    e = h4;

    for (int t = 0; t < 20; ++t)
    {
        temp = s(5, a) + f_0_19(b, c, d) + e + w[t] + k_0_19;
        e = d;
        d = c;
        c = s(30, b);
        b = a;
        a = temp;
    }

    for (int t = 20; t < 40; ++t)
    {
        temp = s(5, a) + f_20_39(b, c, d) + e + w[t] + k_20_39;
        e = d;
        d = c;
        c = s(30, b);
        b = a;
        a = temp;
    }

    for (int t = 40; t < 60; ++t)
    {
        temp = s(5, a) + f_40_59(b, c, d) + e + w[t] + k_40_59;
        e = d;
        d = c;
        c = s(30, b);
        b = a;
        a = temp;
    }

    for (int t = 60; t < 80; ++t)
    {
        temp = s(5, a) + f_60_79(b, c, d) + e + w[t] + k_60_79;
        e = d;
        d = c;
        c = s(30, b);
        b = a;
        a = temp;
    }

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

