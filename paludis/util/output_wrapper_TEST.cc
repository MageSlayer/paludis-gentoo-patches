/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh
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

#include <test/test_framework.hh>
#include <test/test_runner.hh>

#include <paludis/util/system.hh>
#include <paludis/util/join.hh>

#include <fstream>
#include <iterator>
#include <set>

using namespace test;
using namespace paludis;

namespace test_cases
{
    struct StdoutPrefixTest : TestCase
    {
        StdoutPrefixTest() : TestCase("stdout prefix") { }

        void run()
        {
            std::stringstream p;
            TEST_CHECK_EQUAL(run_command(Command("./outputwrapper --stdout-prefix 'o p ' -- "
                            "bash output_wrapper_TEST_dir/stdout_prefix.bash")
                        .with_captured_stdout_stream(&p)), 0);
            std::string s((std::istreambuf_iterator<char>(p)), std::istreambuf_iterator<char>());
            TEST_CHECK_EQUAL(s, "o p one\no p two\no p three\n");
        }
    } test_stdout_prefix;

    struct StderrPrefixTest : TestCase
    {
        StderrPrefixTest() : TestCase("stderr prefix") { }

        void run()
        {
            std::stringstream p;
            TEST_CHECK_EQUAL(0, run_command(Command("./outputwrapper --stderr-prefix 'e p ' -- "
                            "bash output_wrapper_TEST_dir/stderr_prefix.bash 2>&1")
                        .with_captured_stdout_stream(&p)));
            std::string s((std::istreambuf_iterator<char>(p)), std::istreambuf_iterator<char>());
            TEST_CHECK_EQUAL(s, "e p one\ne p two\ne p three\n");
        }
    } test_stderr_prefix;

    struct MixedPrefixTest : TestCase
    {
        MixedPrefixTest() : TestCase("mixed prefix") { }

        void run()
        {
            std::stringstream p;
            TEST_CHECK_EQUAL(run_command(Command("./outputwrapper --stdout-prefix 'o p ' --stderr-prefix 'e p ' -- "
                            "bash output_wrapper_TEST_dir/mixed_prefix.bash 2>&1")
                        .with_captured_stdout_stream(&p)), 0);
            std::multiset<std::string> lines;
            std::string line;
            while (std::getline(p, line))
                lines.insert(line);

            TestMessageSuffix s("lines=(" + join(lines.begin(), lines.end(), ",") + ")");
            TEST_CHECK_EQUAL(lines.size(), static_cast<std::size_t>(3));
            TEST_CHECK(lines.count("e p one"));
            TEST_CHECK(lines.count("e p three"));
            TEST_CHECK(lines.count("o p two"));
        }
    } test_mixed_prefix;

    struct LongLinesTest : TestCase
    {
        LongLinesTest() : TestCase("long lines") { }

        void run()
        {
            std::stringstream p;
            TEST_CHECK_EQUAL(run_command(Command("bash output_wrapper_TEST_dir/long_lines.bash")
                        .with_captured_stdout_stream(&p)), 0);
            std::multiset<std::string> lines;
            std::string line;
            while (std::getline(p, line))
                lines.insert(line);

            TestMessageSuffix s("lines=(" + join(lines.begin(), lines.end(), ",") + ")");
            TEST_CHECK_EQUAL(lines.size(), static_cast<std::size_t>(2));
            TEST_CHECK(lines.count("e p " + std::string(10000, 'e')));
            TEST_CHECK(lines.count("o p " + std::string(10000, 'o')));
        }
    } test_long_lines;

    struct NoTrailingNewlineTest : TestCase
    {
        NoTrailingNewlineTest() : TestCase("no trailing newline") { }

        void run()
        {
            std::stringstream p;
            TEST_CHECK_EQUAL(0, run_command(Command("bash output_wrapper_TEST_dir/no_trailing_newlines.bash 2>&1")
                        .with_captured_stdout_stream(&p)));
            std::multiset<std::string> lines;
            std::string line;
            while (std::getline(p, line))
                lines.insert(line);

            TestMessageSuffix s("lines=(" + join(lines.begin(), lines.end(), ",") + ")");
            TEST_CHECK_EQUAL(lines.size(), static_cast<std::size_t>(1));
            TEST_CHECK(lines.count("o p monkeye p pants"));
        }
    } test_no_trailing_newline;

    struct ExitStatusTest : TestCase
    {
        ExitStatusTest() : TestCase("exit status") { }

        void run()
        {
            std::stringstream p;
            TEST_CHECK_EQUAL(5, run_command(Command("./outputwrapper --stdout-prefix 'o p ' --stderr-prefix 'e p ' -- "
                            "bash output_wrapper_TEST_dir/exit_status.bash 2>&1")
                        .with_captured_stdout_stream(&p)));
            std::multiset<std::string> lines;
            std::string line;
            while (std::getline(p, line))
                lines.insert(line);

            TestMessageSuffix s("lines=(" + join(lines.begin(), lines.end(), ",") + ")");
            TEST_CHECK_EQUAL(lines.size(), static_cast<std::size_t>(2));
            TEST_CHECK(lines.count("o p lorem ipsum dolor"));
            TEST_CHECK(lines.count("e p sit amet"));
        }
    } test_exit_status;

    struct NoWrapBlanksTest : TestCase
    {
        NoWrapBlanksTest() : TestCase("no wrap blanks") { }

        void run()
        {
            std::stringstream p;
            TEST_CHECK_EQUAL(0, run_command(Command("./outputwrapper --stdout-prefix 'o p ' --stderr-prefix 'e p ' -- "
                            "bash output_wrapper_TEST_dir/no_wrap_blanks.bash 2>&1")
                        .with_captured_stdout_stream(&p)));
            std::multiset<std::string> lines;
            std::string line;
            while (std::getline(p, line))
                lines.insert(line);

            TestMessageSuffix s("lines=(" + join(lines.begin(), lines.end(), ",") + ")");
            TEST_CHECK_EQUAL(lines.size(), static_cast<std::size_t>(7));
            TEST_CHECK(lines.count("e p one"));
            TEST_CHECK(lines.count("e p three"));
            TEST_CHECK(lines.count("o p two"));
            TEST_CHECK_EQUAL(lines.count(""), static_cast<std::size_t>(4));
        }
    } test_no_wrap_blanks;

    struct WrapBlanksTest : TestCase
    {
        WrapBlanksTest() : TestCase("wrap blanks") { }

        void run()
        {
            std::stringstream p;
            TEST_CHECK_EQUAL(0, run_command(Command("./outputwrapper --stdout-prefix 'o p ' --stderr-prefix 'e p ' --wrap-blanks -- "
                            "bash output_wrapper_TEST_dir/wrap_blanks.bash 2>&1")
                        .with_captured_stdout_stream(&p)));
            std::multiset<std::string> lines;
            std::string line;
            while (std::getline(p, line))
                lines.insert(line);

            TestMessageSuffix s("lines=(" + join(lines.begin(), lines.end(), ",") + ")");
            TEST_CHECK_EQUAL(lines.size(), static_cast<std::size_t>(7));
            TEST_CHECK(lines.count("e p one"));
            TEST_CHECK(lines.count("e p three"));
            TEST_CHECK(lines.count("o p two"));
            TEST_CHECK_EQUAL(lines.count("e p "), static_cast<std::size_t>(2));
            TEST_CHECK_EQUAL(lines.count("o p "), static_cast<std::size_t>(2));
        }
    } test_wrap_blanks;

    struct DiscardBlankOutputTest : TestCase
    {
        DiscardBlankOutputTest() : TestCase("discard blank output") { }

        void run()
        {
            std::stringstream p;
            TEST_CHECK_EQUAL(0, run_command(Command("./outputwrapper --stdout-prefix 'o p ' --stderr-prefix 'e p ' --discard-blank-output -- "
                            "bash output_wrapper_TEST_dir/discard_blank_output.bash 2>&1")
                        .with_captured_stdout_stream(&p)));
            std::multiset<std::string> lines;
            std::string line;
            while (std::getline(p, line))
                lines.insert(line);

            TestMessageSuffix s("lines=(" + join(lines.begin(), lines.end(), ",") + ")");
            TEST_CHECK_EQUAL(lines.size(), static_cast<std::size_t>(0));
        }
    } test_discard_blank_output;

    struct DiscardBlankOutputNotBlankTest : TestCase
    {
        DiscardBlankOutputNotBlankTest() : TestCase("discard blank output not blank") { }

        void run()
        {
            std::stringstream p;
            TEST_CHECK_EQUAL(0, run_command(Command("./outputwrapper --stdout-prefix 'o p ' --stderr-prefix 'e p ' --discard-blank-output -- "
                            "bash output_wrapper_TEST_dir/discard_blank_output_not_blank.bash 2>&1")
                        .with_captured_stdout_stream(&p)));
            std::multiset<std::string> lines;
            std::string line;
            while (std::getline(p, line))
                lines.insert(line);

            TestMessageSuffix s("lines=(" + join(lines.begin(), lines.end(), ",") + ")");
            TEST_CHECK_EQUAL(lines.size(), static_cast<std::size_t>(4));
            TEST_CHECK(lines.count("o p monkey"));
            TEST_CHECK_EQUAL(lines.count(""), static_cast<std::size_t>(3));
        }
    } test_discard_blank_output_not_blank;

    struct DiscardBlankWrapBlankOutputTest : TestCase
    {
        DiscardBlankWrapBlankOutputTest() : TestCase("discard wrap blank output") { }

        void run()
        {
            std::stringstream p;
            TEST_CHECK_EQUAL(0, run_command(Command(
                            "./outputwrapper --stdout-prefix 'o p ' --stderr-prefix 'e p ' --discard-blank-output --wrap-blanks -- "
                            "bash output_wrapper_TEST_dir/discard_wrap_blank_output.bash 2>&1")
                        .with_captured_stdout_stream(&p)));
            std::multiset<std::string> lines;
            std::string line;
            while (std::getline(p, line))
                lines.insert(line);

            TestMessageSuffix s("lines=(" + join(lines.begin(), lines.end(), ",") + ")");
            TEST_CHECK_EQUAL(lines.size(), static_cast<std::size_t>(0));
        }
    } test_discard_wrap_blank_output;

    struct DiscardBlankWrapBlankOutputNotBlankTest : TestCase
    {
        DiscardBlankWrapBlankOutputNotBlankTest() : TestCase("discard wrap blank output not blank") { }

        void run()
        {
            std::stringstream p;
            TEST_CHECK_EQUAL(0, run_command(
                        Command("./outputwrapper --stdout-prefix 'o p ' --stderr-prefix 'e p ' --discard-blank-output --wrap-blanks -- "
                            "bash output_wrapper_TEST_dir/discard_wrap_blank_output_not_blank.bash 2>&1")
                        .with_captured_stdout_stream(&p)));
            std::multiset<std::string> lines;
            std::string line;
            while (std::getline(p, line))
                lines.insert(line);

            TestMessageSuffix s("lines=(" + join(lines.begin(), lines.end(), ",") + ")");
            TEST_CHECK_EQUAL(lines.size(), static_cast<std::size_t>(4));
            TEST_CHECK(lines.count("o p monkey"));
            TEST_CHECK_EQUAL(lines.count("o p "), static_cast<std::size_t>(3));
        }
    } test_discard_wrap_blank_output_not_blank;

    struct CarriageReturnTest : TestCase
    {
        CarriageReturnTest() : TestCase("carriage return") { }

        void run()
        {
            std::stringstream p;
            TEST_CHECK_EQUAL(0, run_command(Command("./outputwrapper --stdout-prefix 'o p ' --stderr-prefix 'e p ' -- "
                            "bash output_wrapper_TEST_dir/carriage_return.bash 2>&1")
                        .with_captured_stdout_stream(&p)));
            std::multiset<std::string> lines;
            std::string line;
            while (std::getline(p, line))
            {
                std::string::size_type c(line.length());
                while (std::string::npos != (c = line.rfind('\r', c)))
                    line.replace(c, 1, "\\r");
                lines.insert(line);
            }

            TestMessageSuffix s("lines=(" + join(lines.begin(), lines.end(), ",") + ")");
            TEST_CHECK_EQUAL(lines.size(), static_cast<std::size_t>(10));
            TEST_CHECK(lines.count("o p foo\\ro p bar"));
            TEST_CHECK(lines.count("o p foo\\r"));
            TEST_CHECK(lines.count("o p foo\\r\\ro p bar"));
            TEST_CHECK(lines.count("\\ro p foo"));
            TEST_CHECK(lines.count("e p foo\\re p bar"));
            TEST_CHECK(lines.count("e p foo\\r"));
            TEST_CHECK(lines.count("e p foo\\r\\re p bar"));
            TEST_CHECK(lines.count("\\re p foo"));
            TEST_CHECK_EQUAL(lines.count("\\r"), static_cast<std::size_t>(2));
        }
    } test_carriage_return;

    struct CarriageReturnWrapBlankTest : TestCase
    {
        CarriageReturnWrapBlankTest() : TestCase("carriage return wrap blank") { }

        void run()
        {
            std::stringstream p;
            TEST_CHECK_EQUAL(0, run_command(Command("./outputwrapper --stdout-prefix 'o p ' --stderr-prefix 'e p ' --wrap-blanks -- "
                        "bash output_wrapper_TEST_dir/carriage_return.bash 2>&1")
                        .with_captured_stdout_stream(&p)));
            std::multiset<std::string> lines;
            std::string line;
            while (std::getline(p, line))
            {
                std::string::size_type c(line.length());
                while (std::string::npos != (c = line.rfind('\r', c)))
                    line.replace(c, 1, "\\r");
                lines.insert(line);
            }

            TestMessageSuffix s("lines=(" + join(lines.begin(), lines.end(), ",") + ")");
            TEST_CHECK_EQUAL(lines.size(), static_cast<std::size_t>(10));
            TEST_CHECK(lines.count("o p foo\\ro p bar"));
            TEST_CHECK(lines.count("o p foo\\ro p "));
            TEST_CHECK(lines.count("o p foo\\ro p \\ro p bar"));
            TEST_CHECK(lines.count("o p \\ro p foo"));
            TEST_CHECK(lines.count("o p \\ro p "));
            TEST_CHECK(lines.count("e p foo\\re p bar"));
            TEST_CHECK(lines.count("e p foo\\re p "));
            TEST_CHECK(lines.count("e p foo\\re p \\re p bar"));
            TEST_CHECK(lines.count("e p \\re p foo"));
            TEST_CHECK(lines.count("e p \\re p "));
        }
    } test_carriage_return_wrap_blank;

    struct CarriageReturnDiscardBlankOutputBlankTest : TestCase
    {
        CarriageReturnDiscardBlankOutputBlankTest() : TestCase("carriage return discard blank output blank") { }

        void run()
        {
            std::stringstream p;
            TEST_CHECK_EQUAL(0, run_command(Command("./outputwrapper --stdout-prefix 'o p ' --stderr-prefix 'e p ' --discard-blank-output -- "
                            "bash output_wrapper_TEST_dir/carriage_return_blank.bash 2>&1")
                        .with_captured_stdout_stream(&p)));
            std::multiset<std::string> lines;
            std::string line;
            while (std::getline(p, line))
            {
                std::string::size_type c(line.length());
                while (std::string::npos != (c = line.rfind('\r', c)))
                    line.replace(c, 1, "\\r");
                lines.insert(line);
            }

            TestMessageSuffix s("lines=(" + join(lines.begin(), lines.end(), ",") + ")");
            TEST_CHECK_EQUAL(lines.size(), static_cast<std::size_t>(0));
        }
    } test_carriage_return_discard_blank_output_blank;

    struct CarriageReturnDiscardBlankWrapBlankOutputBlankTest : TestCase
    {
        CarriageReturnDiscardBlankWrapBlankOutputBlankTest() : TestCase("carriage return discard blank wrap blank output blank") { }

        void run()
        {
            std::stringstream p;
            TEST_CHECK_EQUAL(0, run_command(
                        Command("./outputwrapper --stdout-prefix 'o p ' --stderr-prefix 'e p ' --discard-blank-output --wrap-blanks -- "
                            "bash output_wrapper_TEST_dir/carriage_return_blank.bash 2>&1")
                        .with_captured_stdout_stream(&p)));
            std::multiset<std::string> lines;
            std::string line;
            while (std::getline(p, line))
            {
                std::string::size_type c(line.length());
                while (std::string::npos != (c = line.rfind('\r', c)))
                    line.replace(c, 1, "\\r");
                lines.insert(line);
            }

            TestMessageSuffix s("lines=(" + join(lines.begin(), lines.end(), ",") + ")");
            TEST_CHECK_EQUAL(lines.size(), static_cast<std::size_t>(0));
        }
    } test_carriage_return_discard_blank_wrap_blank_output_blank;

    struct CarriageReturnDiscardBlankOutputNotBlankTest : TestCase
    {
        CarriageReturnDiscardBlankOutputNotBlankTest() : TestCase("carriage return discard blank output not blank") { }

        void run()
        {
            std::stringstream p;
            TEST_CHECK_EQUAL(0, run_command(Command("./outputwrapper --stdout-prefix 'o p ' --stderr-prefix 'e p ' --discard-blank-output -- "
                            "bash output_wrapper_TEST_dir/carriage_return_nonblank.bash 2>&1")
                        .with_captured_stdout_stream(&p)));
            std::multiset<std::string> lines;
            std::string line;
            while (std::getline(p, line))
            {
                std::string::size_type c(line.length());
                while (std::string::npos != (c = line.rfind('\r', c)))
                    line.replace(c, 1, "\\r");
                lines.insert(line);
            }

            TestMessageSuffix s("lines=(" + join(lines.begin(), lines.end(), ",") + ")");
            TEST_CHECK_EQUAL(lines.size(), static_cast<std::size_t>(6));
            TEST_CHECK_EQUAL(lines.count(""), static_cast<std::size_t>(4));
            TEST_CHECK(lines.count("o p hello"));
            TEST_CHECK(lines.count("e p hello"));
        }
    } test_carriage_return_discard_blank_output_not_blank;

    struct CarriageReturnDiscardBlankWrapBlankOutputNotBlankTest : TestCase
    {
        CarriageReturnDiscardBlankWrapBlankOutputNotBlankTest() : TestCase("carriage return discard blank wrap blank output not blank") { }

        void run()
        {
            std::stringstream p;
            TEST_CHECK_EQUAL(0, run_command(
                        Command("./outputwrapper --stdout-prefix 'o p ' --stderr-prefix 'e p ' --discard-blank-output --wrap-blanks -- "
                            "bash output_wrapper_TEST_dir/carriage_return_nonblank.bash 2>&1")
                        .with_captured_stdout_stream(&p)));
            std::multiset<std::string> lines;
            std::string line;
            while (std::getline(p, line))
            {
                std::string::size_type c(line.length());
                while (std::string::npos != (c = line.rfind('\r', c)))
                    line.replace(c, 1, "\\r");
                lines.insert(line);
            }

            TestMessageSuffix s("lines=(" + join(lines.begin(), lines.end(), ",") + ")");
            TEST_CHECK_EQUAL(lines.size(), static_cast<std::size_t>(6));
            TEST_CHECK_EQUAL(lines.count("o p "), static_cast<std::size_t>(2));
            TEST_CHECK_EQUAL(lines.count("e p "), static_cast<std::size_t>(2));
            TEST_CHECK(lines.count("o p hello"));
            TEST_CHECK(lines.count("e p hello"));
        }
    } test_carriage_return_discard_blank_wrap_blank_output_not_blank;
}

