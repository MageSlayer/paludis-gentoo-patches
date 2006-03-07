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

#include <paludis/config_file.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/stringify.hh>
#include <sstream>
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <vector>

using namespace test;
using namespace paludis;

/** \file
 * Test cases for config_file.hh .
 *
 * \ingroup Test
 * \ingroup ConfigFile
 */

#ifndef DOXYGEN
class TestFile : protected ConfigFile
{
    public:
        TestFile(std::istream * const stream) :
            ConfigFile(stream)
        {
            need_lines();
        }

        TestFile(const std::string & filename) :
            ConfigFile(filename)
        {
            need_lines();
        }

        mutable std::vector<std::string> lines;

    protected:
        void accept_line(const std::string & s) const
        {
            lines.push_back(s);
        }
};
#endif

namespace test_cases
{
    /**
     * \test Test ConfigFile.
     *
     * \ingroup Test
     */
    struct ConfigFileTest : TestCase
    {
        ConfigFileTest() : TestCase("config file") { }

        void run()
        {
            std::stringstream s;
            s << "one" << std::endl;
            s << "  two    \t  " << std::endl;
            s << "   \t  " << std::endl;
            s << "" << std::endl;
            s << "three" << std::endl;
            s << "# blah" << std::endl;
            s << "  # blah" << std::endl;
            s << "#" << std::endl;
            s << "  #  \t  " << std::endl;
            s << "four  four" << std::endl;
            TestFile f(&s);
            TEST_CHECK_EQUAL(f.lines.size(), 4);
            TEST_CHECK_EQUAL(f.lines.at(0), "one");
            TEST_CHECK_EQUAL(f.lines.at(1), "two");
            TEST_CHECK_EQUAL(f.lines.at(2), "three");
            TEST_CHECK_EQUAL(f.lines.at(3), "four  four");
        }
    } test_config_file;

    /**
     * \test Test ConfigFile with file opening.
     *
     * \ingroup Test
     */
    struct ConfigFileOpenFileTest : TestCase
    {
        ConfigFileOpenFileTest() : TestCase("config file open file") { }

        void run()
        {
            FSEntry ff("config_file_TEST_dir/config_file");
            TEST_CHECK(ff.is_regular_file());
            TestFile f(ff);
            TEST_CHECK_EQUAL(f.lines.size(), 1);
            TEST_CHECK_EQUAL(f.lines.at(0), "I am a fish.");

            FSEntry ff2("config_file_TEST_dir/not_a_config_file");
            TEST_CHECK(! ff2.exists());
            TestFile * f2(0);
            TEST_CHECK_THROWS(f2 = new TestFile(ff2), ConfigFileError);

            FSEntry ff3("config_file_TEST_dir/unreadable_file");
            TEST_CHECK(ff3.is_regular_file());
            TestFile * f3(0);
            TEST_CHECK_THROWS(f3 = new TestFile(ff3), ConfigFileError);
        }
    } test_config_file_open_file;

    /**
     * \test Test LineConfigFile.
     *
     * \ingroup Test
     */
    struct LineConfigFileTest : TestCase
    {
        LineConfigFileTest() : TestCase("line config file") { }

        void run()
        {
            std::stringstream s;
            s << "one" << std::endl;
            s << "  two    \t  " << std::endl;
            s << "   \t  " << std::endl;
            s << "" << std::endl;
            s << "three" << std::endl;
            s << "# blah" << std::endl;
            s << "  # blah" << std::endl;
            s << "#" << std::endl;
            s << "  #  \t  " << std::endl;
            s << "four  four" << std::endl;
            LineConfigFile ff(&s);
            std::vector<std::string> f(ff.begin(), ff.end());

            TEST_CHECK_EQUAL(f.size(), 4);
            TEST_CHECK_EQUAL(f.at(0), "one");
            TEST_CHECK_EQUAL(f.at(1), "two");
            TEST_CHECK_EQUAL(f.at(2), "three");
            TEST_CHECK_EQUAL(f.at(3), "four  four");
        }
    } test_line_config_file;

    /**
     * \test Test KeyValueConfigFile basics.
     *
     * \ingroup Test
     */
    struct KeyValueConfigFileTest : TestCase
    {
        KeyValueConfigFileTest() : TestCase("key value config file") { }

        void run()
        {
            std::stringstream s;
            s << "one=first" << std::endl;
            s << "two = second" << std::endl;
            s << "three" << std::endl;
            s << "four = \"fourth\" " << std::endl;
            s << "five = ''" << std::endl;
            KeyValueConfigFile ff(&s);

            TEST_CHECK_EQUAL(ff.get("one"), "first");
            TEST_CHECK_EQUAL(ff.get("two"), "second");
            TEST_CHECK_EQUAL(ff.get("three"), "");
            TEST_CHECK_EQUAL(ff.get("four"), "fourth");
            TEST_CHECK_EQUAL(ff.get("five"), "");
            TEST_CHECK_EQUAL(ff.get("six"), "");
        }
    } test_key_value_config_file;

    /**
     * \test Test KeyValueConfigFile variables.
     *
     * \ingroup Test
     */
    struct KeyValueConfigFileVarsTest : TestCase
    {
        KeyValueConfigFileVarsTest() : TestCase("key value config file with vars") { }

        void run()
        {
            std::stringstream s;
            s << "x=foo" << std::endl;
            s << "y = \"${x}\\\\${y}\\$${z}\"" << std::endl;
            s << "z = $x$y$z" << std::endl;
            KeyValueConfigFile ff(&s);

            TEST_CHECK_EQUAL(ff.get("x"), "foo");
            TEST_CHECK_EQUAL(ff.get("y"), "foo\\$");
            TEST_CHECK_EQUAL(ff.get("z"), "foofoo\\$");
        }
    } test_key_value_config_file_vars;

    /**
     * \test Test KeyValueConfigFile errors.
     *
     * \ingroup Test
     */
    struct KeyValueConfigFileErrorsTest : TestCase
    {
        KeyValueConfigFileErrorsTest() : TestCase("key value config file with errors") { }

        void run()
        {
            std::stringstream s1;
            s1 << "x='" << std::endl;
            TEST_CHECK_THROWS(KeyValueConfigFile ff(&s1), ConfigurationError);

            std::stringstream s2;
            s2 << "x='moo\"" << std::endl;
            TEST_CHECK_THROWS(KeyValueConfigFile ff(&s2), ConfigurationError);

            std::stringstream s3;
            s3 << "x=${foo" << std::endl;
            TEST_CHECK_THROWS(KeyValueConfigFile ff(&s3), ConfigurationError);

            std::stringstream s4;
            s4 << "x=$~" << std::endl;
            TEST_CHECK_THROWS(KeyValueConfigFile ff(&s4), ConfigurationError);

            std::stringstream s5;
            s5 << "x=abc\\" << std::endl;
            TEST_CHECK_THROWS(KeyValueConfigFile ff(&s5), ConfigurationError);
        }
    } test_key_value_config_file_errors;
}

