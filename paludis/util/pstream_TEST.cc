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

#include <paludis/util/pstream.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace paludis;
using namespace test;

/** \file
 * Tests for PStream.
 *
 */

namespace test_cases
{
    /**
     * \test Test PStream on a normal command.
     *
     */
    struct PStreamTest : TestCase
    {
        PStreamTest() : TestCase("pstream") { }

        void run()
        {
            PStream * p;
            TEST_CHECK((p = new PStream("echo hi")));
            std::string line;
            TEST_CHECK(std::getline(*p, line));
            TEST_CHECK_EQUAL(line, "hi");
            TEST_CHECK(! std::getline(*p, line));
            TEST_CHECK_EQUAL(p->exit_status(), 0);
            delete p;
        }
    } test_pstream;

    /**
     * \test Test PStream on a command that doesn't exist.
     *
     */
    struct PStreamNoExistTest : TestCase
    {
        PStreamNoExistTest() : TestCase("pstream nonexistent command") { }

        void run()
        {
            PStream p("thiscommanddoesnotexist 2>/dev/null");
            TEST_CHECK(p.exit_status() != 0);
        }
    } test_pstream_no_exist;

    /**
     * \test Test PStream on a command that returns a failure with no output.
     *
     */
    struct PStreamSilentFailTest : TestCase
    {
        PStreamSilentFailTest() : TestCase("pstream silent fail") { }

        void run()
        {
            PStream p("test -e /doesnotexist");
            TEST_CHECK(p.exit_status() != 0);
        }
    } test_pstream_silent_fail;

    /**
     * \test Test PStream on a command that fails with output.
     *
     */
    struct PStreamFailTest : TestCase
    {
        PStreamFailTest() : TestCase("pstream fail") { }

        void run()
        {
            PStream * p;
            TEST_CHECK((p = new PStream("cat /doesnotexist 2>&1")));
            std::string line;
            TEST_CHECK(std::getline(*p, line));
            TEST_CHECK(! line.empty());
            TEST_CHECK(! std::getline(*p, line));
            TEST_CHECK(p->exit_status() != 0);
            delete p;
        }
    } test_pstream_fail;
}

