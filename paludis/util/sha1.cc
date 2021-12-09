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

#include <paludis/util/sha1.hh>
#include <paludis/util/byte_swap.hh>
#include <paludis/util/digest_registry.hh>
#include <sstream>
#include <istream>
#include <iomanip>
#include <algorithm>

using namespace paludis;

/*
 * Implemented based upon the description in RFC3174.
 */

namespace
{
    template <int n_>
    inline uint32_t s(uint32_t x)
    {
        return (x << n_) | (x >> (32 - n_));
    }

    template <int, int> struct ChunkParams;

    template<>
    struct ChunkParams<0, 20>
    {
        static uint32_t f(uint32_t b, uint32_t c, uint32_t d)
        {
            return (b & c) | (~b & d);
        }
        static const uint32_t k = 0x5A827999U;
    };

    template<>
    struct ChunkParams<20, 40>
    {
        static uint32_t f(uint32_t b, uint32_t c, uint32_t d)
        {
            return b ^ c ^ d;
        }
        static const uint32_t k = 0x6ED9EBA1U;
    };

    template<>
    struct ChunkParams<40, 60>
    {
        static uint32_t f(uint32_t b, uint32_t c, uint32_t d)
        {
            return (b & c) | (b & d) | (c & d);
        }
        static const uint32_t k = 0x8F1BBCDCU;
    };

    template<>
    struct ChunkParams<60, 80>
    {
        static uint32_t f(uint32_t b, uint32_t c, uint32_t d)
        {
            return b ^ c ^ d;
        }
        static const uint32_t k = 0xCA62C1D6U;
    };

    template <int t_, int u_, int i_ = t_>
    struct ProcessChunk : ChunkParams<t_, u_>
    {
        using ChunkParams<t_, u_>::f;
        using ChunkParams<t_, u_>::k;

        static void process(uint32_t * w, uint32_t & a, uint32_t & b, uint32_t & c, uint32_t & d, uint32_t & e)
        {
            uint32_t temp(s<5>(a) + f(b, c, d) + e + w[i_] + k);
            e = d;
            d = c;
            c = s<30>(b);
            b = a;
            a = temp;
            ProcessChunk<t_, u_, i_ + 1>::process(w, a, b, c, d, e);
        }
    };

    template <int t_, int u_>
    struct ProcessChunk<t_, u_, u_>
    {
        static void process(uint32_t *, uint32_t &, uint32_t &, uint32_t &, uint32_t &, uint32_t &)
        {
        }
    };
}

void
SHA1::process_block(uint32_t * w)
{
    uint32_t a(h0);
    uint32_t b(h1);
    uint32_t c(h2);
    uint32_t d(h3);
    uint32_t e(h4);

    std::transform(&w[0], &w[16], &w[0], from_bigendian<uint32_t>);

    for (int t = 16; t < 80; ++t)
        w[t] = s<1>(w[t - 3] ^ w[t - 8] ^ w[t - 14] ^ w[t - 16]);

    ProcessChunk< 0, 20>::process(w, a, b, c, d, e);
    ProcessChunk<20, 40>::process(w, a, b, c, d, e);
    ProcessChunk<40, 60>::process(w, a, b, c, d, e);
    ProcessChunk<60, 80>::process(w, a, b, c, d, e);

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
        block_size = buf->sgetn(reinterpret_cast<char *>(block.m), 64);
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
            block.w[14] = to_bigendian<uint32_t>((size >> 32) & 0xFFFFFFFFU);
            block.w[15] = to_bigendian<uint32_t>(size & 0xFFFFFFFFU);
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

namespace
{
    DigestRegistry::Registration<SHA1> registration("SHA1");
}

