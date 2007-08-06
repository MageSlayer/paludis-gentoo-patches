/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <paludis/util/future-impl.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace paludis;
using namespace test;

namespace
{
    int f()
    {
        return 42;
    }

    int g()
    {
        Future<int> r1(&f), r2(&f), r3(&f), r4(&f);
        return r1() + r2() + r3() - r4();
    }

    void h(int & i)
    {
        i = 42;
    }
}

namespace test_cases
{
    struct FutureTest : TestCase
    {
        FutureTest() : TestCase("future") { }

        void run()
        {
            Future<int> f1(&f);
            TEST_CHECK_EQUAL(f1(), 42);
            TEST_CHECK_EQUAL(f1(), 42);
        }
    } test_future;

    struct FutureFutureTest : TestCase
    {
        FutureFutureTest() : TestCase("future future") { }

        void run()
        {
            Future<int> f1(&g), f2(&g);
            TEST_CHECK_EQUAL(f1(), 84);
            TEST_CHECK_EQUAL(f2(), 84);
        }
    } test_future_future;

    struct VoidFutureTest : TestCase
    {
        VoidFutureTest() : TestCase("void future") { }

        void run()
        {
            int x(17), y(23);
            {
                Future<void> f1(tr1::bind(&h, tr1::ref(x))), f2(tr1::bind(&h, tr1::ref(y)));
                f1();
                TEST_CHECK_EQUAL(x, 42);
            }
            TEST_CHECK_EQUAL(y, 42);
        }
    } test_void_future;
}


