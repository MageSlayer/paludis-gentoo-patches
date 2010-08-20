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
#include <paludis/util/pipe.hh>
#include <paludis/util/safe_ofstream.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <sstream>
#include <sys/types.h>
#include <pwd.h>

using namespace paludis;
using namespace test;

namespace
{
    std::string response_handler(const std::string & s)
    {
        if (s == "ONE")
            return "1";
        else if (s == "TWO")
            return "2";
        else if (s == "THREE")
            return "3";
        else if (s == "FOUR")
            return "4";
        else
            return "9";
    }
}

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

    struct NoPtyTest : TestCase
    {
        NoPtyTest() : TestCase("no pty") { }

        void run()
        {
            std::stringstream stdout_stream, stderr_stream;
            Process test_t_process(ProcessCommand({"test", "-t", "1", "-o", "-t", "2"}));
            test_t_process.capture_stdout(stdout_stream);
            test_t_process.capture_stderr(stderr_stream);
            TEST_CHECK_EQUAL(test_t_process.run().wait(), 1);
        }
    } test_no_pty;

    struct PtyTest : TestCase
    {
        PtyTest() : TestCase("pty") { }

        void run()
        {
            std::stringstream stdout_stream, stderr_stream;
            Process test_t_process(ProcessCommand({"test", "-t", "1", "-a", "-t", "2"}));
            test_t_process.capture_stdout(stdout_stream);
            test_t_process.capture_stderr(stderr_stream);
            test_t_process.use_ptys();

            TEST_CHECK_EQUAL(test_t_process.run().wait(), 0);
        }
    } test_ptys;

    struct SetuidTest : TestCase
    {
        SetuidTest() : TestCase("setuid") { }

        void run()
        {
            if (0 != getuid())
                return;

            std::stringstream stdout_stream;
            Process whoami_process(ProcessCommand({"sh", "-c", "whoami ; groups"}));
            whoami_process.capture_stdout(stdout_stream);

            struct passwd * nobody(getpwnam("nobody"));
            if (nobody)
            {
                whoami_process.setuid_setgid(nobody->pw_uid, nobody->pw_gid);
                TEST_CHECK_EQUAL(whoami_process.run().wait(), 0);
                TEST_CHECK_EQUAL(stdout_stream.str(), "nobody\nnobody\n");
            }
        }
    } test_setuid;

    struct GrabFDTest : TestCase
    {
        GrabFDTest() : TestCase("grab fd") { }

        void run()
        {
            std::stringstream fd_stream;
            Process echo_process(ProcessCommand({"sh", "-c", "echo monkey 1>&$MAGIC_FD"}));
            echo_process.capture_output_to_fd(fd_stream, -1, "MAGIC_FD");

            TEST_CHECK_EQUAL(echo_process.run().wait(), 0);
            TEST_CHECK_EQUAL(fd_stream.str(), "monkey\n");
        }
    } test_grab_fd;

    struct GrabFDFixedTest : TestCase
    {
        GrabFDFixedTest() : TestCase("grab fd fixed") { }

        void run()
        {
            std::stringstream fd_stream;
            Process echo_process(ProcessCommand({"sh", "-c", "echo monkey 1>&5"}));
            echo_process.capture_output_to_fd(fd_stream, 5, "");

            TEST_CHECK_EQUAL(echo_process.run().wait(), 0);
            TEST_CHECK_EQUAL(fd_stream.str(), "monkey\n");
        }
    } test_grab_fd_fixed;

    struct StdinFDTest : TestCase
    {
        StdinFDTest() : TestCase("stdin fd") { }

        void run()
        {
            std::unique_ptr<Pipe> input_pipe(new Pipe(true));

            std::stringstream stdout_stream;
            Process cat_process(ProcessCommand({"rev"}));
            cat_process.capture_stdout(stdout_stream);
            cat_process.set_stdin_fd(input_pipe->read_fd());

            RunningProcessHandle handle(cat_process.run());

            {
                {
                    SafeOFStream s(input_pipe->write_fd());
                    s << "backwards" << std::endl;
                }
                TEST_CHECK(0 == ::close(input_pipe->write_fd()));
                input_pipe->clear_write_fd();
            }

            TEST_CHECK_EQUAL(handle.wait(), 0);
            TEST_CHECK_EQUAL(stdout_stream.str(), "sdrawkcab\n");
        }
    } test_stdin_fd;

    struct PipeCommandTest : TestCase
    {
        PipeCommandTest() : TestCase("pipe command") { }

        void run()
        {
            {
                Process one_two_process(ProcessCommand({ "bash", "process_TEST_dir/pipe_test.bash", "ONE", "TWO" }));
                one_two_process.pipe_command_handler("PALUDIS_PIPE_COMMAND", &response_handler);
                TEST_CHECK_EQUAL(one_two_process.run().wait(), 12);
            }

            {
                Process three_four_process(ProcessCommand({ "bash", "process_TEST_dir/pipe_test.bash", "THREE", "FOUR" }));
                three_four_process.pipe_command_handler("PALUDIS_PIPE_COMMAND", &response_handler);
                TEST_CHECK_EQUAL(three_four_process.run().wait(), 34);
            }
        }
    } test_pipe_command;

    struct CapturedPipeCommandTest : TestCase
    {
        CapturedPipeCommandTest() : TestCase("captured pipe command") { }

        void run()
        {
            std::stringstream stdout_stream;
            Process one_two_three_process(ProcessCommand({ "bash", "process_TEST_dir/captured_pipe_test.bash", "ONE", "TWO", "THREE" }));
            one_two_three_process.capture_stdout(stdout_stream);
            one_two_three_process.pipe_command_handler("PALUDIS_PIPE_COMMAND", &response_handler);
            TEST_CHECK_EQUAL(one_two_three_process.run().wait(), 13);

            std::string line;
            TEST_CHECK(std::getline(stdout_stream, line));
            TEST_CHECK_EQUAL(line, "2");
            TEST_CHECK(! std::getline(stdout_stream, line));
        }
    } test_captured_pipe_command;

    struct PrefixStdoutTest : TestCase
    {
        PrefixStdoutTest() : TestCase("prefix stdout") { }

        void run()
        {
            std::stringstream stdout_stream;
            Process echo_process(ProcessCommand({ "sh", "-c", "echo monkey ; echo in ; echo space"}));
            echo_process.capture_stdout(stdout_stream);
            echo_process.prefix_stdout("prefix> ");

            TEST_CHECK_EQUAL(echo_process.run().wait(), 0);
            TEST_CHECK_EQUAL(stdout_stream.str(), "prefix> monkey\nprefix> in\nprefix> space\n");
        }
    } test_prefix_stdout;

    struct PrefixStderrTest : TestCase
    {
        PrefixStderrTest() : TestCase("prefix stderr") { }

        void run()
        {
            std::stringstream stderr_stream;
            Process echo_process(ProcessCommand({ "sh", "-c", "echo monkey 1>&2 ; echo in 1>&2 ; echo space 1>&2"}));
            echo_process.capture_stderr(stderr_stream);
            echo_process.prefix_stderr("prefix> ");

            TEST_CHECK_EQUAL(echo_process.run().wait(), 0);
            TEST_CHECK_EQUAL(stderr_stream.str(), "prefix> monkey\nprefix> in\nprefix> space\n");
        }
    } test_prefix_stderr;
}

