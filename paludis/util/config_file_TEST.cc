/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008 Ciaran McCreesh
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

#include <paludis/util/config_file.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/system.hh>
#include <paludis/util/map.hh>
#include <paludis/util/options.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <sstream>
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <vector>
#include <cstdlib>
#include <iomanip>
#include <unistd.h>

using namespace test;
using namespace paludis;

/** \file
 * Test cases for config_file.hh .
 *
 * \ingroup grpconfigfile
 */

namespace test_cases
{
    /**
     * \test Test ConfigFile.
     *
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
            s << "five \\" << std::endl;
            s << "six" << std::endl;

            LineConfigFile f(s, LineConfigFileOptions());
            TEST_CHECK_EQUAL(std::distance(f.begin(), f.end()), 5);
            std::vector<std::string> lines;
            std::copy(f.begin(), f.end(), std::back_inserter(lines));
            TEST_CHECK_EQUAL(lines.at(0), "one");
            TEST_CHECK_EQUAL(lines.at(1), "two");
            TEST_CHECK_EQUAL(lines.at(2), "three");
            TEST_CHECK_EQUAL(lines.at(3), "four  four");
            TEST_CHECK_EQUAL(lines.at(4), "five six");

            s.clear();
            s.seekg(0, std::ios::beg);

            LineConfigFile g(s, LineConfigFileOptions() + lcfo_disallow_comments
                    + lcfo_preserve_whitespace + lcfo_no_skip_blank_lines);
            TEST_CHECK_EQUAL(std::distance(g.begin(), g.end()), 11);
            lines.clear();
            std::copy(g.begin(), g.end(), std::back_inserter(lines));
            TEST_CHECK_EQUAL(lines.at(0), "one");
            TEST_CHECK_EQUAL(lines.at(1), "  two    \t  ");
            TEST_CHECK_EQUAL(lines.at(2), "   \t  ");
            TEST_CHECK_EQUAL(lines.at(3), "");
            TEST_CHECK_EQUAL(lines.at(4), "three");
            TEST_CHECK_EQUAL(lines.at(5), "# blah");
            TEST_CHECK_EQUAL(lines.at(6), "  # blah");
            TEST_CHECK_EQUAL(lines.at(7), "#");
            TEST_CHECK_EQUAL(lines.at(8), "  #  \t  ");
            TEST_CHECK_EQUAL(lines.at(9), "four  four");
            TEST_CHECK_EQUAL(lines.at(10), "five six");
        }
    } test_config_file;

    /**
     * \test Test ConfigFile with file opening.
     *
     */
    struct ConfigFileOpenFileTest : TestCase
    {
        ConfigFileOpenFileTest() : TestCase("config file open file") { }

        void run()
        {
            FSEntry ff("config_file_TEST_dir/config_file");
            TEST_CHECK(ff.is_regular_file());
            LineConfigFile f(ff, LineConfigFileOptions());
            TEST_CHECK_EQUAL(std::distance(f.begin(), f.end()), 1);
            TEST_CHECK_EQUAL(*f.begin(), "I am a fish.");

            FSEntry ff2("config_file_TEST_dir/not_a_config_file");
            TEST_CHECK(! ff2.exists());
            LineConfigFile * f2(0);
            TEST_CHECK_THROWS(f2 = new LineConfigFile(ff2, LineConfigFileOptions()), ConfigFileError);

            if (0 != geteuid())
            {
                FSEntry ff3("config_file_TEST_dir/unreadable_file");
                TEST_CHECK(ff3.is_regular_file());
                LineConfigFile * f3(0);
                TEST_CHECK_THROWS(f3 = new LineConfigFile(ff3, LineConfigFileOptions()), ConfigFileError);
            }
        }
    } test_config_file_open_file;

    /**
     * \test Test LineConfigFile.
     *
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
            LineConfigFile ff(s, LineConfigFileOptions());
            std::vector<std::string> f(ff.begin(), ff.end());

            TEST_CHECK_EQUAL(f.size(), std::size_t(4));
            TEST_CHECK_EQUAL(f.at(0), "one");
            TEST_CHECK_EQUAL(f.at(1), "two");
            TEST_CHECK_EQUAL(f.at(2), "three");
            TEST_CHECK_EQUAL(f.at(3), "four  four");
        }
    } test_line_config_file;

    /**
     * \test Test KeyValueConfigFile basics.
     *
     */
    struct KeyValueConfigFileTest : TestCase
    {
        KeyValueConfigFileTest() : TestCase("key value config file") { }

        void run()
        {
            std::stringstream s;
            s << "one=first" << std::endl;
            s << "two = second" << std::endl;
            s << "three=" << std::endl;
            s << "four = \"fourth\" " << std::endl;
            s << "five = ''" << std::endl;
            KeyValueConfigFile ff(s, KeyValueConfigFileOptions() + kvcfo_preserve_whitespace);

            TEST_CHECK_EQUAL(ff.get("one"), "first");
            TEST_CHECK_EQUAL(ff.get("two"), "second");
            TEST_CHECK_EQUAL(ff.get("three"), "");
            TEST_CHECK_EQUAL(ff.get("four"), "fourth ");
            TEST_CHECK_EQUAL(ff.get("five"), "");
            TEST_CHECK_EQUAL(ff.get("six"), "");
        }
    } test_key_value_config_file;

    /**
     * \test Test KeyValueConfigFile continuations.
     *
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
            KeyValueConfigFile ff(s, KeyValueConfigFileOptions());

            TEST_CHECK_EQUAL(ff.get("one"), "first\n first \nfirst");
        }
    } test_key_value_config_file_continuations;

    /**
     * \test Test KeyValueConfigFile variables.
     *
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
            KeyValueConfigFile ff(s, KeyValueConfigFileOptions());

            TEST_CHECK_EQUAL(ff.get("x"), "foo");
            TEST_CHECK_EQUAL(ff.get("y"), "foo\\$");
            TEST_CHECK_EQUAL(ff.get("z"), "foofoo\\$");

            std::stringstream t;
            std::tr1::shared_ptr<Map<std::string, std::string> > t_defs(new Map<std::string, std::string>);
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
            KeyValueConfigFile fg(t, KeyValueConfigFileOptions(), KeyValueConfigFile::Defaults(t_defs));

            TEST_CHECK_EQUAL(fg.get("a"), "foo");
            TEST_CHECK_EQUAL(fg.get("b"), "foo");
            TEST_CHECK_EQUAL(fg.get("c"), "bar");
            TEST_CHECK_EQUAL(fg.get("d"), "bar");
            TEST_CHECK_EQUAL(fg.get("e"), "baz");
            TEST_CHECK_EQUAL(fg.get("f"), "");
            TEST_CHECK_EQUAL(fg.get("g"), "foo     bar");
        }
    } test_key_value_config_file_vars;

    struct KeyValueConfigFileDefaultsTest : TestCase
    {
        KeyValueConfigFileDefaultsTest() : TestCase("key value config file with defaults") { }

        void run()
        {
            setenv("moo", "cow", 1);

            std::stringstream d_s;
            d_s << "foo=oink" << std::endl;
            std::tr1::shared_ptr<KeyValueConfigFile> d_ff(new KeyValueConfigFile(d_s, KeyValueConfigFileOptions(), &getenv_with_default));

            std::stringstream s;
            s << "x=${foo}" << std::endl;
            s << "y=${moo}" << std::endl;
            KeyValueConfigFile ff(s, KeyValueConfigFileOptions(), d_ff);

            TEST_CHECK_EQUAL(ff.get("x"), "oink");
            TEST_CHECK_EQUAL(ff.get("y"), "cow");
        }
    } test_key_value_config_file_defaults;

    struct KeyValueConfigFileSourceTest : TestCase
    {
        KeyValueConfigFileSourceTest() : TestCase("key value config file source") { }

        void run()
        {
            std::stringstream d_s;
            d_s << "six=\"llama\"" << std::endl;
            d_s << "seven=\"spider\"" << std::endl;
            d_s << "source config_file_TEST_dir/sourced_one" << std::endl;
            d_s << "eight=\"octopus\"" << std::endl;
            KeyValueConfigFile ff(d_s, KeyValueConfigFileOptions());

            TEST_CHECK_EQUAL(ff.get("one"), "cat");
            TEST_CHECK_EQUAL(ff.get("two"), "dog");
            TEST_CHECK_EQUAL(ff.get("three"), "koala");
            TEST_CHECK_EQUAL(ff.get("four"), "sheep");
            TEST_CHECK_EQUAL(ff.get("five"), "rabbit");
            TEST_CHECK_EQUAL(ff.get("six"), "llama");
            TEST_CHECK_EQUAL(ff.get("seven"), "spider");
            TEST_CHECK_EQUAL(ff.get("eight"), "octopus");
        }
    } test_key_value_config_file_source;

    /**
     * \test Test KeyValueConfigFile ignore export.
     *
     */
    struct KeyValueConfigFileIgnoreExportTest : TestCase
    {
        KeyValueConfigFileIgnoreExportTest() : TestCase("key value config file ignore export") { }

        void run()
        {
            std::stringstream d_s;
            d_s << "export  \t foo = \"xyzzy\"" << std::endl;
            d_s << "export bar='plugh'" << std::endl;
            d_s << "baz = \"plover\"" << std::endl;
            d_s << "exportfoo = \"exportxyzzy\"" << std::endl;
            KeyValueConfigFile ff(d_s, KeyValueConfigFileOptions() += kvcfo_ignore_export);

            TEST_CHECK_EQUAL(ff.get("foo"), "xyzzy");
            TEST_CHECK_EQUAL(ff.get("bar"), "plugh");
            TEST_CHECK_EQUAL(ff.get("baz"), "plover");
            TEST_CHECK_EQUAL(ff.get("exportfoo"), "exportxyzzy");

            std::stringstream d_s2;
            d_s2 << "export = 42" << std::endl;
            KeyValueConfigFile ff2(d_s2, KeyValueConfigFileOptions());

            TEST_CHECK_EQUAL(ff2.get("export"), "42");
        }
    } test_key_value_config_file_ignore_export;

    /**
     * \test Test KeyValueConfigFile errors.
     *
     */
    struct KeyValueConfigFileErrorsTest : TestCase
    {
        KeyValueConfigFileErrorsTest() : TestCase("key value config file with errors") { }

        void run()
        {
            std::stringstream s1;
            s1 << "x='" << std::endl;
            TEST_CHECK_THROWS(KeyValueConfigFile ff(s1, KeyValueConfigFileOptions()), ConfigurationError);

            std::stringstream s2;
            s2 << "x='moo\"" << std::endl;
            TEST_CHECK_THROWS(KeyValueConfigFile ff(s2, KeyValueConfigFileOptions()), ConfigurationError);

            std::stringstream s3;
            s3 << "x=${foo" << std::endl;
            TEST_CHECK_THROWS(KeyValueConfigFile ff(s3, KeyValueConfigFileOptions()), ConfigurationError);

            std::stringstream s4;
            s4 << "x=$~" << std::endl;
            TEST_CHECK_THROWS(KeyValueConfigFile ff(s4, KeyValueConfigFileOptions()), ConfigurationError);

            std::stringstream s5;
            s5 << "x=abc\\" << std::endl;
            TEST_CHECK_THROWS(KeyValueConfigFile ff(s5, KeyValueConfigFileOptions()), ConfigurationError);

            std::stringstream s6;
            s6 << "x=$" << std::endl;
            TEST_CHECK_THROWS(KeyValueConfigFile ff(s6, KeyValueConfigFileOptions()), ConfigurationError);

            std::stringstream s7;
            s7 << "x=blah \\" << std::endl;
            TEST_CHECK_THROWS(KeyValueConfigFile ff(s7, KeyValueConfigFileOptions()), ConfigurationError);

            std::stringstream s8;
            s8 << "x=blah \\" << std::endl << "# foo" << std::endl;
            TEST_CHECK_THROWS(KeyValueConfigFile ff(s8, KeyValueConfigFileOptions()), ConfigurationError);

            std::stringstream s9;
            s9 << "x='blah" << std::endl << "blah" << std::endl;
            TEST_CHECK_THROWS(KeyValueConfigFile ff(s9, KeyValueConfigFileOptions()), ConfigurationError);

            std::stringstream s10;
            s10 << "export x=blah" << std::endl;
            TEST_CHECK_THROWS(KeyValueConfigFile ff(s10, KeyValueConfigFileOptions()), ConfigurationError);

            std::stringstream s11;
            s11 << "export x = blah" << std::endl;
            TEST_CHECK_THROWS(KeyValueConfigFile ff(s11, (KeyValueConfigFileOptions() += kvcfo_ignore_export) += kvcfo_disallow_space_around_equals), ConfigurationError);

            std::stringstream s12;
            s12 << "export=blah" << std::endl;
            TEST_CHECK_THROWS(KeyValueConfigFile ff(s12, KeyValueConfigFileOptions() += kvcfo_ignore_export), ConfigurationError);
        }
    } test_key_value_config_file_errors;
}

