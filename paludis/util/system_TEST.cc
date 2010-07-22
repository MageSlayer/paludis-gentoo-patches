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
#include <paludis/util/fs_entry.hh>
#include <paludis/util/log.hh>
#include <paludis/util/thread_pool.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <functional>

#include <cctype>
#include <sched.h>

using namespace test;
using namespace paludis;

namespace
{
    void repeatedly_log(bool & b)
    {
        while (! b)
            sched_yield();

        for (int i(0) ; i < 1000 ; ++i)
            Log::get_instance()->message("test.system", ll_debug, lc_context) << "logging stuff";
    }

    void repeatedly_run_command(bool & b)
    {
        while (! b)
            sched_yield();

        for (int i(0) ; i < 100 ; ++i)
            if (0 != run_command("/usr/bin/env true"))
                throw InternalError(PALUDIS_HERE, "true isn't");
    }

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

    struct RunCommandTest : TestCase
    {
        RunCommandTest() : TestCase("run_command") { }

        void run()
        {
            TEST_CHECK(0 == run_command("true"));
            TEST_CHECK(0 != run_command("false"));
            TEST_CHECK_EQUAL(77, run_command("exit 77"));
        }
    } test_run_command;

    struct RunCommandMutexTest : TestCase
    {
        RunCommandMutexTest() : TestCase("run_command mutex") { }

        void run()
        {
            ThreadPool pool;
            bool b(false);
            pool.create_thread(std::bind(&repeatedly_run_command, std::ref(b)));
            pool.create_thread(std::bind(&repeatedly_log, std::ref(b)));
            b = true;
        }
    } test_run_command_mutex;

    struct RunCommandInDirectoryTest : TestCase
    {
        RunCommandInDirectoryTest() : TestCase("run_command_in_directory") { }

        void run()
        {
            FSEntry dir("system_TEST_dir");
            TEST_CHECK(dir.is_directory());

            TEST_CHECK_EQUAL(run_command(Command("touch in_directory").with_chdir(dir)), 0);
            TEST_CHECK(FSEntry(dir / "in_directory").exists());
            TEST_CHECK_EQUAL(run_command(Command("rm in_directory").with_chdir(dir)), 0);
            TEST_CHECK(! FSEntry(dir / "in_directory").exists());
        }
    } test_run_command_in_directory;

    struct MakeEnvCommandTest : TestCase
    {
        MakeEnvCommandTest() : TestCase("make_env_command") { }

        void run()
        {
            TEST_CHECK(0 != run_command(("printenv PALUDUS_TEST_ENV_VAR")));
            TEST_CHECK(0 == run_command(("bash -c '[[ -z $PALUDUS_TEST_ENV_VAR ]]'")));
            TEST_CHECK(0 == run_command(Command("bash -c '[[ -z $PALUDUS_TEST_ENV_VAR ]]'")
                        .with_setenv("PALUDUS_TEST_ENV_VAR", "")));
            TEST_CHECK(0 == run_command(Command("bash -c '[[ -n $PALUDUS_TEST_ENV_VAR ]]'")
                        .with_setenv("PALUDUS_TEST_ENV_VAR", "foo")));
            TEST_CHECK(0 != run_command(Command("bash -c '[[ -z $PALUDUS_TEST_ENV_VAR ]]'")
                        .with_setenv("PALUDUS_TEST_ENV_VAR", "foo")));
            TEST_CHECK(0 != run_command(("bash -c '[[ -n $PALUDUS_TEST_ENV_VAR ]]'")));
            TEST_CHECK(0 != run_command(Command("bash -c '[[ -n $PALUDUS_TEST_ENV_VAR ]]'")
                        .with_setenv("PALUDUS_TEST_ENV_VAR", "")));
            TEST_CHECK(0 == run_command(Command("bash -c '[[ $PALUDUS_TEST_ENV_VAR == foo ]]'")
                        .with_setenv("PALUDUS_TEST_ENV_VAR", "foo")));
            TEST_CHECK(0 != run_command(("bash -c '[[ $PALUDUS_TEST_ENV_VAR == foo ]]'")));
            TEST_CHECK(0 != run_command(Command("bash -c '[[ $PALUDUS_TEST_ENV_VAR == foo ]]'")
                        .with_setenv("PALUDUS_TEST_ENV_VAR", "")));
            TEST_CHECK(0 != run_command(Command("bash -c '[[ $PALUDUS_TEST_ENV_VAR == foo ]]'")
                        .with_setenv("PALUDUS_TEST_ENV_VAR", "bar")));
        }
    } test_make_env_command;

    struct MakeEnvCommandQuoteTest : TestCase
    {
        MakeEnvCommandQuoteTest() : TestCase("make_env_command quotes") { }

        void run()
        {
            TEST_CHECK(0 == run_command(Command(
                            "bash -c '[[ x$PALUDUS_TEST_ENV_VAR == \"x....\" ]]'")
                        .with_setenv("PALUDUS_TEST_ENV_VAR", "....")));
            TEST_CHECK(0 == run_command(Command(
                            "bash -c '[[ x$PALUDUS_TEST_ENV_VAR == \"x..'\"'\"'..\" ]]'")
                        .with_setenv("PALUDUS_TEST_ENV_VAR", "..'..")));
        }
    } test_make_env_command_quotes;

    struct PipeCommandTest : TestCase
    {
        PipeCommandTest() : TestCase("pipe command") { }

        void run()
        {
            TEST_CHECK_EQUAL(run_command(Command("bash system_TEST_dir/pipe_test.bash ONE TWO")
                        .with_pipe_command_handler("PALUDIS_PIPE_COMMAND", &response_handler)), 12);
            TEST_CHECK_EQUAL(run_command(Command("bash system_TEST_dir/pipe_test.bash THREE FOUR")
                        .with_pipe_command_handler("PALUDIS_PIPE_COMMAND", &response_handler)), 34);
        }
    } test_pipe_command;

    struct CapturedTest : TestCase
    {
        CapturedTest() : TestCase("captured stdout") { }

        void run()
        {
            std::stringstream s;
            TEST_CHECK_EQUAL(run_command(Command("echo hi").with_captured_stdout_stream(&s)), 0);
            std::string line;
            TEST_CHECK(std::getline(s, line));
            TEST_CHECK_EQUAL(line, "hi");
            TEST_CHECK(! std::getline(s, line));
        }
    } test_captured;

    struct CapturedErrTest : TestCase
    {
        CapturedErrTest() : TestCase("captured stderr") { }

        void run()
        {
            std::stringstream s;
            TEST_CHECK_EQUAL(run_command(Command("echo hi 1>&2").with_captured_stderr_stream(&s)), 0);
            std::string line;
            TEST_CHECK(std::getline(s, line));
            TEST_CHECK_EQUAL(line, "hi");
            TEST_CHECK(! std::getline(s, line));
        }
    } test_captured_err;

    struct CapturedExtraOutTest : TestCase
    {
        CapturedExtraOutTest() : TestCase("captured extra output") { }

        void run()
        {
            std::stringstream s;
            TEST_CHECK_EQUAL(run_command(Command("echo hi 1>&$TEST_PIPE_FD").with_output_stream(&s, -1, "TEST_PIPE_FD")), 0);
            std::string line;
            TEST_CHECK(std::getline(s, line));
            TEST_CHECK_EQUAL(line, "hi");
            TEST_CHECK(! std::getline(s, line));
        }
    } test_captured_extra_out;

    struct CapturedNoExistTest : TestCase
    {
        CapturedNoExistTest() : TestCase("captured nonexistent command") { }

        void run()
        {
            std::stringstream s;
            TEST_CHECK(run_command(Command("thiscommanddoesnotexist 2>/dev/null").with_captured_stdout_stream(&s)) != 0);
            std::string line;
            TEST_CHECK(! std::getline(s, line));
        }
    } test_captured_no_exist;

    struct CapturedSilentFailTest : TestCase
    {
        CapturedSilentFailTest() : TestCase("captured silent fail") { }

        void run()
        {
            std::stringstream s;
            TEST_CHECK(run_command(Command("test -e /doesnotexist").with_captured_stdout_stream(&s)) != 0);
            std::string line;
            TEST_CHECK(! std::getline(s, line));
        }
    } test_captured_silent_fail;

    struct CapturedFailTest : TestCase
    {
        CapturedFailTest() : TestCase("captured fail") { }

        void run()
        {
            std::stringstream s;
            TEST_CHECK(run_command(Command("cat /doesnotexist 2>&1").with_captured_stdout_stream(&s)) != 0);
            std::string line;
            TEST_CHECK(std::getline(s, line));
            TEST_CHECK(! line.empty());
            TEST_CHECK(! std::getline(s, line));
        }
    } test_captured_fail;

    struct CapturedPipeCommandTest : TestCase
    {
        CapturedPipeCommandTest() : TestCase("captured pipe command") { }

        void run()
        {
            std::stringstream s;
            TEST_CHECK_EQUAL(run_command(Command("bash system_TEST_dir/captured_pipe_test.bash ONE TWO THREE")
                        .with_pipe_command_handler("PALUDIS_PIPE_COMMAND", &response_handler)
                        .with_captured_stdout_stream(&s)),
                    13);
            std::string line;
            TEST_CHECK(std::getline(s, line));
            TEST_CHECK_EQUAL(line, "2");
            TEST_CHECK(! std::getline(s, line));
        }
    } test_captured_pipe_command;

    struct StdinCommandTest : TestCase
    {
        StdinCommandTest() : TestCase("stdin") { }

        void run()
        {
            std::stringstream is, os;
            for (int n(0) ; n < 300 ; ++n)
                is << "I chased a bug around a tree\n";

            TEST_CHECK_EQUAL(run_command(Command("cat")
                        .with_input_stream(&is, 0, "")
                        .with_captured_stdout_stream(&os)
                        ),
                    0);

            std::string line;
            for (int n(0) ; n < 300 ; ++n)
            {
                std::getline(os, line);
                TEST_CHECK_EQUAL(line, "I chased a bug around a tree");
            }

            TEST_CHECK(! std::getline(os, line));
        }
    } test_stdin;

    struct InputCommandTest : TestCase
    {
        InputCommandTest() : TestCase("input") { }

        void run()
        {
            std::stringstream is, os;
            for (int n(0) ; n < 300 ; ++n)
                is << "The cat crept into the crypt\n";

            TEST_CHECK_EQUAL(run_command(Command("cat <&$THE_FD")
                        .with_input_stream(&is, -1, "THE_FD")
                        .with_captured_stdout_stream(&os)
                        ),
                    0);

            std::string line;
            for (int n(0) ; n < 300 ; ++n)
            {
                std::getline(os, line);
                TEST_CHECK_EQUAL(line, "The cat crept into the crypt");
            }

            TEST_CHECK(! std::getline(os, line));
        }
    } test_input;

    struct BecomeChildCommandTest : TestCase
    {
        BecomeChildCommandTest() : TestCase("become child") { }

        void run()
        {
            std::stringstream os;
            Command cmd("./system_TEST_become_child");
            cmd
                .with_captured_stdout_stream(&os)
                ;

            TEST_CHECK_EQUAL(123, run_command(cmd));
            TEST_CHECK_EQUAL(os.str(), "giant space monkey");
        }
    } test_become_child;
}

