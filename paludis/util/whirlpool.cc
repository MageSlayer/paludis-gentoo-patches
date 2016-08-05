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

#include <paludis/util/whirlpool.hh>
#include <paludis/util/byte_swap.hh>
#include <paludis/util/digest_registry.hh>
#include <sstream>
#include <istream>
#include <iomanip>
#include <limits>
#include <algorithm>
#include <utility>

using namespace paludis;

/*
 * Implemented based upon the description in http://www.larc.usp.br/~pbarreto/WhirlpoolPage.html.
 */

namespace
{
    inline uint64_t
    assemble(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t e, uint8_t f, uint8_t g, uint8_t h)
    {
        return to_bigendian(
               (static_cast<uint64_t>(a) << 56) | (static_cast<uint64_t>(b) << 48)
             | (static_cast<uint64_t>(c) << 40) | (static_cast<uint64_t>(d) << 32)
             | (static_cast<uint64_t>(e) << 24) | (static_cast<uint64_t>(f) << 16)
             | (static_cast<uint64_t>(g) <<  8) |  static_cast<uint64_t>(h));
    }

    template <typename R_, R_ f(uint8_t)>
    struct Precompute
    {
        static const R_ table[256];
    };

    template <typename R_, R_ f_(uint8_t)>
    const R_ Precompute<R_, f_>::table[] = {
#define PRECOMP(X) \
        f_(X + 0x0), f_(X + 0x1), f_(X + 0x2), f_(X + 0x3), \
        f_(X + 0x4), f_(X + 0x5), f_(X + 0x6), f_(X + 0x7), \
        f_(X + 0x8), f_(X + 0x9), f_(X + 0xA), f_(X + 0xB), \
        f_(X + 0xC), f_(X + 0xD), f_(X + 0xE), f_(X + 0xF)
        PRECOMP(0x00), PRECOMP(0x10), PRECOMP(0x20), PRECOMP(0x30),
        PRECOMP(0x40), PRECOMP(0x50), PRECOMP(0x60), PRECOMP(0x70),
        PRECOMP(0x80), PRECOMP(0x90), PRECOMP(0xA0), PRECOMP(0xB0),
        PRECOMP(0xC0), PRECOMP(0xD0), PRECOMP(0xE0), PRECOMP(0xF0)
#undef PRECOMP
    };

    struct FromArray
    {
        const uint64_t *_a;
        FromArray(const uint64_t *a) : _a(a) {}

        uint64_t operator() (int i) const
        {
            return _a[i];
        }
    };

    inline uint8_t
    S(uint8_t u)
    {
        static const uint8_t E[]  = { 1, 0xB, 9, 0xC, 0xD, 6, 0xF, 3, 0xE, 8, 7, 4, 0xA, 2, 5, 0 };
        static const uint8_t E1[] = { 0xF, 0, 0xD, 7, 0xB, 0xE, 5, 0xA, 9, 2, 0xC, 1, 3, 4, 8, 6 };
        static const uint8_t R[]  = { 7, 0xC, 0xB, 0xD, 0xE, 4, 9, 0xF, 6, 3, 8, 0xA, 2, 5, 1, 0 };
        uint8_t e(E[u >> 4]), e1(E1[u & 0xF]), r(R[e ^ e1]);
        return (E[e ^ r] << 4) | E1[r ^ e1];
    }

    template <int bit_, bool flag_>
    struct MultiplyBit
    {
        static uint16_t multiply_bit(uint8_t a)
        {
            return a << bit_;
        }
    };

    template <int bit_>
    struct MultiplyBit<bit_, false>
    {
        static uint16_t multiply_bit(uint8_t)
        {
            return 0;
        }
    };

    template <uint8_t b_, int bit_>
    inline uint16_t
    multiply_bit(uint8_t a)
    {
        return MultiplyBit<bit_, (b_ & (1 << bit_)) != 0>::multiply_bit(a);
    }

    template <int bit_, bool flag_>
    struct ReduceModulo
    {
        static uint16_t reduce_modulo(uint16_t p)
        {
            return p & (0x100 << bit_) ? p ^ (0x11D << bit_) : p;
        }
    };

    template <int bit_>
    struct ReduceModulo<bit_, false>
    {
        static uint16_t reduce_modulo(uint16_t p)
        {
            return p;
        }
    };

    template <uint8_t b_, int bit_>
    inline uint16_t
    reduce_modulo(uint16_t p)
    {
        return ReduceModulo<bit_, b_ >= (2 << bit_)>::reduce_modulo(p);
    }

    // Multiplication in ((Z/2Z)[x])/(x^8+x^4+x^3+x^2+1).  b_ is a template
    // argument because only a few values are actually used in the algorithm,
    // so we can make specialised functions that only operate on the required
    // bits.
    template <uint8_t b_>
    inline uint8_t
    mul(uint8_t a)
    {
        uint16_t p(multiply_bit<b_, 0>(a) ^ multiply_bit<b_, 1>(a)
                 ^ multiply_bit<b_, 2>(a) ^ multiply_bit<b_, 3>(a));
        return reduce_modulo<b_, 0>(reduce_modulo<b_, 1>(reduce_modulo<b_, 2>(p)));
    }

    // Instead of storing the C matrix itself, since each matrix element is only used
    // to multiply, store pointers to the specialised mul<> functions.  Just the first
    // row represented here; the cyclicity is emulated below.
    uint8_t (* const C[])(uint8_t) = {
        mul<1>, mul<1>, mul<4>, mul<1>, mul<8>, mul<5>, mul<2>, mul<9>
    };

    // Optimisation suggested in section 7.1 of the specification
    template <int k_>
    inline uint64_t
    T(uint8_t a)
    {
        return assemble(
            C[( 8 - k_) % 8](S(a)), C[( 9 - k_) % 8](S(a)),
            C[(10 - k_) % 8](S(a)), C[(11 - k_) % 8](S(a)),
            C[(12 - k_) % 8](S(a)), C[(13 - k_) % 8](S(a)),
            C[(14 - k_) % 8](S(a)), C[(15 - k_) % 8](S(a)));
    }

    const uint64_t * const T_table[] = {
        Precompute<uint64_t, T<0> >::table, Precompute<uint64_t, T<1> >::table,
        Precompute<uint64_t, T<2> >::table, Precompute<uint64_t, T<3> >::table,
        Precompute<uint64_t, T<4> >::table, Precompute<uint64_t, T<5> >::table,
        Precompute<uint64_t, T<6> >::table, Precompute<uint64_t, T<7> >::table
    };

    // Fuse theta and gamma together so we can use the fused T lookup tables.
    template <typename A_>
    struct DoThetaGamma
    {
        A_ _a;
        DoThetaGamma(A_ a) : _a(a) {}

        uint64_t operator() (int i) const
        {
            return T_table[0][_a(i, 0)] ^ T_table[1][_a(i, 1)]
                 ^ T_table[2][_a(i, 2)] ^ T_table[3][_a(i, 3)]
                 ^ T_table[4][_a(i, 4)] ^ T_table[5][_a(i, 5)]
                 ^ T_table[6][_a(i, 6)] ^ T_table[7][_a(i, 7)];
        }
    };

    template <typename A_>
    inline DoThetaGamma<A_>
    theta_gamma(A_ a)
    {
        return DoThetaGamma<A_>(a);
    }

    template <typename A_>
    struct DoPi
    {
        A_ _a;
        DoPi(A_ a) : _a(a) {}

        // It's awkward to calculate this row-wise, and it's awkward to use it row-wise
        // in DoThetaGamma where this is used, so make this one byte-wise.  It isn't
        // wasteful to calculate _a(...) and then use only one byte because _a is only
        // ever a FromArray or DoSigma, which are just array lookups.  Of course, all
        // this totally breaks the abstraction....
        uint8_t operator() (int i, int j) const
        {
            // GCC isn't clever enough to optimise the big endian version combined with
            // from_bigendian, so do it manually.
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
            return (_a((i - j + 8) % 8) >> ((7 - j) * 8)) & 0xFF;
#else
            return (_a((i - j + 8) % 8) >> (     j  * 8)) & 0xFF;
#endif
        }
    };

    template <typename A_>
    inline DoPi<A_>
    pi(A_ a)
    {
        return DoPi<A_>(a);
    }

    // Due to the memoisation the class itself doesn't really need to be a
    // template, and making it one slows things down considerably.
    //template <typename K_, typename A_>
    struct DoSigma
    {
        // Memoise, otherwise it takes effectively forever to finish.
        uint64_t _s[8];

        template <typename K_, typename A_>
        DoSigma(K_ k, A_ a)
        {
            // Manually unrolling this significantly speeds it up.
            _s[0] = a(0) ^ k(0);
            _s[1] = a(1) ^ k(1);
            _s[2] = a(2) ^ k(2);
            _s[3] = a(3) ^ k(3);
            _s[4] = a(4) ^ k(4);
            _s[5] = a(5) ^ k(5);
            _s[6] = a(6) ^ k(6);
            _s[7] = a(7) ^ k(7);
        }

        uint64_t operator() (int i) const
        {
            return _s[i];
        }
    };

    template <typename K_, typename A_>
    inline DoSigma/*<K_, A_>*/
    sigma(K_ k, A_ a)
    {
        return DoSigma/*<K_, A_>*/(k, a);
    }

    template <int r_>
    inline FromArray
    c()
    {
        static const uint64_t data[8] = {
            assemble(
                S(8 * (r_ - 1) + 0), S(8 * (r_ - 1) + 1),
                S(8 * (r_ - 1) + 2), S(8 * (r_ - 1) + 3),
                S(8 * (r_ - 1) + 4), S(8 * (r_ - 1) + 5),
                S(8 * (r_ - 1) + 6), S(8 * (r_ - 1) + 7))
            // Remaining rows are all zeros.
        };
        return FromArray(data);
    }

    template <typename K_, typename A_>
    struct DoRho
    {
        typedef DoSigma/*<K_, DoThetaGamma<DoPi<A_> > >*/ Type;
    };

    template <typename K_, typename A_>
    inline typename DoRho<K_, A_>::Type
    rho(K_ k, A_ a)
    {
        // gamma and pi transposed relative to the specification so theta and
        // gamma can be fused.  This works because gamma works on each individual
        // element, whereas pi rearranges elements regardless of their value.
        return sigma(k, theta_gamma(pi(a)));
    }

    template <int r_, typename K_> struct DoK;
    template <int r_, typename K_> inline typename DoK<r_, K_>::Type K(K_ k);

    template <int r_, typename K_>
    struct DoK
    {
        typedef typename DoRho<
            FromArray,
            /*typename DoK<r_ - 1, K_>::Type*/ K_
            >::Type Type;

        // For efficiency, this expects the K from the previous round, rather than
        // the initial K.
        static Type doK(K_ k)
        {
            return rho(c<r_>(), /*K<r_ - 1, K_>(k)*/ k);
        }
    };

    template <typename K_>
    struct DoK<0, K_>
    {
        typedef K_ Type;

        static Type doK(K_ k)
        {
            return k;
        }
    };

    template <int r_, typename K_>
    inline typename DoK<r_, K_>::Type
    K(K_ k)
    {
        return DoK<r_, K_>::doK(k);
    }

    template <int R_, typename K_, typename A_> struct DoW;
    template <int R_, typename K_, typename A_> inline typename DoW<R_, K_, A_>::Type W(K_ k, A_ a);

    template <int R_, typename K_, typename A_>
    struct DoW
    {
        typedef std::pair<
            typename DoRho<
                typename DoK<R_, typename DoW<R_ - 1, K_, A_>::Type::second_type>::Type,
                typename DoW<R_ - 1, K_, A_>::Type::first_type
                >::Type,
            typename DoK<R_, typename DoW<R_ - 1, K_, A_>::Type::second_type>::Type
            > Type;

        static Type doW(K_ k, A_ a)
        {
            // Pass K back up so it can be used to calculate the K for the next round.
            auto prev(W<R_ - 1>(k, a));
            auto new_k(K<R_>(prev.second));
            return Type(rho(new_k, prev.first), new_k);
        }
    };

    template <typename K_, typename A_>
    struct DoW<0, K_, A_>
    {
        typedef std::pair<
            DoSigma/*<typename DoK<0, K_>::Type, A_>*/,
            typename DoK<0, K_>::Type
            > Type;

        static Type doW(K_ k, A_ a)
        {
            auto new_k(K<0>(k));
            return Type(sigma(new_k, a), new_k);
        }
    };

    template <int R_, typename K_, typename A_>
    inline typename DoW<R_, K_, A_>::Type
    W(K_ k, A_ a)
    {
        return DoW<R_, K_, A_>::doW(k, a);
    }

    template <int R_ = 10>
    inline typename DoW<R_, FromArray, FromArray>::Type::first_type
    W(const uint64_t *k, const uint64_t *a)
    {
        return W<R_>(FromArray(k), FromArray(a)).first;
    }
}

void
Whirlpool::process_block(const uint64_t *eta)
{
    auto w(W(H, eta));

    // This is only safe because sigma (and hence rho and hence W) memoises;
    // otherwise we'd be overwriting values that we still need.
    for (int i = 0; i < 8; ++i)
        H[i] = w(i) ^ H[i] ^ eta[i];
}

Whirlpool::Whirlpool(std::istream & s)
{
    std::fill(&H[0], &H[8], 0);

    std::streambuf * buf(s.rdbuf());
    uint64_t size_1(0), size_2(0), size_3(0), size_4(0);

    union mu
    {
        uint8_t m[64];
        // Nominally uint8_t[8][8], but for efficiency we process an entire row
        // at a time where possible.
        uint64_t eta[8];
    } block;
    std::streamsize block_size;

    do
    {
        block_size = buf->sgetn(reinterpret_cast<char *>(block.m), 64);
        if (size_1 > std::numeric_limits<uint64_t>::max() - block_size * 8)
        {
            if (size_2 == std::numeric_limits<uint64_t>::max())
            {
                if (size_3 == std::numeric_limits<uint64_t>::max())
                {
                    ++size_4;
                }
                ++size_3;
            }
            ++size_2;
        }
        size_1 += block_size * 8;

        if (64 != block_size)
        {
            block.m[block_size++] = 0x80U;

            if (32 < block_size)
            {
                std::fill(&block.m[block_size], &block.m[64], 0);
                process_block(block.eta);
                block_size = 0;
            }

            std::fill(&block.m[block_size], &block.m[32], 0);
            block.eta[4] = to_bigendian(size_4);
            block.eta[5] = to_bigendian(size_3);
            block.eta[6] = to_bigendian(size_2);
            block.eta[7] = to_bigendian(size_1);
        }

        process_block(block.eta);
    } while (64 == block_size);
}

std::string
Whirlpool::hexsum() const
{
    std::stringstream result;
    result << std::hex << std::right << std::setfill('0');
    for (unsigned long i : H)
        result << std::setw(16) << from_bigendian(i);
    return result.str();
}

namespace
{
    DigestRegistry::Registration<Whirlpool> registration("WHIRLPOOL");
}

