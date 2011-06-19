/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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
#include <paludis/util/fs_path.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/system.hh>
#include <paludis/util/map.hh>
#include <paludis/util/options.hh>
#include <paludis/util/wrapped_forward_iterator.hh>

#include <sstream>
#include <vector>
#include <cstdlib>
#include <iomanip>
#include <unistd.h>

#include <gtest/gtest.h>

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

TEST(LineConfigFile, Works)
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
    ASSERT_EQ(7, std::distance(f.begin(), f.end()));
    std::vector<std::string> lines;
    std::copy(f.begin(), f.end(), std::back_inserter(lines));
    EXPECT_EQ("one", lines.at(0));
    EXPECT_EQ("two", lines.at(1));
    EXPECT_EQ("three", lines.at(2));
    EXPECT_EQ("four four", lines.at(3));
    EXPECT_EQ("five six", lines.at(4));
    EXPECT_EQ("seveneight", lines.at(5));
    EXPECT_EQ("nine # ten", lines.at(6));

    s.clear();
    s.seekg(0, std::ios::beg);

    LineConfigFile g(s, { lcfo_disallow_comments, lcfo_preserve_whitespace, lcfo_no_skip_blank_lines });
    ASSERT_EQ(13, std::distance(g.begin(), g.end()));
    lines.clear();
    std::copy(g.begin(), g.end(), std::back_inserter(lines));
    EXPECT_EQ("one", lines.at(0));
    EXPECT_EQ("  two    \t  ", lines.at(1));
    EXPECT_EQ("   \t  ", lines.at(2));
    EXPECT_EQ("", lines.at(3));
    EXPECT_EQ("three", lines.at(4));
    EXPECT_EQ("# blah", lines.at(5));
    EXPECT_EQ("  # blah", lines.at(6));
    EXPECT_EQ("#", lines.at(7));
    EXPECT_EQ("  #  \t  ", lines.at(8));
    EXPECT_EQ("four  four", lines.at(9));
    EXPECT_EQ("five six", lines.at(10));
    EXPECT_EQ("seveneight", lines.at(11));
    EXPECT_EQ("nine # ten", lines.at(12));

    s.clear();
    s.seekg(0, std::ios::beg);

    LineConfigFile h(s, { lcfo_allow_inline_comments });
    ASSERT_EQ(7, std::distance(f.begin(), f.end()));
    lines.clear();
    std::copy(h.begin(), h.end(), std::back_inserter(lines));
    EXPECT_EQ("one", lines.at(0));
    EXPECT_EQ("two", lines.at(1));
    EXPECT_EQ("three", lines.at(2));
    EXPECT_EQ("four four", lines.at(3));
    EXPECT_EQ("five six", lines.at(4));
    EXPECT_EQ("seveneight", lines.at(5));
    EXPECT_EQ("nine", lines.at(6));
}

TEST(LineConfigFile, Open)
{
    FSPath ff("config_file_TEST_dir/config_file");
    EXPECT_TRUE(ff.stat().is_regular_file());
    LineConfigFile f(ff, { });
    ASSERT_EQ(1, std::distance(f.begin(), f.end()));
    EXPECT_EQ("I am a fish.", *f.begin());

    FSPath ff2("config_file_TEST_dir/not_a_config_file");
    EXPECT_TRUE(! ff2.stat().exists());
    LineConfigFile * f2(0);
    EXPECT_THROW(f2 = new LineConfigFile(ff2, { }), ConfigFileError);

    if (0 != geteuid())
    {
        FSPath ff3("config_file_TEST_dir/unreadable_file");
        EXPECT_TRUE(ff3.stat().is_regular_file());
        LineConfigFile * f3(0);
        EXPECT_THROW(f3 = new LineConfigFile(ff3, { }), ConfigFileError);
    }
}

TEST(KeyValueConfigFile, Works)
{
    std::stringstream s;
    s << "one=first" << std::endl;
    s << "two = second" << std::endl;
    s << "three=" << std::endl;
    s << "four = \"fourth\" " << std::endl;
    s << "five = ''" << std::endl;
    KeyValueConfigFile ff(s, { kvcfo_preserve_whitespace },
            &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation);

    EXPECT_EQ("first", ff.get("one"));
    EXPECT_EQ("second", ff.get("two"));
    EXPECT_EQ("", ff.get("three"));
    EXPECT_EQ("fourth ", ff.get("four"));
    EXPECT_EQ("", ff.get("five"));
    EXPECT_EQ("", ff.get("six"));
}

TEST(KeyValueConfigFile, Continuations)
{
    std::stringstream s;
    s << "one='first" << std::endl;
    s << " first " << std::endl;
    s << "first'" << std::endl;
    KeyValueConfigFile ff(s, { },
            &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation);

    EXPECT_EQ("first\n first \nfirst", ff.get("one"));
}

TEST(KeyValueConfigFile, Variables)
{
    std::stringstream s;
    s << "x=foo" << std::endl;
    s << "y = \"${x}\\\\${y}\\$${z}\"" << std::endl;
    s << "z = $x$y$z" << std::endl;
    KeyValueConfigFile ff(s, { },
            &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation);

    EXPECT_EQ("foo", ff.get("x"));
    EXPECT_EQ("foo\\$", ff.get("y"));
    EXPECT_EQ("foofoo\\$", ff.get("z"));

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

    EXPECT_EQ("foo", fg.get("a"));
    EXPECT_EQ("foo", fg.get("b"));
    EXPECT_EQ("bar", fg.get("c"));
    EXPECT_EQ("bar", fg.get("d"));
    EXPECT_EQ("baz", fg.get("e"));
    EXPECT_EQ("", fg.get("f"));
    EXPECT_EQ("foo bar", fg.get("g"));
}

TEST(KeyValueConfigFile, Defaults)
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

    EXPECT_EQ("oink", ff.get("x"));
    EXPECT_EQ("cow", ff.get("y"));
}

TEST(KeyValueConfigFile, Source)
{
    std::stringstream d_s;
    d_s << "six=\"llama\"" << std::endl;
    d_s << "seven=\"spider\"" << std::endl;
    d_s << "source config_file_TEST_dir/sourced_one" << std::endl;
    d_s << "eight=\"octopus\"" << std::endl;
    KeyValueConfigFile ff(d_s, { },
            &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation);

    EXPECT_EQ("cat", ff.get("one"));
    EXPECT_EQ("dog", ff.get("two"));
    EXPECT_EQ("koala", ff.get("three"));
    EXPECT_EQ("sheep", ff.get("four"));
    EXPECT_EQ("rabbit", ff.get("five"));
    EXPECT_EQ("llama", ff.get("six"));
    EXPECT_EQ("spider", ff.get("seven"));
    EXPECT_EQ("octopus", ff.get("eight"));
}

TEST(KeyValueConfigFile, IgnoreExport)
{
    std::stringstream d_s;
    d_s << "export  \t foo = \"xyzzy\"" << std::endl;
    d_s << "export bar='plugh'" << std::endl;
    d_s << "baz = \"plover\"" << std::endl;
    d_s << "exportfoo = \"exportxyzzy\"" << std::endl;
    KeyValueConfigFile ff(d_s, { kvcfo_ignore_export },
            &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation);

    EXPECT_EQ("xyzzy", ff.get("foo"));
    EXPECT_EQ("plugh", ff.get("bar"));
    EXPECT_EQ("plover", ff.get("baz"));
    EXPECT_EQ("exportxyzzy", ff.get("exportfoo"));

    std::stringstream d_s2;
    d_s2 << "export = 42" << std::endl;
    KeyValueConfigFile ff2(d_s2, { },
            &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation);

    EXPECT_EQ("42", ff2.get("export"));
}

TEST(KeyValueConfigFile, Errors)
{
    std::stringstream s1;
    s1 << "x='" << std::endl;
    EXPECT_THROW(KeyValueConfigFile ff(s1, { },
                &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation), ConfigurationError);

    std::stringstream s2;
    s2 << "x='moo\"" << std::endl;
    EXPECT_THROW(KeyValueConfigFile ff(s2, { },
                &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation), ConfigurationError);

    std::stringstream s3;
    s3 << "x=${foo" << std::endl;
    EXPECT_THROW(KeyValueConfigFile ff(s3, { },
                &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation), ConfigurationError);

    std::stringstream s4;
    s4 << "x=$~" << std::endl;
    EXPECT_THROW(KeyValueConfigFile ff(s4, { },
                &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation), ConfigurationError);

    std::stringstream s5;
    s5 << "x=abc\\" << std::endl;
    EXPECT_THROW(KeyValueConfigFile ff(s5, { },
                &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation), ConfigurationError);

    std::stringstream s6;
    s6 << "x=$" << std::endl;
    EXPECT_THROW(KeyValueConfigFile ff(s6, { },
                &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation), ConfigurationError);

    std::stringstream s7;
    s7 << "x=blah \\" << std::endl;
    EXPECT_THROW(KeyValueConfigFile ff(s7, { },
                &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation), ConfigurationError);

    std::stringstream s8;
    s8 << "x=blah \\" << std::endl << "# foo" << std::endl;
    EXPECT_THROW(KeyValueConfigFile ff(s8, { },
                &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation), ConfigurationError);

    std::stringstream s9;
    s9 << "x='blah" << std::endl << "blah" << std::endl;
    EXPECT_THROW(KeyValueConfigFile ff(s9, { },
                &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation), ConfigurationError);

    std::stringstream s10;
    s10 << "export x=blah" << std::endl;
    EXPECT_THROW(KeyValueConfigFile ff(s10, { },
                &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation), ConfigurationError);

    std::stringstream s11;
    s11 << "export x = blah" << std::endl;
    EXPECT_THROW(KeyValueConfigFile ff(s11, { kvcfo_ignore_export, kvcfo_disallow_space_around_equals },
                &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation), ConfigurationError);
}

TEST(LineConfigFile, InlineComments)
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
    ASSERT_EQ(5, std::distance(f.begin(), f.end()));
    std::vector<std::string> lines;
    std::copy(f.begin(), f.end(), std::back_inserter(lines));
    EXPECT_EQ("one", lines.at(0));
    EXPECT_EQ("two", lines.at(1));
    EXPECT_EQ("three", lines.at(2));
    EXPECT_EQ("four", lines.at(3));
    EXPECT_EQ("five", lines.at(4));
}

TEST(KeyValueConfigFile, InlineComments)
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

    ASSERT_EQ(5, std::distance(ff.begin(), ff.end()));
    EXPECT_EQ("one", ff.get("one"));
    EXPECT_EQ("two", ff.get("two"));
    EXPECT_EQ("three", ff.get("three"));
    EXPECT_EQ("four", ff.get("four"));
    EXPECT_EQ("five", ff.get("five"));
}

TEST(KeyValueConfigFile, MultipleAssigns)
{
    std::stringstream d_s;
    d_s << "one=\"one\" two=two" << std::endl;
    d_s << "three = \\" << std::endl;
    d_s << "three four = \\" << std::endl;
    d_s << "\"four\" # one=three" << std::endl;
    d_s << "five=five # six=six" << std::endl;
    KeyValueConfigFile ff(d_s, { kvcfo_allow_inline_comments, kvcfo_allow_multiple_assigns_per_line, kvcfo_disallow_space_inside_unquoted_values },
            &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation);

    ASSERT_EQ(5, std::distance(ff.begin(), ff.end()));
    EXPECT_EQ("one", ff.get("one"));
    EXPECT_EQ("two", ff.get("two"));
    EXPECT_EQ("three", ff.get("three"));
    EXPECT_EQ("four", ff.get("four"));
    EXPECT_EQ("five", ff.get("five"));
}

TEST(KeyValueConfigFile, Sections)
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

    ASSERT_EQ(6, std::distance(ff.begin(), ff.end()));
    EXPECT_EQ("b", ff.get("a"));
    EXPECT_EQ("d", ff.get("c"));
    EXPECT_EQ("c", ff.get("foo/a"));
    EXPECT_EQ("d", ff.get("bar/bar/a"));
    EXPECT_EQ("x", ff.get("var/b"));
    EXPECT_EQ("b x d", ff.get("var/a"));
}

TEST(KeyValueConfigFile, FancyAssigns)
{
    std::stringstream d_s;
    d_s << "a = A" << std::endl;
    d_s << "a ?= X" << std::endl;
    d_s << "b ?= B" << std::endl;
    KeyValueConfigFile ff(d_s, { kvcfo_allow_sections, kvcfo_allow_fancy_assigns },
            &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation);

    ASSERT_EQ(2, std::distance(ff.begin(), ff.end()));
    EXPECT_EQ("A", ff.get("a"));
    EXPECT_EQ("B", ff.get("b"));
}

TEST(KeyValueConfigFile, EnvVars)
{
    ::setenv("A_VAR", "AAAARGH", 1);
    ::setenv("B_VAR", "BRRRRGH", 1);

    std::stringstream d_s;
    d_s << "a = ${ENV{A_VAR}}" << std::endl;
    d_s << "b = ${ENV{B_VAR}}" << std::endl;
    KeyValueConfigFile ff(d_s, { kvcfo_allow_env },
            &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation);

    ASSERT_EQ(2, std::distance(ff.begin(), ff.end()));
    EXPECT_EQ("AAAARGH", ff.get("a"));
    EXPECT_EQ("BRRRRGH", ff.get("b"));
}

TEST(KeyValueConfigFile, AnnoyingLibtoolQuotes)
{
    std::stringstream s1;
    s1 << "x='foo 'bar' baz'\ny=z";
    EXPECT_THROW(KeyValueConfigFile ff(s1, { },
                &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation), ConfigurationError);

    std::stringstream s2;
    s2 << "x='foo 'bar' baz'\ny=z";
    KeyValueConfigFile ff(s2, { kvcfo_ignore_single_quotes_inside_strings },
            &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation);

    ASSERT_EQ(2, std::distance(ff.begin(), ff.end()));
    EXPECT_EQ("foo 'bar' baz", ff.get("x"));
    EXPECT_EQ("z", ff.get("y"));

    std::stringstream s3;
    s3 << "x='foo 'bar' baz'";
    KeyValueConfigFile f3(s3, { kvcfo_ignore_single_quotes_inside_strings },
            &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation);

    ASSERT_EQ(1, std::distance(f3.begin(), f3.end()));
    EXPECT_EQ("foo 'bar' baz", f3.get("x"));

    std::stringstream s4;
    s4 << "x='foo'";
    KeyValueConfigFile f4(s4, { kvcfo_ignore_single_quotes_inside_strings },
            &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation);

    ASSERT_EQ(1, std::distance(f4.begin(), f4.end()));
    EXPECT_EQ("foo", f4.get("x"));
}

