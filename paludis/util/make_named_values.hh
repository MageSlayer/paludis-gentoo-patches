/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_MAKE_NAMED_VALUES_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_MAKE_NAMED_VALUES_HH 1

/** \file
 * A load of make_named_values functions.
 *
 * Bizarre oddity in C++98: you can only use an initialiser list when using
 * equals to initialise a newly constructed object. C++0x fixes this, but in the
 * mean time we can hack around it with this mess...
 */

namespace paludis
{
#ifdef PALUDIS_HAVE_VARIADIC_TEMPLATES

#  ifdef PALUDIS_HAVE_RVALUE_REFERENCES

    template <typename R_, typename... T_>
    R_ make_named_values(T_ && ... a)
    {
        R_ result = { a... };
        return result;
    }

#  else

    template <typename R_, typename... T_>
    R_ make_named_values(const T_ & ... a)
    {
        R_ result = { a... };
        return result;
    }

#  endif

#else

    template <typename R_, typename T1_>
    R_ make_named_values(const T1_ & v1)
    {
        R_ result = { v1 };
        return result;
    }

    template <typename R_, typename T1_, typename T2_>
    R_ make_named_values(const T1_ & v1, const T2_ & v2)
    {
        R_ result = { v1, v2 };
        return result;
    }

    template <typename R_, typename T1_, typename T2_, typename T3_>
    R_ make_named_values(const T1_ & v1, const T2_ & v2, const T3_ & v3)
    {
        R_ result = { v1, v2, v3 };
        return result;
    }

    template <typename R_, typename T1_, typename T2_, typename T3_, typename T4_>
    R_ make_named_values(const T1_ & v1, const T2_ & v2, const T3_ & v3, const T4_ & v4)
    {
        R_ result = { v1, v2, v3, v4 };
        return result;
    }

    template <typename R_, typename T1_, typename T2_, typename T3_, typename T4_, typename T5_>
    R_ make_named_values(const T1_ & v1, const T2_ & v2, const T3_ & v3, const T4_ & v4, const T5_ & v5)
    {
        R_ result = { v1, v2, v3, v4, v5 };
        return result;
    }

    template <typename R_, typename T1_, typename T2_, typename T3_, typename T4_, typename T5_,
             typename T6_>
    R_ make_named_values(const T1_ & v1, const T2_ & v2, const T3_ & v3, const T4_ & v4, const T5_ & v5,
            const T6_ & v6)
    {
        R_ result = { v1, v2, v3, v4, v5, v6 };
        return result;
    }

    template <typename R_, typename T1_, typename T2_, typename T3_, typename T4_, typename T5_,
             typename T6_, typename T7_>
    R_ make_named_values(const T1_ & v1, const T2_ & v2, const T3_ & v3, const T4_ & v4, const T5_ & v5,
            const T6_ & v6, const T7_ & v7)
    {
        R_ result = { v1, v2, v3, v4, v5, v6, v7 };
        return result;
    }

    template <typename R_, typename T1_, typename T2_, typename T3_, typename T4_, typename T5_,
             typename T6_, typename T7_, typename T8_>
    R_ make_named_values(const T1_ & v1, const T2_ & v2, const T3_ & v3, const T4_ & v4, const T5_ & v5,
            const T6_ & v6, const T7_ & v7, const T8_ & v8)
    {
        R_ result = { v1, v2, v3, v4, v5, v6, v7, v8 };
        return result;
    }

    template <typename R_, typename T1_, typename T2_, typename T3_, typename T4_, typename T5_,
             typename T6_, typename T7_, typename T8_, typename T9_>
     R_ make_named_values(const T1_ & v1, const T2_ & v2, const T3_ & v3, const T4_ & v4, const T5_ & v5,
            const T6_ & v6, const T7_ & v7, const T8_ & v8, const T9_ & v9)
    {
        R_ result = { v1, v2, v3, v4, v5, v6, v7, v8, v9 };
        return result;
    }

    template <typename R_, typename T1_, typename T2_, typename T3_, typename T4_, typename T5_,
             typename T6_, typename T7_, typename T8_, typename T9_, typename T10_>
     R_ make_named_values(const T1_ & v1, const T2_ & v2, const T3_ & v3, const T4_ & v4, const T5_ & v5,
            const T6_ & v6, const T7_ & v7, const T8_ & v8, const T9_ & v9, const T10_ & v10)
    {
        R_ result = { v1, v2, v3, v4, v5, v6, v7, v8, v9, v10 };
        return result;
    }

    template <typename R_, typename T1_, typename T2_, typename T3_, typename T4_, typename T5_,
             typename T6_, typename T7_, typename T8_, typename T9_, typename T10_,
             typename T11_>
     R_ make_named_values(const T1_ & v1, const T2_ & v2, const T3_ & v3, const T4_ & v4, const T5_ & v5,
            const T6_ & v6, const T7_ & v7, const T8_ & v8, const T9_ & v9, const T10_ & v10,
            const T11_ & v11)
    {
        R_ result = { v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11 };
        return result;
    }

    template <typename R_, typename T1_, typename T2_, typename T3_, typename T4_, typename T5_,
             typename T6_, typename T7_, typename T8_, typename T9_, typename T10_,
             typename T11_, typename T12_>
     R_ make_named_values(const T1_ & v1, const T2_ & v2, const T3_ & v3, const T4_ & v4, const T5_ & v5,
            const T6_ & v6, const T7_ & v7, const T8_ & v8, const T9_ & v9, const T10_ & v10,
            const T11_ & v11, const T12_ & v12)
    {
        R_ result = { v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12 };
        return result;
    }

    template <typename R_, typename T1_, typename T2_, typename T3_, typename T4_, typename T5_,
             typename T6_, typename T7_, typename T8_, typename T9_, typename T10_,
             typename T11_, typename T12_, typename T13_>
     R_ make_named_values(const T1_ & v1, const T2_ & v2, const T3_ & v3, const T4_ & v4, const T5_ & v5,
            const T6_ & v6, const T7_ & v7, const T8_ & v8, const T9_ & v9, const T10_ & v10,
            const T11_ & v11, const T12_ & v12, const T13_ & v13)
    {
        R_ result = { v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13 };
        return result;
    }

    template <typename R_, typename T1_, typename T2_, typename T3_, typename T4_, typename T5_,
             typename T6_, typename T7_, typename T8_, typename T9_, typename T10_,
             typename T11_, typename T12_, typename T13_, typename T14_>
     R_ make_named_values(const T1_ & v1, const T2_ & v2, const T3_ & v3, const T4_ & v4, const T5_ & v5,
            const T6_ & v6, const T7_ & v7, const T8_ & v8, const T9_ & v9, const T10_ & v10,
            const T11_ & v11, const T12_ & v12, const T13_ & v13, const T14_ & v14)
    {
        R_ result = { v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14 };
        return result;
    }

    template <typename R_, typename T1_, typename T2_, typename T3_, typename T4_, typename T5_,
             typename T6_, typename T7_, typename T8_, typename T9_, typename T10_,
             typename T11_, typename T12_, typename T13_, typename T14_, typename T15_>
     R_ make_named_values(const T1_ & v1, const T2_ & v2, const T3_ & v3, const T4_ & v4, const T5_ & v5,
            const T6_ & v6, const T7_ & v7, const T8_ & v8, const T9_ & v9, const T10_ & v10,
            const T11_ & v11, const T12_ & v12, const T13_ & v13, const T14_ & v14, const T15_ & v15)
    {
        R_ result = { v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15 };
        return result;
    }

    template <typename R_, typename T1_, typename T2_, typename T3_, typename T4_, typename T5_,
             typename T6_, typename T7_, typename T8_, typename T9_, typename T10_,
             typename T11_, typename T12_, typename T13_, typename T14_, typename T15_,
             typename T16_>
     R_ make_named_values(const T1_ & v1, const T2_ & v2, const T3_ & v3, const T4_ & v4, const T5_ & v5,
            const T6_ & v6, const T7_ & v7, const T8_ & v8, const T9_ & v9, const T10_ & v10,
            const T11_ & v11, const T12_ & v12, const T13_ & v13, const T14_ & v14, const T15_ & v15,
            const T16_ & v16)
    {
        R_ result = { v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16 };
        return result;
    }

    template <typename R_, typename T1_, typename T2_, typename T3_, typename T4_, typename T5_,
             typename T6_, typename T7_, typename T8_, typename T9_, typename T10_,
             typename T11_, typename T12_, typename T13_, typename T14_, typename T15_,
             typename T16_, typename T17_>
     R_ make_named_values(const T1_ & v1, const T2_ & v2, const T3_ & v3, const T4_ & v4, const T5_ & v5,
            const T6_ & v6, const T7_ & v7, const T8_ & v8, const T9_ & v9, const T10_ & v10,
            const T11_ & v11, const T12_ & v12, const T13_ & v13, const T14_ & v14, const T15_ & v15,
            const T16_ & v16, const T17_ & v17)
    {
        R_ result = { v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17 };
        return result;
    }

    template <typename R_, typename T1_, typename T2_, typename T3_, typename T4_, typename T5_,
             typename T6_, typename T7_, typename T8_, typename T9_, typename T10_,
             typename T11_, typename T12_, typename T13_, typename T14_, typename T15_,
             typename T16_, typename T17_, typename T18_>
     R_ make_named_values(const T1_ & v1, const T2_ & v2, const T3_ & v3, const T4_ & v4, const T5_ & v5,
            const T6_ & v6, const T7_ & v7, const T8_ & v8, const T9_ & v9, const T10_ & v10,
            const T11_ & v11, const T12_ & v12, const T13_ & v13, const T14_ & v14, const T15_ & v15,
            const T16_ & v16, const T17_ & v17, const T18_ & v18)
    {
        R_ result = { v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18 };
        return result;
    }

    template <typename R_, typename T1_, typename T2_, typename T3_, typename T4_, typename T5_,
             typename T6_, typename T7_, typename T8_, typename T9_, typename T10_,
             typename T11_, typename T12_, typename T13_, typename T14_, typename T15_,
             typename T16_, typename T17_, typename T18_, typename T19_>
     R_ make_named_values(const T1_ & v1, const T2_ & v2, const T3_ & v3, const T4_ & v4, const T5_ & v5,
            const T6_ & v6, const T7_ & v7, const T8_ & v8, const T9_ & v9, const T10_ & v10,
            const T11_ & v11, const T12_ & v12, const T13_ & v13, const T14_ & v14, const T15_ & v15,
            const T16_ & v16, const T17_ & v17, const T18_ & v18, const T19_ & v19)
    {
        R_ result = { v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19 };
        return result;
    }

    template <typename R_, typename T1_, typename T2_, typename T3_, typename T4_, typename T5_,
             typename T6_, typename T7_, typename T8_, typename T9_, typename T10_,
             typename T11_, typename T12_, typename T13_, typename T14_, typename T15_,
             typename T16_, typename T17_, typename T18_, typename T19_, typename T20_>
     R_ make_named_values(const T1_ & v1, const T2_ & v2, const T3_ & v3, const T4_ & v4, const T5_ & v5,
            const T6_ & v6, const T7_ & v7, const T8_ & v8, const T9_ & v9, const T10_ & v10,
            const T11_ & v11, const T12_ & v12, const T13_ & v13, const T14_ & v14, const T15_ & v15,
            const T16_ & v16, const T17_ & v17, const T18_ & v18, const T19_ & v19, const T20_ & v20)
    {
        R_ result = { v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20 };
        return result;
    }

    template <typename R_, typename T1_, typename T2_, typename T3_, typename T4_, typename T5_,
             typename T6_, typename T7_, typename T8_, typename T9_, typename T10_,
             typename T11_, typename T12_, typename T13_, typename T14_, typename T15_,
             typename T16_, typename T17_, typename T18_, typename T19_, typename T20_,
             typename T21_>
     R_ make_named_values(const T1_ & v1, const T2_ & v2, const T3_ & v3, const T4_ & v4, const T5_ & v5,
            const T6_ & v6, const T7_ & v7, const T8_ & v8, const T9_ & v9, const T10_ & v10,
            const T11_ & v11, const T12_ & v12, const T13_ & v13, const T14_ & v14, const T15_ & v15,
            const T16_ & v16, const T17_ & v17, const T18_ & v18, const T19_ & v19, const T20_ & v20,
            const T21_ & v21)
    {
        R_ result = { v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20,
            v21 };
        return result;
    }

    template <typename R_, typename T1_, typename T2_, typename T3_, typename T4_, typename T5_,
             typename T6_, typename T7_, typename T8_, typename T9_, typename T10_,
             typename T11_, typename T12_, typename T13_, typename T14_, typename T15_,
             typename T16_, typename T17_, typename T18_, typename T19_, typename T20_,
             typename T21_, typename T22_>
     R_ make_named_values(const T1_ & v1, const T2_ & v2, const T3_ & v3, const T4_ & v4, const T5_ & v5,
            const T6_ & v6, const T7_ & v7, const T8_ & v8, const T9_ & v9, const T10_ & v10,
            const T11_ & v11, const T12_ & v12, const T13_ & v13, const T14_ & v14, const T15_ & v15,
            const T16_ & v16, const T17_ & v17, const T18_ & v18, const T19_ & v19, const T20_ & v20,
            const T21_ & v21, const T22_ & v22)
    {
        R_ result = { v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20,
            v21, v22 };
        return result;
    }

    template <typename R_, typename T1_, typename T2_, typename T3_, typename T4_, typename T5_,
             typename T6_, typename T7_, typename T8_, typename T9_, typename T10_,
             typename T11_, typename T12_, typename T13_, typename T14_, typename T15_,
             typename T16_, typename T17_, typename T18_, typename T19_, typename T20_,
             typename T21_, typename T22_, typename T23_>
     R_ make_named_values(const T1_ & v1, const T2_ & v2, const T3_ & v3, const T4_ & v4, const T5_ & v5,
            const T6_ & v6, const T7_ & v7, const T8_ & v8, const T9_ & v9, const T10_ & v10,
            const T11_ & v11, const T12_ & v12, const T13_ & v13, const T14_ & v14, const T15_ & v15,
            const T16_ & v16, const T17_ & v17, const T18_ & v18, const T19_ & v19, const T20_ & v20,
            const T21_ & v21, const T22_ & v22, const T23_ & v23)
    {
        R_ result = { v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20,
            v21, v22, v23 };
        return result;
    }

    template <typename R_, typename T1_, typename T2_, typename T3_, typename T4_, typename T5_,
             typename T6_, typename T7_, typename T8_, typename T9_, typename T10_,
             typename T11_, typename T12_, typename T13_, typename T14_, typename T15_,
             typename T16_, typename T17_, typename T18_, typename T19_, typename T20_,
             typename T21_, typename T22_, typename T23_, typename T24_>
     R_ make_named_values(const T1_ & v1, const T2_ & v2, const T3_ & v3, const T4_ & v4, const T5_ & v5,
            const T6_ & v6, const T7_ & v7, const T8_ & v8, const T9_ & v9, const T10_ & v10,
            const T11_ & v11, const T12_ & v12, const T13_ & v13, const T14_ & v14, const T15_ & v15,
            const T16_ & v16, const T17_ & v17, const T18_ & v18, const T19_ & v19, const T20_ & v20,
            const T21_ & v21, const T22_ & v22, const T23_ & v23, const T24_ & v24)
    {
        R_ result = { v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20,
            v21, v22, v23, v24 };
        return result;
    }

    template <typename R_, typename T1_, typename T2_, typename T3_, typename T4_, typename T5_,
             typename T6_, typename T7_, typename T8_, typename T9_, typename T10_,
             typename T11_, typename T12_, typename T13_, typename T14_, typename T15_,
             typename T16_, typename T17_, typename T18_, typename T19_, typename T20_,
             typename T21_, typename T22_, typename T23_, typename T24_, typename T25_>
     R_ make_named_values(const T1_ & v1, const T2_ & v2, const T3_ & v3, const T4_ & v4, const T5_ & v5,
            const T6_ & v6, const T7_ & v7, const T8_ & v8, const T9_ & v9, const T10_ & v10,
            const T11_ & v11, const T12_ & v12, const T13_ & v13, const T14_ & v14, const T15_ & v15,
            const T16_ & v16, const T17_ & v17, const T18_ & v18, const T19_ & v19, const T20_ & v20,
            const T21_ & v21, const T22_ & v22, const T23_ & v23, const T24_ & v24, const T25_ & v25)
    {
        R_ result = { v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20,
            v21, v22, v23, v24, v25 };
        return result;
    }

    template <typename R_, typename T1_, typename T2_, typename T3_, typename T4_, typename T5_,
             typename T6_, typename T7_, typename T8_, typename T9_, typename T10_,
             typename T11_, typename T12_, typename T13_, typename T14_, typename T15_,
             typename T16_, typename T17_, typename T18_, typename T19_, typename T20_,
             typename T21_, typename T22_, typename T23_, typename T24_, typename T25_,
             typename T26_>
     R_ make_named_values(const T1_ & v1, const T2_ & v2, const T3_ & v3, const T4_ & v4, const T5_ & v5,
            const T6_ & v6, const T7_ & v7, const T8_ & v8, const T9_ & v9, const T10_ & v10,
            const T11_ & v11, const T12_ & v12, const T13_ & v13, const T14_ & v14, const T15_ & v15,
            const T16_ & v16, const T17_ & v17, const T18_ & v18, const T19_ & v19, const T20_ & v20,
            const T21_ & v21, const T22_ & v22, const T23_ & v23, const T24_ & v24, const T25_ & v25,
            const T26_ & v26)
    {
        R_ result = { v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20,
            v21, v22, v23, v24, v25, v26 };
        return result;
    }

    template <typename R_, typename T1_, typename T2_, typename T3_, typename T4_, typename T5_,
             typename T6_, typename T7_, typename T8_, typename T9_, typename T10_,
             typename T11_, typename T12_, typename T13_, typename T14_, typename T15_,
             typename T16_, typename T17_, typename T18_, typename T19_, typename T20_,
             typename T21_, typename T22_, typename T23_, typename T24_, typename T25_,
             typename T26_, typename T27_>
     R_ make_named_values(const T1_ & v1, const T2_ & v2, const T3_ & v3, const T4_ & v4, const T5_ & v5,
            const T6_ & v6, const T7_ & v7, const T8_ & v8, const T9_ & v9, const T10_ & v10,
            const T11_ & v11, const T12_ & v12, const T13_ & v13, const T14_ & v14, const T15_ & v15,
            const T16_ & v16, const T17_ & v17, const T18_ & v18, const T19_ & v19, const T20_ & v20,
            const T21_ & v21, const T22_ & v22, const T23_ & v23, const T24_ & v24, const T25_ & v25,
            const T26_ & v26, const T27_ & v27)
    {
        R_ result = { v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20,
            v21, v22, v23, v24, v25, v26, v27 };
        return result;
    }

    template <typename R_, typename T1_, typename T2_, typename T3_, typename T4_, typename T5_,
             typename T6_, typename T7_, typename T8_, typename T9_, typename T10_,
             typename T11_, typename T12_, typename T13_, typename T14_, typename T15_,
             typename T16_, typename T17_, typename T18_, typename T19_, typename T20_,
             typename T21_, typename T22_, typename T23_, typename T24_, typename T25_,
             typename T26_, typename T27_, typename T28_>
     R_ make_named_values(const T1_ & v1, const T2_ & v2, const T3_ & v3, const T4_ & v4, const T5_ & v5,
            const T6_ & v6, const T7_ & v7, const T8_ & v8, const T9_ & v9, const T10_ & v10,
            const T11_ & v11, const T12_ & v12, const T13_ & v13, const T14_ & v14, const T15_ & v15,
            const T16_ & v16, const T17_ & v17, const T18_ & v18, const T19_ & v19, const T20_ & v20,
            const T21_ & v21, const T22_ & v22, const T23_ & v23, const T24_ & v24, const T25_ & v25,
            const T26_ & v26, const T27_ & v27, const T28_ & v28)
    {
        R_ result = { v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20,
            v21, v22, v23, v24, v25, v26, v27, v28 };
        return result;
    }

    template <typename R_, typename T1_, typename T2_, typename T3_, typename T4_, typename T5_,
             typename T6_, typename T7_, typename T8_, typename T9_, typename T10_,
             typename T11_, typename T12_, typename T13_, typename T14_, typename T15_,
             typename T16_, typename T17_, typename T18_, typename T19_, typename T20_,
             typename T21_, typename T22_, typename T23_, typename T24_, typename T25_,
             typename T26_, typename T27_, typename T28_, typename T29_>
     R_ make_named_values(const T1_ & v1, const T2_ & v2, const T3_ & v3, const T4_ & v4, const T5_ & v5,
            const T6_ & v6, const T7_ & v7, const T8_ & v8, const T9_ & v9, const T10_ & v10,
            const T11_ & v11, const T12_ & v12, const T13_ & v13, const T14_ & v14, const T15_ & v15,
            const T16_ & v16, const T17_ & v17, const T18_ & v18, const T19_ & v19, const T20_ & v20,
            const T21_ & v21, const T22_ & v22, const T23_ & v23, const T24_ & v24, const T25_ & v25,
            const T26_ & v26, const T27_ & v27, const T28_ & v28, const T29_ & v29)
    {
        R_ result = { v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20,
            v21, v22, v23, v24, v25, v26, v27, v28, v29 };
        return result;
    }

    template <typename R_, typename T1_, typename T2_, typename T3_, typename T4_, typename T5_,
             typename T6_, typename T7_, typename T8_, typename T9_, typename T10_,
             typename T11_, typename T12_, typename T13_, typename T14_, typename T15_,
             typename T16_, typename T17_, typename T18_, typename T19_, typename T20_,
             typename T21_, typename T22_, typename T23_, typename T24_, typename T25_,
             typename T26_, typename T27_, typename T28_, typename T29_, typename T30_>
     R_ make_named_values(const T1_ & v1, const T2_ & v2, const T3_ & v3, const T4_ & v4, const T5_ & v5,
            const T6_ & v6, const T7_ & v7, const T8_ & v8, const T9_ & v9, const T10_ & v10,
            const T11_ & v11, const T12_ & v12, const T13_ & v13, const T14_ & v14, const T15_ & v15,
            const T16_ & v16, const T17_ & v17, const T18_ & v18, const T19_ & v19, const T20_ & v20,
            const T21_ & v21, const T22_ & v22, const T23_ & v23, const T24_ & v24, const T25_ & v25,
            const T26_ & v26, const T27_ & v27, const T28_ & v28, const T29_ & v29, const T30_ & v30)
    {
        R_ result = { v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20,
            v21, v22, v23, v24, v25, v26, v27, v28, v29, v30 };
        return result;
    }

    template <typename R_, typename T1_, typename T2_, typename T3_, typename T4_, typename T5_,
             typename T6_, typename T7_, typename T8_, typename T9_, typename T10_,
             typename T11_, typename T12_, typename T13_, typename T14_, typename T15_,
             typename T16_, typename T17_, typename T18_, typename T19_, typename T20_,
             typename T21_, typename T22_, typename T23_, typename T24_, typename T25_,
             typename T26_, typename T27_, typename T28_, typename T29_, typename T30_,
             typename T31_>
     R_ make_named_values(const T1_ & v1, const T2_ & v2, const T3_ & v3, const T4_ & v4, const T5_ & v5,
            const T6_ & v6, const T7_ & v7, const T8_ & v8, const T9_ & v9, const T10_ & v10,
            const T11_ & v11, const T12_ & v12, const T13_ & v13, const T14_ & v14, const T15_ & v15,
            const T16_ & v16, const T17_ & v17, const T18_ & v18, const T19_ & v19, const T20_ & v20,
            const T21_ & v21, const T22_ & v22, const T23_ & v23, const T24_ & v24, const T25_ & v25,
            const T26_ & v26, const T27_ & v27, const T28_ & v28, const T29_ & v29, const T30_ & v30,
            const T31_ & v31)
    {
        R_ result = { v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20,
            v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31 };
        return result;
    }

    template <typename R_, typename T1_, typename T2_, typename T3_, typename T4_, typename T5_,
             typename T6_, typename T7_, typename T8_, typename T9_, typename T10_,
             typename T11_, typename T12_, typename T13_, typename T14_, typename T15_,
             typename T16_, typename T17_, typename T18_, typename T19_, typename T20_,
             typename T21_, typename T22_, typename T23_, typename T24_, typename T25_,
             typename T26_, typename T27_, typename T28_, typename T29_, typename T30_,
             typename T31_, typename T32_>
     R_ make_named_values(const T1_ & v1, const T2_ & v2, const T3_ & v3, const T4_ & v4, const T5_ & v5,
            const T6_ & v6, const T7_ & v7, const T8_ & v8, const T9_ & v9, const T10_ & v10,
            const T11_ & v11, const T12_ & v12, const T13_ & v13, const T14_ & v14, const T15_ & v15,
            const T16_ & v16, const T17_ & v17, const T18_ & v18, const T19_ & v19, const T20_ & v20,
            const T21_ & v21, const T22_ & v22, const T23_ & v23, const T24_ & v24, const T25_ & v25,
            const T26_ & v26, const T27_ & v27, const T28_ & v28, const T29_ & v29, const T30_ & v30,
            const T31_ & v31, const T32_ & v32)
    {
        R_ result = { v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20,
            v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32 };
        return result;
    }

    template <typename R_, typename T1_, typename T2_, typename T3_, typename T4_, typename T5_,
             typename T6_, typename T7_, typename T8_, typename T9_, typename T10_,
             typename T11_, typename T12_, typename T13_, typename T14_, typename T15_,
             typename T16_, typename T17_, typename T18_, typename T19_, typename T20_,
             typename T21_, typename T22_, typename T23_, typename T24_, typename T25_,
             typename T26_, typename T27_, typename T28_, typename T29_, typename T30_,
             typename T31_, typename T32_, typename T33_>
     R_ make_named_values(const T1_ & v1, const T2_ & v2, const T3_ & v3, const T4_ & v4, const T5_ & v5,
            const T6_ & v6, const T7_ & v7, const T8_ & v8, const T9_ & v9, const T10_ & v10,
            const T11_ & v11, const T12_ & v12, const T13_ & v13, const T14_ & v14, const T15_ & v15,
            const T16_ & v16, const T17_ & v17, const T18_ & v18, const T19_ & v19, const T20_ & v20,
            const T21_ & v21, const T22_ & v22, const T23_ & v23, const T24_ & v24, const T25_ & v25,
            const T26_ & v26, const T27_ & v27, const T28_ & v28, const T29_ & v29, const T30_ & v30,
            const T31_ & v31, const T32_ & v32, const T33_ & v33)
    {
        R_ result = { v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20,
            v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32, v33 };
        return result;
    }

    template <typename R_, typename T1_, typename T2_, typename T3_, typename T4_, typename T5_,
             typename T6_, typename T7_, typename T8_, typename T9_, typename T10_,
             typename T11_, typename T12_, typename T13_, typename T14_, typename T15_,
             typename T16_, typename T17_, typename T18_, typename T19_, typename T20_,
             typename T21_, typename T22_, typename T23_, typename T24_, typename T25_,
             typename T26_, typename T27_, typename T28_, typename T29_, typename T30_,
             typename T31_, typename T32_, typename T33_, typename T34_>
     R_ make_named_values(const T1_ & v1, const T2_ & v2, const T3_ & v3, const T4_ & v4, const T5_ & v5,
            const T6_ & v6, const T7_ & v7, const T8_ & v8, const T9_ & v9, const T10_ & v10,
            const T11_ & v11, const T12_ & v12, const T13_ & v13, const T14_ & v14, const T15_ & v15,
            const T16_ & v16, const T17_ & v17, const T18_ & v18, const T19_ & v19, const T20_ & v20,
            const T21_ & v21, const T22_ & v22, const T23_ & v23, const T24_ & v24, const T25_ & v25,
            const T26_ & v26, const T27_ & v27, const T28_ & v28, const T29_ & v29, const T30_ & v30,
            const T31_ & v31, const T32_ & v32, const T33_ & v33, const T34_ & v34)
    {
        R_ result = { v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20,
            v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32, v33, v34 };
        return result;
    }

    template <typename R_, typename T1_, typename T2_, typename T3_, typename T4_, typename T5_,
             typename T6_, typename T7_, typename T8_, typename T9_, typename T10_,
             typename T11_, typename T12_, typename T13_, typename T14_, typename T15_,
             typename T16_, typename T17_, typename T18_, typename T19_, typename T20_,
             typename T21_, typename T22_, typename T23_, typename T24_, typename T25_,
             typename T26_, typename T27_, typename T28_, typename T29_, typename T30_,
             typename T31_, typename T32_, typename T33_, typename T34_, typename T35_>
     R_ make_named_values(const T1_ & v1, const T2_ & v2, const T3_ & v3, const T4_ & v4, const T5_ & v5,
            const T6_ & v6, const T7_ & v7, const T8_ & v8, const T9_ & v9, const T10_ & v10,
            const T11_ & v11, const T12_ & v12, const T13_ & v13, const T14_ & v14, const T15_ & v15,
            const T16_ & v16, const T17_ & v17, const T18_ & v18, const T19_ & v19, const T20_ & v20,
            const T21_ & v21, const T22_ & v22, const T23_ & v23, const T24_ & v24, const T25_ & v25,
            const T26_ & v26, const T27_ & v27, const T28_ & v28, const T29_ & v29, const T30_ & v30,
            const T31_ & v31, const T32_ & v32, const T33_ & v33, const T34_ & v34, const T35_ & v35)
    {
        R_ result = { v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20,
            v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32, v33, v34, v35 };
        return result;
    }

    template <typename R_, typename T1_, typename T2_, typename T3_, typename T4_, typename T5_,
             typename T6_, typename T7_, typename T8_, typename T9_, typename T10_,
             typename T11_, typename T12_, typename T13_, typename T14_, typename T15_,
             typename T16_, typename T17_, typename T18_, typename T19_, typename T20_,
             typename T21_, typename T22_, typename T23_, typename T24_, typename T25_,
             typename T26_, typename T27_, typename T28_, typename T29_, typename T30_,
             typename T31_, typename T32_, typename T33_, typename T34_, typename T35_,
             typename T36_>
     R_ make_named_values(const T1_ & v1, const T2_ & v2, const T3_ & v3, const T4_ & v4, const T5_ & v5,
            const T6_ & v6, const T7_ & v7, const T8_ & v8, const T9_ & v9, const T10_ & v10,
            const T11_ & v11, const T12_ & v12, const T13_ & v13, const T14_ & v14, const T15_ & v15,
            const T16_ & v16, const T17_ & v17, const T18_ & v18, const T19_ & v19, const T20_ & v20,
            const T21_ & v21, const T22_ & v22, const T23_ & v23, const T24_ & v24, const T25_ & v25,
            const T26_ & v26, const T27_ & v27, const T28_ & v28, const T29_ & v29, const T30_ & v30,
            const T31_ & v31, const T32_ & v32, const T33_ & v33, const T34_ & v34, const T35_ & v35,
            const T36_ & v36)
    {
        R_ result = { v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20,
            v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32, v33, v34, v35, v36 };
        return result;
    }

    template <typename R_, typename T1_, typename T2_, typename T3_, typename T4_, typename T5_,
             typename T6_, typename T7_, typename T8_, typename T9_, typename T10_,
             typename T11_, typename T12_, typename T13_, typename T14_, typename T15_,
             typename T16_, typename T17_, typename T18_, typename T19_, typename T20_,
             typename T21_, typename T22_, typename T23_, typename T24_, typename T25_,
             typename T26_, typename T27_, typename T28_, typename T29_, typename T30_,
             typename T31_, typename T32_, typename T33_, typename T34_, typename T35_,
             typename T36_, typename T37_>
     R_ make_named_values(const T1_ & v1, const T2_ & v2, const T3_ & v3, const T4_ & v4, const T5_ & v5,
            const T6_ & v6, const T7_ & v7, const T8_ & v8, const T9_ & v9, const T10_ & v10,
            const T11_ & v11, const T12_ & v12, const T13_ & v13, const T14_ & v14, const T15_ & v15,
            const T16_ & v16, const T17_ & v17, const T18_ & v18, const T19_ & v19, const T20_ & v20,
            const T21_ & v21, const T22_ & v22, const T23_ & v23, const T24_ & v24, const T25_ & v25,
            const T26_ & v26, const T27_ & v27, const T28_ & v28, const T29_ & v29, const T30_ & v30,
            const T31_ & v31, const T32_ & v32, const T33_ & v33, const T34_ & v34, const T35_ & v35,
            const T36_ & v36, const T37_ & v37)
    {
        R_ result = { v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20,
            v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32, v33, v34, v35, v36, v37 };
        return result;
    }

    template <typename R_, typename T1_, typename T2_, typename T3_, typename T4_, typename T5_,
             typename T6_, typename T7_, typename T8_, typename T9_, typename T10_,
             typename T11_, typename T12_, typename T13_, typename T14_, typename T15_,
             typename T16_, typename T17_, typename T18_, typename T19_, typename T20_,
             typename T21_, typename T22_, typename T23_, typename T24_, typename T25_,
             typename T26_, typename T27_, typename T28_, typename T29_, typename T30_,
             typename T31_, typename T32_, typename T33_, typename T34_, typename T35_,
             typename T36_, typename T37_, typename T38_>
     R_ make_named_values(const T1_ & v1, const T2_ & v2, const T3_ & v3, const T4_ & v4, const T5_ & v5,
            const T6_ & v6, const T7_ & v7, const T8_ & v8, const T9_ & v9, const T10_ & v10,
            const T11_ & v11, const T12_ & v12, const T13_ & v13, const T14_ & v14, const T15_ & v15,
            const T16_ & v16, const T17_ & v17, const T18_ & v18, const T19_ & v19, const T20_ & v20,
            const T21_ & v21, const T22_ & v22, const T23_ & v23, const T24_ & v24, const T25_ & v25,
            const T26_ & v26, const T27_ & v27, const T28_ & v28, const T29_ & v29, const T30_ & v30,
            const T31_ & v31, const T32_ & v32, const T33_ & v33, const T34_ & v34, const T35_ & v35,
            const T36_ & v36, const T37_ & v37, const T38_ & v38)
    {
        R_ result = { v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20,
            v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32, v33, v34, v35, v36, v37, v38 };
        return result;
    }

    template <typename R_, typename T1_, typename T2_, typename T3_, typename T4_, typename T5_,
             typename T6_, typename T7_, typename T8_, typename T9_, typename T10_,
             typename T11_, typename T12_, typename T13_, typename T14_, typename T15_,
             typename T16_, typename T17_, typename T18_, typename T19_, typename T20_,
             typename T21_, typename T22_, typename T23_, typename T24_, typename T25_,
             typename T26_, typename T27_, typename T28_, typename T29_, typename T30_,
             typename T31_, typename T32_, typename T33_, typename T34_, typename T35_,
             typename T36_, typename T37_, typename T38_, typename T39_>
     R_ make_named_values(const T1_ & v1, const T2_ & v2, const T3_ & v3, const T4_ & v4, const T5_ & v5,
            const T6_ & v6, const T7_ & v7, const T8_ & v8, const T9_ & v9, const T10_ & v10,
            const T11_ & v11, const T12_ & v12, const T13_ & v13, const T14_ & v14, const T15_ & v15,
            const T16_ & v16, const T17_ & v17, const T18_ & v18, const T19_ & v19, const T20_ & v20,
            const T21_ & v21, const T22_ & v22, const T23_ & v23, const T24_ & v24, const T25_ & v25,
            const T26_ & v26, const T27_ & v27, const T28_ & v28, const T29_ & v29, const T30_ & v30,
            const T31_ & v31, const T32_ & v32, const T33_ & v33, const T34_ & v34, const T35_ & v35,
            const T36_ & v36, const T37_ & v37, const T38_ & v38, const T39_ & v39)
    {
        R_ result = { v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20,
            v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32, v33, v34, v35, v36, v37, v38, v39 };
        return result;
    }

    template <typename R_, typename T1_, typename T2_, typename T3_, typename T4_, typename T5_,
             typename T6_, typename T7_, typename T8_, typename T9_, typename T10_,
             typename T11_, typename T12_, typename T13_, typename T14_, typename T15_,
             typename T16_, typename T17_, typename T18_, typename T19_, typename T20_,
             typename T21_, typename T22_, typename T23_, typename T24_, typename T25_,
             typename T26_, typename T27_, typename T28_, typename T29_, typename T30_,
             typename T31_, typename T32_, typename T33_, typename T34_, typename T35_,
             typename T36_, typename T37_, typename T38_, typename T39_, typename T40_>
     R_ make_named_values(const T1_ & v1, const T2_ & v2, const T3_ & v3, const T4_ & v4, const T5_ & v5,
            const T6_ & v6, const T7_ & v7, const T8_ & v8, const T9_ & v9, const T10_ & v10,
            const T11_ & v11, const T12_ & v12, const T13_ & v13, const T14_ & v14, const T15_ & v15,
            const T16_ & v16, const T17_ & v17, const T18_ & v18, const T19_ & v19, const T20_ & v20,
            const T21_ & v21, const T22_ & v22, const T23_ & v23, const T24_ & v24, const T25_ & v25,
            const T26_ & v26, const T27_ & v27, const T28_ & v28, const T29_ & v29, const T30_ & v30,
            const T31_ & v31, const T32_ & v32, const T33_ & v33, const T34_ & v34, const T35_ & v35,
            const T36_ & v36, const T37_ & v37, const T38_ & v38, const T39_ & v39, const T40_ & v40)
    {
        R_ result = { v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20,
            v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32, v33, v34, v35, v36, v37, v38, v39, v40 };
        return result;
    }

#endif
}

#endif
