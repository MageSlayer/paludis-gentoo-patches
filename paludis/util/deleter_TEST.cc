/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

#include <algorithm>
#include <paludis/util/deleter.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <vector>

using namespace test;
using namespace paludis;

/** \file
 * Test cases for deleter.hh.
 *
 * \ingroup Test
 */

#ifndef DOXYGEN
struct MyClass
{
    static int instances;

    MyClass()
    {
        ++instances;
    }

    ~MyClass()
    {
        --instances;
    }
};

int MyClass::instances = 0;
#endif

namespace test_cases
{
    /**
     * Test Deleter.
     *
     * \ingroup Test
     */
    struct DeleterTest : TestCase
    {
        DeleterTest() : TestCase("deleter") { }

        void run()
        {
            std::vector<MyClass *> v;
            TEST_CHECK_EQUAL(MyClass::instances, 0);
            v.push_back(new MyClass);
            TEST_CHECK_EQUAL(MyClass::instances, 1);
            v.push_back(new MyClass);
            TEST_CHECK_EQUAL(MyClass::instances, 2);
            v.push_back(new MyClass);
            TEST_CHECK_EQUAL(MyClass::instances, 3);
            std::for_each(v.begin(), v.end(), Deleter());
            TEST_CHECK_EQUAL(MyClass::instances, 0);
        }
    } test_deleter;
}
