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

#include <paludis/util/is_const.hh>
#include <string>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace test;
using namespace paludis;

/**
 * \file
 * Test cases for is_const.hh .
 */

namespace test_cases
{
    /**
     * \test Test IsConst.
     */
    struct IsConstTest : TestCase
    {
        IsConstTest() : TestCase("is const") { }

        void run()
        {
            TEST_CHECK(IsConst<const int>::value);
            TEST_CHECK(IsConst<const std::string>::value);
            TEST_CHECK(IsConst<const std::string &>::value);
            TEST_CHECK(IsConst<int * const>::value);

            TEST_CHECK(! IsConst<int>::value);
            TEST_CHECK(! IsConst<std::string>::value);
            TEST_CHECK(! IsConst<std::string &>::value);
            TEST_CHECK(! IsConst<const int *>::value);
        }
    } test_is_const;
}



