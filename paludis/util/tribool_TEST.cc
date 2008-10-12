/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 Ciaran McCreesh
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

#include <paludis/util/tribool.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace paludis;
using namespace test;

namespace test_cases
{
    struct TriboolDefaultCtorTest : TestCase
    {
        TriboolDefaultCtorTest() : TestCase("default ctor") { }

        void run()
        {
            Tribool b;
            TEST_CHECK(b.is_false());
            TEST_CHECK(! b.is_true());
            TEST_CHECK(! b.is_indeterminate());
        }
    } test_default_ctor;

    struct TriboolBoolCtorTest : TestCase
    {
        TriboolBoolCtorTest() : TestCase("bool ctor") { }

        void run()
        {
            Tribool b(true);
            TEST_CHECK(! b.is_false());
            TEST_CHECK(b.is_true());
            TEST_CHECK(! b.is_indeterminate());

            Tribool f(false);
            TEST_CHECK(f.is_false());
            TEST_CHECK(! f.is_true());
            TEST_CHECK(! f.is_indeterminate());
        }
    } test_bool_ctor;

    struct TriboolIndetCtorTest : TestCase
    {
        TriboolIndetCtorTest() : TestCase("indet ctor") { }

        void run()
        {
            Tribool b(indeterminate);
            TEST_CHECK(! b.is_false());
            TEST_CHECK(! b.is_true());
            TEST_CHECK(b.is_indeterminate());
        }
    } test_indet_ctor;
}

