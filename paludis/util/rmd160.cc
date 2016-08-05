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

#include "rmd160.hh"
#include <paludis/util/attributes.hh>
#include <paludis/util/digest_registry.hh>
#include <sstream>
#include <istream>
#include <iomanip>

using namespace paludis;

/*
 * Implemented based upon the description at:
 *   http://homes.esat.kuleuven.be/~bosselae/ripemd160.html
 */

namespace
{
    inline uint32_t rl(uint32_t x, unsigned int shift) PALUDIS_ATTRIBUTE((always_inline));
    inline uint32_t rl(uint32_t x, unsigned int shift)
    {
        return (x << shift) | (x >> (32 - shift));
    }

    inline uint32_t _f(uint32_t j, uint32_t x, uint32_t y, uint32_t z) PALUDIS_ATTRIBUTE((always_inline));
    inline uint32_t _f(uint32_t j, uint32_t x, uint32_t y, uint32_t z)
    {
        switch (j / 16)
        {
            case 0:
                return x ^ y ^ z;

            case 1:
                return (x & y) | (~x & z);

            case 2:
                return (x | ~y) ^ z;

            case 3:
                return (x & z) | (y & ~z);

            case 4:
                return x ^ (y | ~z);
        }

        throw 0;
    }

    inline uint32_t _x(unsigned i, const uint8_t * const block) PALUDIS_ATTRIBUTE((always_inline));
    inline uint32_t _x(unsigned i, const uint8_t * const block)
    {
        return
            (block[(i << 2) + 3] << 24) |
            (block[(i << 2) + 2] << 16) |
            (block[(i << 2) + 1] << 8) |
            (block[(i << 2) + 0]);
    }

    inline uint32_t _e(const uint32_t x) PALUDIS_ATTRIBUTE((always_inline));
    inline uint32_t _e(const uint32_t x)
    {
        return
            ((x & 0xff) << 24) |
            ((x & 0xff00) << 8) |
            ((x & 0xff0000) >> 8) |
            ((x & 0xff000000) >> 24);
    }
}

void
RMD160::_update(const uint8_t * const block)
{
    uint32_t a(_h[0]), b(_h[1]), c(_h[2]), d(_h[3]), e(_h[4]);
    uint32_t ap(_h[0]), bp(_h[1]), cp(_h[2]), dp(_h[3]), ep(_h[4]);
    uint32_t t;

    for (unsigned j(0) ; j <= 79 ; ++j)
    {
        t = a + _f(j, b, c, d) + _x(_r[j], block) + _k[j / 16];
        t = rl(t, _s[j]) + e;
        a = e;
        e = d;
        d = rl(c, 10);
        c = b;
        b = t;

        t = ap + _f(79 - j, bp, cp, dp) + _x(_rp[j], block) + _kp[j / 16];
        t = rl(t, _sp[j]) + ep;
        ap = ep;
        ep = dp;
        dp = rl(cp, 10);
        cp = bp;
        bp = t;
    }

    t = _h[1] + c + dp;
    _h[1] = _h[2] + d + ep;
    _h[2] = _h[3] + e + ap;
    _h[3] = _h[4] + a + bp;
    _h[4] = _h[0] + b + cp;
    _h[0] = t;
}

RMD160::RMD160(std::istream & stream) :
    _size(0),
    _done_one_pad(false)
{
    _h[0] = 0x67452301;
    _h[1] = 0xefcdab89;
    _h[2] = 0x98badcfe;
    _h[3] = 0x10325476;
    _h[4] = 0xc3d2e1f0;

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

    buffer[56] = static_cast<uint8_t>(_size >> (0 * 8));
    buffer[57] = static_cast<uint8_t>(_size >> (1 * 8));
    buffer[58] = static_cast<uint8_t>(_size >> (2 * 8));
    buffer[59] = static_cast<uint8_t>(_size >> (3 * 8));
    buffer[60] = static_cast<uint8_t>(_size >> (4 * 8));
    buffer[61] = static_cast<uint8_t>(_size >> (5 * 8));
    buffer[62] = static_cast<uint8_t>(_size >> (6 * 8));
    buffer[63] = static_cast<uint8_t>(_size >> (7 * 8));
    _update(&buffer[0]);
}

std::string
RMD160::hexsum() const
{
    std::stringstream result;

    for (unsigned int j : _h)
        result << std::hex << std::right << std::setw(8) << std::setfill('0') <<
            _e(static_cast<unsigned int>(j)) << std::flush;

    return result.str();
}

int
RMD160::_get(std::istream & stream)
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

const uint8_t RMD160::_r[80] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
    7, 4, 13, 1, 10, 6, 15, 3, 12, 0, 9, 5, 2, 14, 11, 8,
    3, 10, 14, 4, 9, 15, 8, 1, 2, 7, 0, 6, 13, 11, 5, 12,
    1, 9, 11, 10, 0, 8, 12, 4, 13, 3, 7, 15, 14, 5, 6, 2,
    4, 0, 5, 9, 7, 12, 2, 10, 14, 1, 3, 8, 11, 6, 15, 13
};

const uint8_t RMD160::_rp[80] = {
    5, 14, 7, 0, 9, 2, 11, 4, 13, 6, 15, 8, 1, 10, 3, 12,
    6, 11, 3, 7, 0, 13, 5, 10, 14, 15, 8, 12, 4, 9, 1, 2,
    15, 5, 1, 3, 7, 14, 6, 9, 11, 8, 12, 2, 10, 0, 4, 13,
    8, 6, 4, 1, 3, 11, 15, 0, 5, 12, 2, 13, 9, 7, 10, 14,
    12, 15, 10, 4, 1, 5, 8, 7, 6, 2, 13, 14, 0, 3, 9, 11
};

const uint8_t RMD160::_s[80] = {
    11, 14, 15, 12, 5, 8, 7, 9, 11, 13, 14, 15, 6, 7, 9, 8,
    7, 6, 8, 13, 11, 9, 7, 15, 7, 12, 15, 9, 11, 7, 13, 12,
    11, 13, 6, 7, 14, 9, 13, 15, 14, 8, 13, 6, 5, 12, 7, 5,
    11, 12, 14, 15, 14, 15, 9, 8, 9, 14, 5, 6, 8, 6, 5, 12,
    9, 15, 5, 11, 6, 8, 13, 12, 5, 12, 13, 14, 11, 8, 5, 6
};

const uint8_t RMD160::_sp[80] = {
    8, 9, 9, 11, 13, 15, 15, 5, 7, 7, 8, 11, 14, 14, 12, 6,
    9, 13, 15, 7, 12, 8, 9, 11, 7, 7, 12, 7, 6, 15, 13, 11,
    9, 7, 15, 11, 8, 6, 6, 14, 12, 13, 5, 14, 13, 13, 7, 5,
    15, 5, 8, 11, 14, 14, 6, 14, 6, 9, 12, 9, 12, 5, 15, 8,
    8, 5, 12, 9, 12, 5, 14, 6, 8, 13, 6, 5, 15, 13, 11, 11
};

const uint32_t RMD160::_k[5] = {
    0x00000000, 0x5a827999, 0x6ed9eba1, 0x8f1bbcdc, 0xa953fd4e
};

const uint32_t RMD160::_kp[5] = {
    0x50a28be6, 0x5c4dd124, 0x6d703ef3, 0x7a6d76e9, 0x00000000
};

namespace
{
    DigestRegistry::Registration<RMD160> registration("RMD160");
}

