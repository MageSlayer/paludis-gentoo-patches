/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010 Ciaran McCreesh
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

#include <paludis/util/system.hh>
#include <paludis/util/log.hh>
#include <paludis/util/thread_pool.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <functional>

#include <cctype>
#include <sched.h>

using namespace test;
using namespace paludis;

namespace test_cases
{
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

    struct GetenvOrErrorTest : TestCase
    {
        GetenvOrErrorTest() : TestCase("getenv_or_error") { }

        void run()
        {
            TEST_CHECK(! getenv_or_error("HOME").empty());
            TEST_CHECK_THROWS(getenv_or_error("THEREISNOSUCHVARIABLE"), GetenvError);
        }
    } test_getenv_or_error;

    struct KernelVersionTest : TestCase
    {
        KernelVersionTest() : TestCase("kernel version") { }

        void run()
        {
            TEST_CHECK(! kernel_version().empty());
#ifdef linux
            TEST_CHECK('2' == kernel_version().at(0));
            TEST_CHECK('.' == kernel_version().at(1));
#elif defined(__FreeBSD__)
            TEST_CHECK(isdigit(kernel_version().at(0)));
            TEST_CHECK('.' == kernel_version().at(1));
#else
#  error You need to write a sanity test for kernel_version() for your platform.
#endif
        }
    } test_kernel_version;
}

