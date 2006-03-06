/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

#include <paludis/util/getenv.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace test;
using namespace paludis;

namespace test_cases
{

    /**
     * \test Test getenv_with_default.
     *
     * \ingroup Test
     */
    struct GetenvWithDefaultTest : TestCase
    {
        GetenvWithDefaultTest() : TestCase("getenv_with_default") { }

        void run()
        {
            TEST_CHECK(! getenv_with_default("HOME", "!").empty());
            TEST_CHECK_EQUAL(getenv_with_default("HOME", "!").at(0), '/');
            TEST_CHECK_EQUAL(getenv_with_default("THEREISNOSUCHVARIABLE", "moo"), "moo");
        }
    } test_getenv_with_default;

    /**
     * \test Test getenv_or_error.
     *
     * \ingroup Test
     */
    struct GetenvOrErrorTest : TestCase
    {
        GetenvOrErrorTest() : TestCase("getenv_or_error") { }

        void run()
        {
            TEST_CHECK(! getenv_or_error("HOME").empty());
            TEST_CHECK_THROWS(getenv_or_error("THEREISNOSUCHVARIABLE"), GetenvError);
        }
    } test_getenv_or_error;
}
