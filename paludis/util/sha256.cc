/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh
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

#include "sha256.hh"
#include <paludis/util/attributes.hh>
#include <paludis/util/digest_registry.hh>
#include <istream>
#include <iomanip>
#include <sstream>

using namespace paludis;

/*
 * Implemented based upon the description at:
 *   http://csrc.nist.gov/cryptval/shs/sha256-384-512.pdf
 */

namespace
{
    inline uint32_t ch(uint32_t x, uint32_t y, uint32_t z) PALUDIS_ATTRIBUTE((always_inline));
    inline uint32_t ch(uint32_t x, uint32_t y, uint32_t z)
    {
        return (x & y) ^ (~x & z);
    }

    inline uint32_t maj(uint32_t x, uint32_t y, uint32_t z) PALUDIS_ATTRIBUTE((always_inline));
    inline uint32_t maj(uint32_t x, uint32_t y, uint32_t z)
    {
        return (x & y) ^ (x & z) ^ (y & z);
    }

    inline uint32_t rr(uint32_t x, unsigned int shift) PALUDIS_ATTRIBUTE((always_inline));
    inline uint32_t rr(uint32_t x, unsigned int shift)
    {
        return (x >> shift) | (x << (32 - shift));
    }

    inline uint32_t sigma0(uint32_t x) PALUDIS_ATTRIBUTE((always_inline));
    inline uint32_t sigma0(uint32_t x)
    {
        return rr(x, 2) ^ rr(x, 13) ^ rr(x, 22);
    }

    inline uint32_t sigma1(uint32_t x) PALUDIS_ATTRIBUTE((always_inline));
    inline uint32_t sigma1(uint32_t x)
    {
        return rr(x, 6) ^ rr(x, 11) ^ rr(x, 25);
    }

    inline uint32_t lsigma0(uint32_t x) PALUDIS_ATTRIBUTE((always_inline));
    inline uint32_t lsigma0(uint32_t x)
    {
        return rr(x, 7) ^ rr(x, 18) ^ (x >> 3);
    }

    inline uint32_t lsigma1(uint32_t x) PALUDIS_ATTRIBUTE((always_inline));
    inline uint32_t lsigma1(uint32_t x)
    {
        return rr(x, 17) ^ rr(x, 19) ^ (x >> 10);
    }

    inline void wload(unsigned j, const uint8_t * const block,
            uint32_t * const dest) PALUDIS_ATTRIBUTE((always_inline));
    inline void wload(unsigned j, const uint8_t * const block,
            uint32_t * const dest)
    {
        dest[j] =
            (block[(j << 2) + 0] << 24) |
            (block[(j << 2) + 1] << 16) |
            (block[(j << 2) + 2] << 8) |
            (block[(j << 2) + 3]);
    }

    inline void wblend(unsigned j, uint32_t * const dest) PALUDIS_ATTRIBUTE((always_inline));
    inline void wblend(unsigned j, uint32_t * const dest)
    {
        dest[j] = lsigma1(dest[j - 2]) + dest[j - 7] + lsigma0(dest[j - 15]) + dest[j - 16];
    }
}

void
SHA256::_update(const uint8_t * const block)
{
    uint32_t a(_h[0]), b(_h[1]), c(_h[2]), d(_h[3]), e(_h[4]), f(_h[5]),
             g(_h[6]), h(_h[7]);

    uint32_t w[64];

    for (uint32_t j(0) ; j < 16 ; ++j)
        wload(j, block, &w[0]);

    for (uint32_t j(16) ; j <= 63 ; ++j)
        wblend(j, &w[0]);

    for (uint32_t j(0) ; j <= 63 ; ++j)
    {
        uint32_t t1(h + sigma1(e) + ch(e, f, g) + _k[j] + w[j]);
        uint32_t t2(sigma0(a) + maj(a, b, c));
        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }

    _h[0] += a;
    _h[1] += b;
    _h[2] += c;
    _h[3] += d;
    _h[4] += e;
    _h[5] += f;
    _h[6] += g;
    _h[7] += h;
}

SHA256::SHA256(std::istream & stream) :
    _size(0),
    _done_one_pad(false)
{
    _h[0] = 0x6a09e667;
    _h[1] = 0xbb67ae85;
    _h[2] = 0x3c6ef372;
    _h[3] = 0xa54ff53a;
    _h[4] = 0x510e527f;
    _h[5] = 0x9b05688c;
    _h[6] = 0x1f83d9ab;
    _h[7] = 0x5be0cd19;

    uint8_t buffer[64];
    int c, s(0);
    while (-1 != ((c = _get(stream))))
    {
        buffer[s++] = c;
        if (64 == s)
        {
            _update(&buffer[0]);
            s = 0;
        }
    }
    while (56 != s)
    {
        buffer[s++] = 0;
        if (64 == s)
        {
            _update(&buffer[0]);
            s = 0;
        }
    }

    buffer[56] = static_cast<uint8_t>(_size >> (7 * 8));
    buffer[57] = static_cast<uint8_t>(_size >> (6 * 8));
    buffer[58] = static_cast<uint8_t>(_size >> (5 * 8));
    buffer[59] = static_cast<uint8_t>(_size >> (4 * 8));
    buffer[60] = static_cast<uint8_t>(_size >> (3 * 8));
    buffer[61] = static_cast<uint8_t>(_size >> (2 * 8));
    buffer[62] = static_cast<uint8_t>(_size >> (1 * 8));
    buffer[63] = static_cast<uint8_t>(_size >> (0 * 8));
    _update(&buffer[0]);
}

std::string
SHA256::hexsum() const
{
    std::stringstream result;

    for (int j(0) ; j < 8 ; ++j)
        result << std::hex << std::right << std::setw(8) << std::setfill('0') <<
            static_cast<unsigned int>(_h[j]) << std::flush;

    return result.str();
}

int
SHA256::_get(std::istream & stream)
{
    char c;
    if (stream.get(c))
    {
        _size += 8;
        return static_cast<unsigned char>(c);
    }
    else if (! _done_one_pad)
    {
        _done_one_pad = true;
        return 0x80;
    }
    else
        return -1;
}

const uint32_t
paludis::SHA256::_k[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

namespace
{
    DigestRegistry::Registration<SHA256> registration("SHA256");
}

