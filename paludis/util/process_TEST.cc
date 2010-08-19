/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010 Ciaran McCreesh
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

#include <paludis/util/process.hh>
#include <paludis/util/fs_entry.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <sstream>

using namespace paludis;
using namespace test;


namespace test_cases
{
    struct TrueTest : TestCase
    {
        TrueTest() : TestCase("true") { }

        void run()
        {
            Process true_process(ProcessCommand({"true"}));
            TEST_CHECK_EQUAL(true_process.run().wait(), 0);
        }
    } test_true;

    struct FalseTest : TestCase
    {
        FalseTest() : TestCase("false") { }

        void run()
        {
            Process false_process(ProcessCommand({"false"}));
            TEST_CHECK_EQUAL(false_process.run().wait(), 1);
        }
    } test_false;

    struct NoWaitTest : TestCase
    {
        NoWaitTest() : TestCase("no wait") { }

        void run()
        {
            Process true_process(ProcessCommand({"true"}));
            TEST_CHECK_THROWS({ RunningProcessHandle handle(true_process.run()); }, ProcessError);
        }
    } test_no_wait;

    struct TwoWaitTest : TestCase
    {
        TwoWaitTest() : TestCase("two wait") { }

        void run()
        {
            Process true_process(ProcessCommand({"true"}));

            RunningProcessHandle handle(true_process.run());
            TEST_CHECK_EQUAL(handle.wait(), 0);
            TEST_CHECK_THROWS(int PALUDIS_ATTRIBUTE((unused)) x(handle.wait()), ProcessError);
        }
    } test_two_wait;

    struct GrabStdoutTest : TestCase
    {
        GrabStdoutTest() : TestCase("grab stdout") { }

        void run()
        {
            std::stringstream stdout_stream;
            Process echo_process(ProcessCommand({"echo", "monkey"}));
            echo_process.capture_stdout(stdout_stream);

            TEST_CHECK_EQUAL(echo_process.run().wait(), 0);
            TEST_CHECK_EQUAL(stdout_stream.str(), "monkey\n");
        }
    } test_grab_stdout;

    struct GrabStderrTest : TestCase
    {
        GrabStderrTest() : TestCase("grab stderr") { }

        void run()
        {
            std::stringstream stderr_stream;
            Process echo_process(ProcessCommand({"sh", "-c", "echo monkey 1>&2"}));
            echo_process.capture_stderr(stderr_stream);

            TEST_CHECK_EQUAL(echo_process.run().wait(), 0);
            TEST_CHECK_EQUAL(stderr_stream.str(), "monkey\n");
        }
    } test_grab_stderr;

    struct GrabStdoutStderrTest : TestCase
    {
        GrabStdoutStderrTest() : TestCase("grab stdout stderr") { }

        void run()
        {
            std::stringstream stdout_stream, stderr_stream;
            Process echo_process(ProcessCommand({"sh", "-c", "echo monkey 1>&2 ; echo chimp"}));
            echo_process.capture_stdout(stdout_stream);
            echo_process.capture_stderr(stderr_stream);

            TEST_CHECK_EQUAL(echo_process.run().wait(), 0);

            TEST_CHECK_EQUAL(stdout_stream.str(), "chimp\n");
            TEST_CHECK_EQUAL(stderr_stream.str(), "monkey\n");
        }
    } test_grab_stdout_stderr;

    struct GrabStdoutLongTest : TestCase
    {
        GrabStdoutLongTest() : TestCase("grab stdout long") { }

        void run()
        {
            std::stringstream stdout_stream;
            Process echo_process(ProcessCommand({"seq", "1", "100000"}));
            echo_process.capture_stdout(stdout_stream);

            TEST_CHECK_EQUAL(echo_process.run().wait(), 0);

            std::string s;
            for (int x(1) ; x <= 100000 ; ++x)
            {
                TEST_CHECK(std::getline(stdout_stream, s));
                TEST_CHECK_STRINGIFY_EQUAL(s, stringify(x));
            }

            TEST_CHECK(! std::getline(stdout_stream, s));
        }
    } test_grab_stdout_long;

    struct SetenvTest : TestCase
    {
        SetenvTest() : TestCase("setenv") { }

        void run()
        {
            std::stringstream stdout_stream;
            Process printenv_process(ProcessCommand({"printenv", "monkey"}));
            printenv_process.capture_stdout(stdout_stream);
            printenv_process.setenv("monkey", "in space");

            TEST_CHECK_EQUAL(printenv_process.run().wait(), 0);
            TEST_CHECK_EQUAL(stdout_stream.str(), "in space\n");
        }
    } test_setenv;

    struct ChdirTest : TestCase
    {
        ChdirTest() : TestCase("chdir") { }

        void run()
        {
            std::stringstream stdout_stream;
            Process pwd_process(ProcessCommand({"pwd"}));
            pwd_process.capture_stdout(stdout_stream);
            pwd_process.chdir(FSEntry("/"));

            TEST_CHECK_EQUAL(pwd_process.run().wait(), 0);
            TEST_CHECK_EQUAL(stdout_stream.str(), "/\n");
        }
    } test_chdir;
}

