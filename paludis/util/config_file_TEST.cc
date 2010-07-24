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

namespace
{
    std::string predefined(const std::shared_ptr<const Map<std::string, std::string> > & m,
            const KeyValueConfigFile &, const std::string & s)
    {
        if (m->end() == m->find(s))
            return "";
        return m->find(s)->second;
    }

    std::string predefined_from_env(const KeyValueConfigFile &, const std::string & s)
    {
        return getenv_with_default(s, "");
    }

    std::string predefined_from_config_file(const KeyValueConfigFile & f,
            const KeyValueConfigFile &, const std::string & s)
    {
        return f.get(s);
    }
}

namespace test_cases
{
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
            s << "five \\" << std::endl;
            s << "six" << std::endl;
            s << "seven\\" << std::endl;
            s << "eight" << std::endl;
            s << "nine # ten" << std::endl;

            LineConfigFile f(s, { });
            TEST_CHECK_EQUAL(std::distance(f.begin(), f.end()), 7);
            std::vector<std::string> lines;
            std::copy(f.begin(), f.end(), std::back_inserter(lines));
            TEST_CHECK_EQUAL(lines.at(0), "one");
            TEST_CHECK_EQUAL(lines.at(1), "two");
            TEST_CHECK_EQUAL(lines.at(2), "three");
            TEST_CHECK_EQUAL(lines.at(3), "four four");
            TEST_CHECK_EQUAL(lines.at(4), "five six");
            TEST_CHECK_EQUAL(lines.at(5), "seveneight");
            TEST_CHECK_EQUAL(lines.at(6), "nine # ten");

            s.clear();
            s.seekg(0, std::ios::beg);

            LineConfigFile g(s, { lcfo_disallow_comments, lcfo_preserve_whitespace, lcfo_no_skip_blank_lines });
            TEST_CHECK_EQUAL(std::distance(g.begin(), g.end()), 13);
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
            TEST_CHECK_EQUAL(lines.at(11), "seveneight");
            TEST_CHECK_EQUAL(lines.at(12), "nine # ten");

            s.clear();
            s.seekg(0, std::ios::beg);

            LineConfigFile h(s, { lcfo_allow_inline_comments });
            TEST_CHECK_EQUAL(std::distance(f.begin(), f.end()), 7);
            lines.clear();
            std::copy(h.begin(), h.end(), std::back_inserter(lines));
            TEST_CHECK_EQUAL(lines.at(0), "one");
            TEST_CHECK_EQUAL(lines.at(1), "two");
            TEST_CHECK_EQUAL(lines.at(2), "three");
            TEST_CHECK_EQUAL(lines.at(3), "four four");
            TEST_CHECK_EQUAL(lines.at(4), "five six");
            TEST_CHECK_EQUAL(lines.at(5), "seveneight");
            TEST_CHECK_EQUAL(lines.at(6), "nine");
        }
    } test_line_config_file;

    struct ConfigFileOpenFileTest : TestCase
    {
        ConfigFileOpenFileTest() : TestCase("config file open file") { }

        void run()
        {
            FSEntry ff("config_file_TEST_dir/config_file");
            TEST_CHECK(ff.is_regular_file());
            LineConfigFile f(ff, { });
            TEST_CHECK_EQUAL(std::distance(f.begin(), f.end()), 1);
            TEST_CHECK_EQUAL(*f.begin(), "I am a fish.");

            FSEntry ff2("config_file_TEST_dir/not_a_config_file");
            TEST_CHECK(! ff2.exists());
            LineConfigFile * f2(0);
            TEST_CHECK_THROWS(f2 = new LineConfigFile(ff2, { }), ConfigFileError);

            if (0 != geteuid())
            {
                FSEntry ff3("config_file_TEST_dir/unreadable_file");
                TEST_CHECK(ff3.is_regular_file());
                LineConfigFile * f3(0);
                TEST_CHECK_THROWS(f3 = new LineConfigFile(ff3, { }), ConfigFileError);
            }
        }
    } test_config_file_open_file;

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
            KeyValueConfigFile ff(s, { kvcfo_preserve_whitespace },
                    &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation);

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
            KeyValueConfigFile ff(s, { },
                    &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation);

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
            KeyValueConfigFile ff(s, { },
                    &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation);

            TEST_CHECK_EQUAL(ff.get("x"), "foo");
            TEST_CHECK_EQUAL(ff.get("y"), "foo\\$");
            TEST_CHECK_EQUAL(ff.get("z"), "foofoo\\$");

            std::stringstream t;
            std::shared_ptr<Map<std::string, std::string> > t_defs(std::make_shared<Map<std::string, std::string>>());
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
            KeyValueConfigFile fg(t, { },
                    std::bind(&predefined, t_defs, std::placeholders::_1, std::placeholders::_2),
                    &KeyValueConfigFile::no_transformation);

            TEST_CHECK_EQUAL(fg.get("a"), "foo");
            TEST_CHECK_EQUAL(fg.get("b"), "foo");
            TEST_CHECK_EQUAL(fg.get("c"), "bar");
            TEST_CHECK_EQUAL(fg.get("d"), "bar");
            TEST_CHECK_EQUAL(fg.get("e"), "baz");
            TEST_CHECK_EQUAL(fg.get("f"), "");
            TEST_CHECK_EQUAL(fg.get("g"), "foo bar");
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
            std::shared_ptr<KeyValueConfigFile> d_ff(std::make_shared<KeyValueConfigFile>(d_s, KeyValueConfigFileOptions(),
                        &predefined_from_env,
                        &KeyValueConfigFile::no_transformation));

            std::stringstream s;
            s << "x=${foo}" << std::endl;
            s << "y=${moo}" << std::endl;
            KeyValueConfigFile ff(s, { },
                    std::bind(&predefined_from_config_file, std::cref(*d_ff), std::placeholders::_1, std::placeholders::_2),
                    &KeyValueConfigFile::no_transformation);

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
            KeyValueConfigFile ff(d_s, { },
                    &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation);

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
            KeyValueConfigFile ff(d_s, { kvcfo_ignore_export },
                    &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation);

            TEST_CHECK_EQUAL(ff.get("foo"), "xyzzy");
            TEST_CHECK_EQUAL(ff.get("bar"), "plugh");
            TEST_CHECK_EQUAL(ff.get("baz"), "plover");
            TEST_CHECK_EQUAL(ff.get("exportfoo"), "exportxyzzy");

            std::stringstream d_s2;
            d_s2 << "export = 42" << std::endl;
            KeyValueConfigFile ff2(d_s2, { },
                    &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation);

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
            TEST_CHECK_THROWS(KeyValueConfigFile ff(s1, { },
                        &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation), ConfigurationError);

            std::stringstream s2;
            s2 << "x='moo\"" << std::endl;
            TEST_CHECK_THROWS(KeyValueConfigFile ff(s2, { },
                        &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation), ConfigurationError);

            std::stringstream s3;
            s3 << "x=${foo" << std::endl;
            TEST_CHECK_THROWS(KeyValueConfigFile ff(s3, { },
                        &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation), ConfigurationError);

            std::stringstream s4;
            s4 << "x=$~" << std::endl;
            TEST_CHECK_THROWS(KeyValueConfigFile ff(s4, { },
                        &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation), ConfigurationError);

            std::stringstream s5;
            s5 << "x=abc\\" << std::endl;
            TEST_CHECK_THROWS(KeyValueConfigFile ff(s5, { },
                        &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation), ConfigurationError);

            std::stringstream s6;
            s6 << "x=$" << std::endl;
            TEST_CHECK_THROWS(KeyValueConfigFile ff(s6, { },
                        &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation), ConfigurationError);

            std::stringstream s7;
            s7 << "x=blah \\" << std::endl;
            TEST_CHECK_THROWS(KeyValueConfigFile ff(s7, { },
                        &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation), ConfigurationError);

            std::stringstream s8;
            s8 << "x=blah \\" << std::endl << "# foo" << std::endl;
            TEST_CHECK_THROWS(KeyValueConfigFile ff(s8, { },
                        &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation), ConfigurationError);

            std::stringstream s9;
            s9 << "x='blah" << std::endl << "blah" << std::endl;
            TEST_CHECK_THROWS(KeyValueConfigFile ff(s9, { },
                        &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation), ConfigurationError);

            std::stringstream s10;
            s10 << "export x=blah" << std::endl;
            TEST_CHECK_THROWS(KeyValueConfigFile ff(s10, { },
                        &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation), ConfigurationError);

            std::stringstream s11;
            s11 << "export x = blah" << std::endl;
            TEST_CHECK_THROWS(KeyValueConfigFile ff(s11, { kvcfo_ignore_export, kvcfo_disallow_space_around_equals },
                        &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation), ConfigurationError);
        }
    } test_key_value_config_file_errors;

    struct LineConfigFileInlineCommentsTest : TestCase
    {
        LineConfigFileInlineCommentsTest() : TestCase("line config file inline comments") { }

        void run()
        {
            std::stringstream s;
            s << "one # moo" << std::endl;
            s << "tw\\" << std::endl;
            s << "o # moo" << std::endl;
            s << "" << std::endl;
            s << "three #" << std::endl;
            s << "#" << std::endl;
            s << "four" << std::endl;
            s << "five # moo" << std::endl;

            LineConfigFile f(s, { lcfo_allow_inline_comments });
            TEST_CHECK_EQUAL(std::distance(f.begin(), f.end()), 5);
            std::vector<std::string> lines;
            std::copy(f.begin(), f.end(), std::back_inserter(lines));
            TEST_CHECK_EQUAL(lines.at(0), "one");
            TEST_CHECK_EQUAL(lines.at(1), "two");
            TEST_CHECK_EQUAL(lines.at(2), "three");
            TEST_CHECK_EQUAL(lines.at(3), "four");
            TEST_CHECK_EQUAL(lines.at(4), "five");
        }
    } test_line_config_file_inline_comments;

    struct KeyValueConfigFileInlineCommentsTest : TestCase
    {
        KeyValueConfigFileInlineCommentsTest() : TestCase("key value config inline comments") { }

        void run()
        {
            std::stringstream d_s;
            d_s << "one=\"one\" # foo" << std::endl;
            d_s << "two=two # bar" << std::endl;
            d_s << "three = \\" << std::endl;
            d_s << "three # bar" << std::endl;
            d_s << "four = four # foo" << std::endl;
            d_s << "five = five # moo" << std::endl;
            KeyValueConfigFile ff(d_s, { kvcfo_allow_inline_comments },
                    &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation);

            TEST_CHECK_EQUAL(std::distance(ff.begin(), ff.end()), 5);
            TEST_CHECK_EQUAL(ff.get("one"), "one");
            TEST_CHECK_EQUAL(ff.get("two"), "two");
            TEST_CHECK_EQUAL(ff.get("three"), "three");
            TEST_CHECK_EQUAL(ff.get("four"), "four");
            TEST_CHECK_EQUAL(ff.get("five"), "five");
        }
    } test_key_value_config_file_inline_comments;

    struct KeyValueConfigFileMultipleAssignsTest : TestCase
    {
        KeyValueConfigFileMultipleAssignsTest() : TestCase("key value config multiple assigns") { }

        void run()
        {
            std::stringstream d_s;
            d_s << "one=\"one\" two=two" << std::endl;
            d_s << "three = \\" << std::endl;
            d_s << "three four = \\" << std::endl;
            d_s << "\"four\" # one=three" << std::endl;
            d_s << "five=five # six=six" << std::endl;
            KeyValueConfigFile ff(d_s, { kvcfo_allow_inline_comments, kvcfo_allow_multiple_assigns_per_line, kvcfo_disallow_space_inside_unquoted_values },
                    &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation);

            TEST_CHECK_EQUAL(std::distance(ff.begin(), ff.end()), 5);
            TEST_CHECK_EQUAL(ff.get("one"), "one");
            TEST_CHECK_EQUAL(ff.get("two"), "two");
            TEST_CHECK_EQUAL(ff.get("three"), "three");
            TEST_CHECK_EQUAL(ff.get("four"), "four");
            TEST_CHECK_EQUAL(ff.get("five"), "five");
        }
    } test_key_value_config_file_multiple_assigns;

    struct KeyValueConfigFileSectionsTest : TestCase
    {
        KeyValueConfigFileSectionsTest() : TestCase("key value config sections") { }

        void run()
        {
            std::stringstream d_s;
            d_s << "a = b" << std::endl;
            d_s << "c = d" << std::endl;
            d_s << "[foo]" << std::endl;
            d_s << "a = c" << std::endl;
            d_s << "[bar bar]" << std::endl;
            d_s << "a = d" << std::endl;
            d_s << "[var]" << std::endl;
            d_s << "b = x" << std::endl;
            d_s << "a = ${a} ${b} ${c}" << std::endl;
            KeyValueConfigFile ff(d_s, { kvcfo_allow_sections },
                    &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation);

            TEST_CHECK_EQUAL(std::distance(ff.begin(), ff.end()), 6);
            TEST_CHECK_EQUAL(ff.get("a"), "b");
            TEST_CHECK_EQUAL(ff.get("c"), "d");
            TEST_CHECK_EQUAL(ff.get("foo/a"), "c");
            TEST_CHECK_EQUAL(ff.get("bar/bar/a"), "d");
            TEST_CHECK_EQUAL(ff.get("var/b"), "x");
            TEST_CHECK_EQUAL(ff.get("var/a"), "b x d");
        }
    } test_key_value_config_file_sections;
}

