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

#include <paludis/config_file.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/collection_concrete.hh>
#include <sstream>
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <vector>
#include <unistd.h>

using namespace test;
using namespace paludis;

/** \file
 * Test cases for config_file.hh .
 *
 * \ingroup grptestcases
 * \ingroup grpconfigfile
 */

namespace
{
    /**
     * A ConfigFile descendent for use in tests.
     *
     * \ingroup grptestcases
     */
    class TestFile : protected ConfigFile
    {
        public:
            /**
             * Constructor.
             */
            TestFile(std::istream * const stream) :
                ConfigFile(stream)
            {
                need_lines();
            }

            /**
             * Constructor.
             */
            TestFile(const std::string & filename) :
                ConfigFile(filename)
            {
                need_lines();
            }

            /**
             * Constructor.
             */
            TestFile(const FSEntry & filename) :
                ConfigFile(filename)
            {
                need_lines();
            }

            /**
             * Our lines.
             */
            mutable std::vector<std::string> lines;

        protected:
            void accept_line(const std::string & s) const
            {
                lines.push_back(s);
            }
    };
}

namespace test_cases
{
    /**
     * \test Test ConfigFile.
     *
     * \ingroup grptestcases
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
     * \ingroup grptestcases
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

            if (0 != geteuid())
            {
                FSEntry ff3("config_file_TEST_dir/unreadable_file");
                TEST_CHECK(ff3.is_regular_file());
                TestFile * f3(0);
                TEST_CHECK_THROWS(f3 = new TestFile(ff3), ConfigFileError);
            }
        }
    } test_config_file_open_file;

    /**
     * \test Test LineConfigFile.
     *
     * \ingroup grptestcases
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
     * \ingroup grptestcases
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
     * \test Test KeyValueConfigFile continuations.
     *
     * \ingroup grptestcases
     */
    struct KeyValueConfigFileContinuationsTest : TestCase
    {
        KeyValueConfigFileContinuationsTest() : TestCase("key value config file continuations") { }

        void run()
        {
            std::stringstream s;
            s << "one='first" << std::endl;
            s << " first " << std::endl;
            s << "first'" << std::endl;
            KeyValueConfigFile ff(&s);

            TEST_CHECK_EQUAL(ff.get("one"), "first first first");
        }
    } test_key_value_config_file_continuations;

    /**
     * \test Test KeyValueConfigFile variables.
     *
     * \ingroup grptestcases
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

            std::stringstream t;
            AssociativeCollection<std::string, std::string>::Pointer t_defs(
                    new AssociativeCollection<std::string, std::string>::Concrete);
            t_defs->insert("a", "moo");
            t_defs->insert("d", "bar");
            t_defs->insert("e", "baz");
            t << "a=foo" << std::endl;
            t << "b=$a" << std::endl;
            t << "c=$d" << std::endl;
            t << "d=$d" << std::endl;
            t << "f = " << std::endl;
            t << "g = foo \\" << std::endl;
            t << "    bar" << std::endl;
            KeyValueConfigFile fg(&t, t_defs);

            TEST_CHECK_EQUAL(fg.get("a"), "foo");
            TEST_CHECK_EQUAL(fg.get("b"), "foo");
            TEST_CHECK_EQUAL(fg.get("c"), "bar");
            TEST_CHECK_EQUAL(fg.get("d"), "bar");
            TEST_CHECK_EQUAL(fg.get("e"), "baz");
            TEST_CHECK_EQUAL(fg.get("f"), "");
            TEST_CHECK_EQUAL(fg.get("g"), "foo bar");
        }
    } test_key_value_config_file_vars;

    /**
     * \test Test KeyValueConfigFile errors.
     *
     * \ingroup grptestcases
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

            std::stringstream s6;
            s6 << "x=$" << std::endl;
            TEST_CHECK_THROWS(KeyValueConfigFile ff(&s6), ConfigurationError);

            std::stringstream s7;
            s7 << "x=blah \\" << std::endl;
            TEST_CHECK_THROWS(KeyValueConfigFile ff(&s7), ConfigurationError);

            std::stringstream s8;
            s8 << "x=blah \\" << std::endl << "# foo" << std::endl;
            TEST_CHECK_THROWS(KeyValueConfigFile ff(&s8), ConfigurationError);

            std::stringstream s9;
            s9 << "x='blah" << std::endl << "blah" << std::endl;
            TEST_CHECK_THROWS(KeyValueConfigFile ff(&s9), ConfigurationError);
        }
    } test_key_value_config_file_errors;
}

