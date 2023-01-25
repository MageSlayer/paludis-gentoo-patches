/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010, 2011 Ciaran McCreesh
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
#include <paludis/util/fs_path.hh>
#include <paludis/util/pipe.hh>
#include <paludis/util/safe_ofstream.hh>
#include <paludis/util/stringify.hh>

#include <sstream>
#include <sys/types.h>
#include <pwd.h>

#include <gtest/gtest.h>

using namespace paludis;

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

TEST(Process, True)
{
    Process true_process(ProcessCommand({"true"}));
    EXPECT_EQ(0, true_process.run().wait());
}

TEST(Process, False)
{
    Process false_process(ProcessCommand({"false"}));
    EXPECT_EQ(1, false_process.run().wait());
}

TEST(Process, NoWait)
{
    Process true_process(ProcessCommand({"true"}));
    EXPECT_THROW({ RunningProcessHandle handle(true_process.run()); }, ProcessError);
}

TEST(Process, TwoWait)
{
    Process true_process(ProcessCommand({"true"}));

    RunningProcessHandle handle(true_process.run());
    EXPECT_EQ(0, handle.wait());
    EXPECT_THROW(int PALUDIS_ATTRIBUTE((unused)) x(handle.wait()), ProcessError);
}

TEST(Process, GrabStdout)
{
    std::stringstream stdout_stream;
    Process echo_process(ProcessCommand({"echo", "monkey"}));
    echo_process.capture_stdout(stdout_stream);

    EXPECT_EQ(0, echo_process.run().wait());
    EXPECT_EQ("monkey\n", stdout_stream.str());
}

TEST(Process, GrabStdoutSingleCommand)
{
    std::stringstream stdout_stream;
    Process echo_process(ProcessCommand("echo giant space monkey"));
    echo_process.capture_stdout(stdout_stream);

    EXPECT_EQ(0, echo_process.run().wait());
    EXPECT_EQ("giant space monkey\n", stdout_stream.str());
}

TEST(Process, GrabStderr)
{
    std::stringstream stderr_stream;
    Process echo_process(ProcessCommand({"bash", "-c", "echo monkey 1>&2"}));
    echo_process.capture_stderr(stderr_stream);

    EXPECT_EQ(0, echo_process.run().wait());
    EXPECT_EQ("monkey\n", stderr_stream.str());
}

TEST(Process, GrabStdoutStderr)
{
    std::stringstream stdout_stream;
    std::stringstream stderr_stream;
    Process echo_process(ProcessCommand({"bash", "-c", "echo monkey 1>&2 ; echo chimp"}));
    echo_process.capture_stdout(stdout_stream);
    echo_process.capture_stderr(stderr_stream);

    EXPECT_EQ(0, echo_process.run().wait());

    EXPECT_EQ("chimp\n", stdout_stream.str());
    EXPECT_EQ("monkey\n", stderr_stream.str());
}

TEST(Process, GrabStdoutLong)
{
    std::stringstream stdout_stream;
    Process echo_process(ProcessCommand({"seq", "1", "100000"}));
    echo_process.capture_stdout(stdout_stream);

    EXPECT_EQ(0, echo_process.run().wait());

    std::string s;
    for (int x(1) ; x <= 100000 ; ++x)
    {
        ASSERT_TRUE(bool(std::getline(stdout_stream, s)));
        ASSERT_EQ(stringify(x), s);
    }

    ASSERT_TRUE(! std::getline(stdout_stream, s));
}

TEST(Process, Setenv)
{
    std::stringstream stdout_stream;
    Process printenv_process(ProcessCommand({"printenv", "monkey"}));
    printenv_process.capture_stdout(stdout_stream);
    printenv_process.setenv("monkey", "in space");

    EXPECT_EQ(0, printenv_process.run().wait());
    EXPECT_EQ("in space\n", stdout_stream.str());
}

TEST(Process, Chdir)
{
    std::stringstream stdout_stream;
    Process pwd_process(ProcessCommand({"pwd"}));
    pwd_process.capture_stdout(stdout_stream);
    pwd_process.chdir(FSPath("/"));

    EXPECT_EQ(0, pwd_process.run().wait());
    EXPECT_EQ("/\n", stdout_stream.str());
}

TEST(Process, NoPty)
{
    std::stringstream stdout_stream;
    std::stringstream stderr_stream;
    Process test_t_process(ProcessCommand({"test", "-t", "1", "-o", "-t", "2"}));
    test_t_process.capture_stdout(stdout_stream);
    test_t_process.capture_stderr(stderr_stream);
    EXPECT_EQ(1, test_t_process.run().wait());
}

TEST(Process, Pty)
{
    std::stringstream stdout_stream;
    std::stringstream stderr_stream;
    Process test_t_process(ProcessCommand({"test", "-t", "1", "-a", "-t", "2"}));
    test_t_process.capture_stdout(stdout_stream);
    test_t_process.capture_stderr(stderr_stream);
    test_t_process.use_ptys();

    EXPECT_EQ(0, test_t_process.run().wait());
}

TEST(Process, Setuid)
{
    if (0 != getuid())
        return;

    std::stringstream stdout_stream;
    Process whoami_process(ProcessCommand({"bash", "-c", "whoami ; groups"}));
    whoami_process.capture_stdout(stdout_stream);

    struct passwd * nobody(getpwnam("nobody"));
    if (nobody)
    {
        whoami_process.setuid_setgid(nobody->pw_uid, nobody->pw_gid);
        EXPECT_EQ(0, whoami_process.run().wait());
        EXPECT_EQ("nobody\nnobody\n", stdout_stream.str());
    }
}

TEST(Process, GrabFD)
{
    std::stringstream fd_stream;
    Process echo_process(ProcessCommand({"bash", "-c", "echo monkey 1>&$MAGIC_FD"}));
    echo_process.capture_output_to_fd(fd_stream, -1, "MAGIC_FD");

    EXPECT_EQ(0, echo_process.run().wait());
    EXPECT_EQ("monkey\n", fd_stream.str());
}

TEST(Process, GrabFDFixed)
{
    std::stringstream fd_stream;
    Process echo_process(ProcessCommand({"bash", "-c", "echo monkey 1>&5"}));
    echo_process.capture_output_to_fd(fd_stream, 5, "");

    EXPECT_EQ(0, echo_process.run().wait());
    EXPECT_EQ("monkey\n", fd_stream.str());
}

TEST(Process, StdinFD)
{
    std::unique_ptr<Pipe> input_pipe(new Pipe(true));

    std::stringstream stdout_stream;
    Process cat_process(ProcessCommand({"rev"}));
    cat_process.capture_stdout(stdout_stream);
    cat_process.set_stdin_fd(input_pipe->read_fd());

    RunningProcessHandle handle(cat_process.run());

    {
        {
            SafeOFStream s(input_pipe->write_fd(), true);
            s << "backwards" << std::endl;
        }

        ASSERT_TRUE(0 == ::close(input_pipe->write_fd()));
        input_pipe->clear_write_fd();
    }

    EXPECT_EQ(0, handle.wait());
    EXPECT_EQ("sdrawkcab\n", stdout_stream.str());
}

TEST(Process, PipeCommand)
{
    {
        Process one_two_process(ProcessCommand({ "bash", "process_TEST_dir/pipe_test.bash", "ONE", "TWO" }));
        one_two_process.pipe_command_handler("PALUDIS_PIPE_COMMAND", &response_handler);
        EXPECT_EQ(12, one_two_process.run().wait());
    }

    {
        Process three_four_process(ProcessCommand({ "bash", "process_TEST_dir/pipe_test.bash", "THREE", "FOUR" }));
        three_four_process.pipe_command_handler("PALUDIS_PIPE_COMMAND", &response_handler);
        EXPECT_EQ(34, three_four_process.run().wait());
    }
}

TEST(Process, CapturedPipeCommand)
{
    std::stringstream stdout_stream;
    Process one_two_three_process(ProcessCommand({ "bash", "process_TEST_dir/captured_pipe_test.bash", "ONE", "TWO", "THREE" }));
    one_two_three_process.capture_stdout(stdout_stream);
    one_two_three_process.pipe_command_handler("PALUDIS_PIPE_COMMAND", &response_handler);
    EXPECT_EQ(13, one_two_three_process.run().wait());

    std::string line;
    ASSERT_TRUE(bool(std::getline(stdout_stream, line)));
    EXPECT_EQ("2", line);
    ASSERT_TRUE(! std::getline(stdout_stream, line));
}

TEST(Process, PrefixStdout)
{
    std::stringstream stdout_stream;
    Process echo_process(ProcessCommand({ "bash", "-c", "echo monkey ; echo in ; echo space"}));
    echo_process.capture_stdout(stdout_stream);
    echo_process.prefix_stdout("prefix> ");

    EXPECT_EQ(0, echo_process.run().wait());
    EXPECT_EQ("prefix> monkey\nprefix> in\nprefix> space\n", stdout_stream.str());
}

TEST(Process, PrefixStderr)
{
    std::stringstream stderr_stream;
    Process echo_process(ProcessCommand({ "bash", "-c", "echo monkey 1>&2 ; echo in 1>&2 ; echo space 1>&2"}));
    echo_process.capture_stderr(stderr_stream);
    echo_process.prefix_stderr("prefix> ");

    EXPECT_EQ(0, echo_process.run().wait());
    EXPECT_EQ("prefix> monkey\nprefix> in\nprefix> space\n", stderr_stream.str());
}

TEST(Process, Clearenv)
{
    ::setenv("BANANAS", "IN PYJAMAS", 1);
    std::stringstream stdout_stream;
    Process printenv_process(ProcessCommand({"printenv", "BANANAS"}));
    printenv_process.capture_stdout(stdout_stream);
    printenv_process.clearenv();

    EXPECT_EQ(1, printenv_process.run().wait());
    EXPECT_EQ("", stdout_stream.str());
}

TEST(Process, ClearenvPres)
{
    ::setenv("PALUDIS_BANANAS", "PALUDIS_IN PYJAMAS", 1);
    std::stringstream stdout_stream;
    Process printenv_process(ProcessCommand({"printenv", "PALUDIS_BANANAS"}));
    printenv_process.capture_stdout(stdout_stream);
    printenv_process.clearenv();

    EXPECT_EQ(0, printenv_process.run().wait());
    EXPECT_EQ("PALUDIS_IN PYJAMAS\n", stdout_stream.str());
}

TEST(Process, SendFD)
{
    std::stringstream stdout_stream;
    std::stringstream in_stream;
    in_stream << "monkey" << std::endl;

    Process cat_process(ProcessCommand({"bash", "-c", "cat <&$MAGIC_FD"}));
    cat_process.send_input_to_fd(in_stream, -1, "MAGIC_FD");
    cat_process.capture_stdout(stdout_stream);

    EXPECT_EQ(0, cat_process.run().wait());
    EXPECT_EQ("monkey\n", stdout_stream.str());
}

TEST(Process, SendFDFixed)
{
    std::stringstream stdout_stream;
    std::stringstream in_stream;
    in_stream << "monkey" << std::endl;

    Process cat_process(ProcessCommand({"bash", "-c", "cat <&5"}));
    cat_process.send_input_to_fd(in_stream, 5, "");
    cat_process.capture_stdout(stdout_stream);

    EXPECT_EQ(0, cat_process.run().wait());
    EXPECT_EQ("monkey\n", stdout_stream.str());
}

TEST(Process, ExecError)
{
    Process process(ProcessCommand({"paludis-nonexisting-command"}));
    EXPECT_THROW({ auto ph = process.run(); }, ProcessError);
}

