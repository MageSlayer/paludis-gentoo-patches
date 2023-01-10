/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2011 David Leverton
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

#include <paludis/util/sha512.hh>
#include <paludis/util/byte_swap.hh>
#include <paludis/util/digest_registry.hh>
#include <sstream>
#include <istream>
#include <iomanip>
#include <limits>
#include <algorithm>

using namespace paludis;

/*
 * Implemented based upon the description in FIPS 180-3.
 */

namespace
{
    template <int n_>
    inline uint64_t rotr(uint64_t x)
    {
        return (x >> n_) | (x << (64 - n_));
    }

    inline uint64_t ch(uint64_t x, uint64_t y, uint64_t z)
    {
        return (x & y) ^ (~x & z);
    }

    inline uint64_t maj(uint64_t x, uint64_t y, uint64_t z)
    {
        return (x & y) ^ (x & z) ^ (y & z);
    }

    inline uint64_t Sigma0(uint64_t x)
    {
        return rotr<28>(x) ^ rotr<34>(x) ^ rotr<39>(x);
    }

    inline uint64_t Sigma1(uint64_t x)
    {
        return rotr<14>(x) ^ rotr<18>(x) ^ rotr<41>(x);
    }

    inline uint64_t sigma0(uint64_t x)
    {
        return rotr<1>(x) ^ rotr<8>(x) ^ (x >> 7);
    }

    inline uint64_t sigma1(uint64_t x)
    {
        return rotr<19>(x) ^ rotr<61>(x) ^ (x >> 6);
    }

    uint64_t k[] = {
        0x428A2F98D728AE22ULL, 0x7137449123EF65CDULL, 0xB5C0FBCFEC4D3B2FULL, 0xE9B5DBA58189DBBCULL,
        0x3956C25BF348B538ULL, 0x59F111F1B605D019ULL, 0x923F82A4AF194F9BULL, 0xAB1C5ED5DA6D8118ULL,
        0xD807AA98A3030242ULL, 0x12835B0145706FBEULL, 0x243185BE4EE4B28CULL, 0x550C7DC3D5FFB4E2ULL,
        0x72BE5D74F27B896FULL, 0x80DEB1FE3B1696B1ULL, 0x9BDC06A725C71235ULL, 0xC19BF174CF692694ULL,
        0xE49B69C19EF14AD2ULL, 0xEFBE4786384F25E3ULL, 0x0FC19DC68B8CD5B5ULL, 0x240CA1CC77AC9C65ULL,
        0x2DE92C6F592B0275ULL, 0x4A7484AA6EA6E483ULL, 0x5CB0A9DCBD41FBD4ULL, 0x76F988DA831153B5ULL,
        0x983E5152EE66DFABULL, 0xA831C66D2DB43210ULL, 0xB00327C898FB213FULL, 0xBF597FC7BEEF0EE4ULL,
        0xC6E00BF33DA88FC2ULL, 0xD5A79147930AA725ULL, 0x06CA6351E003826FULL, 0x142929670A0E6E70ULL,
        0x27B70A8546D22FFCULL, 0x2E1B21385C26C926ULL, 0x4D2C6DFC5AC42AEDULL, 0x53380D139D95B3DFULL,
        0x650A73548BAF63DEULL, 0x766A0ABB3C77B2A8ULL, 0x81C2C92E47EDAEE6ULL, 0x92722C851482353BULL,
        0xA2BFE8A14CF10364ULL, 0xA81A664BBC423001ULL, 0xC24B8B70D0F89791ULL, 0xC76C51A30654BE30ULL,
        0xD192E819D6EF5218ULL, 0xD69906245565A910ULL, 0xF40E35855771202AULL, 0x106AA07032BBD1B8ULL,
        0x19A4C116B8D2D0C8ULL, 0x1E376C085141AB53ULL, 0x2748774CDF8EEB99ULL, 0x34B0BCB5E19B48A8ULL,
        0x391C0CB3C5C95A63ULL, 0x4ED8AA4AE3418ACBULL, 0x5B9CCA4F7763E373ULL, 0x682E6FF3D6B2B8A3ULL,
        0x748F82EE5DEFB2FCULL, 0x78A5636F43172F60ULL, 0x84C87814A1F0AB72ULL, 0x8CC702081A6439ECULL,
        0x90BEFFFA23631E28ULL, 0xA4506CEBDE82BDE9ULL, 0xBEF9A3F7B2C67915ULL, 0xC67178F2E372532BULL,
        0xCA273ECEEA26619CULL, 0xD186B8C721C0C207ULL, 0xEADA7DD6CDE0EB1EULL, 0xF57D4F7FEE6ED178ULL,
        0x06F067AA72176FBAULL, 0x0A637DC5A2C898A6ULL, 0x113F9804BEF90DAEULL, 0x1B710B35131C471BULL,
        0x28DB77F523047D84ULL, 0x32CAAB7B40C72493ULL, 0x3C9EBE0A15C9BEBCULL, 0x431D67C49C100D4CULL,
        0x4CC5D4BECB3E42B6ULL, 0x597F299CFC657E2AULL, 0x5FCB6FAB3AD6FAECULL, 0x6C44198C4A475817ULL
    };
}

void
SHA512::process_block(uint64_t * w)
{
    uint64_t a(h0);
    uint64_t b(h1);
    uint64_t c(h2);
    uint64_t d(h3);
    uint64_t e(h4);
    uint64_t f(h5);
    uint64_t g(h6);
    uint64_t h(h7);

    std::transform(&w[0], &w[16], &w[0], from_bigendian<uint64_t>);

    for (int t = 16; t < 80; ++t)
        w[t] = sigma1(w[t - 2]) + w[t - 7] + sigma0(w[t - 15]) + w[t - 16];

    for (int t = 0; t < 80; ++t)
    {
        uint64_t t1(h + Sigma1(e) + ch(e, f, g) + k[t] + w[t]);
        uint64_t t2(Sigma0(a) + maj(a, b, c));
        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }

    h0 += a;
    h1 += b;
    h2 += c;
    h3 += d;
    h4 += e;
    h5 += f;
    h6 += g;
    h7 += h;
}

SHA512::SHA512(std::istream & s) :
    h0(0x6A09E667F3BCC908ULL),
    h1(0xBB67AE8584CAA73BULL),
    h2(0x3C6EF372FE94F82BULL),
    h3(0xA54FF53A5F1D36F1ULL),
    h4(0x510E527FADE682D1ULL),
    h5(0x9B05688C2B3E6C1FULL),
    h6(0x1F83D9ABFB41BD6BULL),
    h7(0x5BE0CD19137E2179ULL)
{
    std::streambuf * buf(s.rdbuf());
    uint64_t size_l(0);
    uint64_t size_h(0);

    union
    {
        uint8_t  m[128];
        uint64_t w[80];
    } block;
    std::streamsize block_size;

    do
    {
        block_size = buf->sgetn(reinterpret_cast<char *>(block.m), 128);
        if (size_l > std::numeric_limits<uint64_t>::max() - block_size * 8)
            ++size_h;
        size_l += block_size * 8;

        if (128 != block_size)
        {
            block.m[block_size++] = 0x80U;

            if (112 < block_size)
            {
                std::fill(&block.m[block_size], &block.m[128], 0);
                process_block(block.w);
                block_size = 0;
            }

            std::fill(&block.m[block_size], &block.m[112], 0);
            block.w[14] = to_bigendian(size_h);
            block.w[15] = to_bigendian(size_l);
        }

        process_block(block.w);
    } while (128 == block_size);
}

std::string
SHA512::hexsum() const
{
    std::stringstream result;
    result << std::hex << std::right << std::setfill('0');
    result << std::setw(16) << h0;
    result << std::setw(16) << h1;
    result << std::setw(16) << h2;
    result << std::setw(16) << h3;
    result << std::setw(16) << h4;
    result << std::setw(16) << h5;
    result << std::setw(16) << h6;
    result << std::setw(16) << h7;
    return result.str();
}

namespace
{
    DigestRegistry::Registration<SHA512> registration("SHA512");
}

