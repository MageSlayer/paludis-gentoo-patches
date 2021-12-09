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

#include <paludis/util/md5.hh>
#include <paludis/util/digest_registry.hh>
#include <sstream>
#include <istream>
#include <iomanip>

using namespace paludis;

/*
 * Implemented based upon the description in RFC1321.
 */

namespace
{
    inline uint32_t _rl(uint32_t x, unsigned int shift) PALUDIS_ATTRIBUTE((always_inline));
    inline uint32_t _rl(uint32_t x, unsigned int shift)
    {
        return (x << shift) | (x >> (32 - shift));
    }

    inline uint32_t _f(uint32_t x, uint32_t y, uint32_t z) PALUDIS_ATTRIBUTE((always_inline));
    inline uint32_t _f(uint32_t x, uint32_t y, uint32_t z)
    {
        return (x & y) | (~x & z);
    }

    inline uint32_t _g(uint32_t x, uint32_t y, uint32_t z) PALUDIS_ATTRIBUTE((always_inline));
    inline uint32_t _g(uint32_t x, uint32_t y, uint32_t z)
    {
        return (x & z) | (y & ~z);
    }

    inline uint32_t _h(uint32_t x, uint32_t y, uint32_t z) PALUDIS_ATTRIBUTE((always_inline));
    inline uint32_t _h(uint32_t x, uint32_t y, uint32_t z)
    {
        return x ^ y ^ z;
    }

    inline uint32_t _i(uint32_t x, uint32_t y, uint32_t z) PALUDIS_ATTRIBUTE((always_inline));
    inline uint32_t _i(uint32_t x, uint32_t y, uint32_t z)
    {
        return y ^ (x | ~z);
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
MD5::_update(const uint8_t * const block)
{
    uint32_t a(_r[0]);
    uint32_t b(_r[1]);
    uint32_t c(_r[2]);
    uint32_t d(_r[3]);
    uint32_t f;
    uint32_t g;
    uint32_t t;

    for (int i(0) ; i < 16 ; ++i)
    {
        f = _f(b, c, d);
        g = i;

        t = d;
        d = c;
        c = b;
        b = _rl(a + f + _t[i] + _x(g, block), _s[i]) + b;
        a = t;
    }

    for (int i(16) ; i < 32 ; ++i)
    {
        f = _g(b, c, d);
        g = (5 * i + 1) & 0x0f;

        t = d;
        d = c;
        c = b;
        b = _rl(a + f + _t[i] + _x(g, block), _s[i]) + b;
        a = t;
    }

    for (int i(32) ; i < 48 ; ++i)
    {
        f = _h(b, c, d);
        g = (3 * i + 5) & 0x0f;

        t = d;
        d = c;
        c = b;
        b = _rl(a + f + _t[i] + _x(g, block), _s[i]) + b;
        a = t;
    }

    for (int i(48) ; i < 64 ; ++i)
    {
        f = _i(b, c, d);
        g = (7 * i) & 0x0f;

        t = d;
        d = c;
        c = b;
        b = _rl(a + f + _t[i] + _x(g, block), _s[i]) + b;
        a = t;
    }

    _r[0] += a;
    _r[1] += b;
    _r[2] += c;
    _r[3] += d;
}

MD5::MD5(std::istream & stream) :
    _size(0),
    _done_one_pad(false)
{
    _r[0] = 0x67452301;
    _r[1] = 0xefcdab89;
    _r[2] = 0x98badcfe;
    _r[3] = 0x10325476;

    uint8_t buffer[64];
    int c;
    int s(0);
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
MD5::hexsum() const
{
    std::stringstream result;

    for (unsigned int j : _r)
        result << std::hex << std::right << std::setw(8) << std::setfill('0') <<
            _e(static_cast<unsigned int>(j)) << std::flush;

    return result.str();
}

int
MD5::_get(std::istream & stream)
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

const uint8_t MD5::_s[64] = {
    7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,
    5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,
    4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,
    6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21
};

const uint32_t MD5::_t[64] = {
    0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
    0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
    0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
    0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
    0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
    0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
    0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
    0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
    0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
    0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
    0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
    0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
    0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
    0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
    0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
    0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
};

namespace
{
    DigestRegistry::Registration<MD5> registration("MD5");
}

