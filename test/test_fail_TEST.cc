/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <string>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace test;

/** \file
 * This test should fail.
 *
 * \ingroup Test
 */

namespace test_cases
{
    /**
     * \test This test should fail.
     */
    struct FailTest : TestCase
    {
        FailTest() : TestCase("test the test code: this should fail") { }

        void run()
        {
            TEST_CHECK_STRINGIFY_EQUAL("this comparison", "should fail");
        }
    } test_fail;
}

