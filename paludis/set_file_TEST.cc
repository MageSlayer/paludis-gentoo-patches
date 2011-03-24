/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#include <paludis/set_file.hh>
#include <paludis/dep_spec.hh>
#include <paludis/user_dep_spec.hh>

#include <paludis/environments/test/test_environment.hh>

#include <paludis/util/make_named_values.hh>
#include <paludis/util/safe_ifstream.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/accept_visitor.hh>
#include <paludis/util/stringify.hh>

#include <algorithm>

#include <gtest/gtest.h>

using namespace paludis;

namespace
{
    struct SetSpecStringifier
    {
        std::ostringstream s;

        void visit(const SetSpecTree::NodeType<AllDepSpec>::Type & node)
        {
            s << "( ";
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
            s << ") ";
        }

        void visit(const SetSpecTree::NodeType<PackageDepSpec>::Type & node)
        {
            s << *node.spec() << " ";
        }

        void visit(const SetSpecTree::NodeType<NamedSetDepSpec>::Type & node)
        {
            s << *node.spec() << " ";
        }
    };
}

TEST(SetFile, Simple)
{
    using namespace std::placeholders;

    TestEnvironment env;

    SetFile f(make_named_values<SetFileParams>(
                n::environment() = static_cast<Environment *>(0),
                n::file_name() = FSPath("set_file_TEST_dir/simple1"),
                n::parser() = std::bind(&parse_user_package_dep_spec, _1, &env, UserPackageDepSpecOptions(), filter::All()),
                n::set_operator_mode() = sfsmo_natural,
                n::type() = sft_simple
                ));

    {
        SetSpecStringifier p;
        f.contents()->top()->accept(p);
        EXPECT_EQ("( foo/bar >=bar/baz-1.23 ) ", stringify(p.s.str()));
    }

    f.add("foo/bar");
    f.add("moo/oink");
    {
        SetSpecStringifier p;
        f.contents()->top()->accept(p);
        EXPECT_EQ("( foo/bar >=bar/baz-1.23 moo/oink ) ", stringify(p.s.str()));
    }

    f.rewrite();

    {
        SafeIFStream ff(FSPath("set_file_TEST_dir/simple1"));
        ASSERT_TRUE(ff);
        std::string g((std::istreambuf_iterator<char>(ff)), std::istreambuf_iterator<char>());
        EXPECT_EQ("# this is a comment\n\nfoo/bar\n>=bar/baz-1.23\n\n# the end\nmoo/oink\n", g);
    }

    EXPECT_TRUE(f.remove(">=bar/baz-1.23"));
    EXPECT_TRUE(! f.remove("bar/cow"));

    {
        SetSpecStringifier p;
        f.contents()->top()->accept(p);
        EXPECT_EQ("( foo/bar moo/oink ) ", stringify(p.s.str()));
    }

    f.rewrite();

    {
        SafeIFStream ff(FSPath("set_file_TEST_dir/simple1"));
        ASSERT_TRUE(ff);
        std::string g((std::istreambuf_iterator<char>(ff)), std::istreambuf_iterator<char>());
        EXPECT_EQ("# this is a comment\n\nfoo/bar\n\n# the end\nmoo/oink\n", g);
    }
}

TEST(SetFile, PaludisConf)
{
    using namespace std::placeholders;

    TestEnvironment env;

    SetFile f(make_named_values<SetFileParams>(
                n::environment() = static_cast<Environment *>(0),
                n::file_name() = FSPath("set_file_TEST_dir/paludisconf1"),
                n::parser() = std::bind(&parse_user_package_dep_spec, _1, &env, UserPackageDepSpecOptions(), filter::All()),
                n::set_operator_mode() = sfsmo_natural,
                n::type() = sft_paludis_conf
                ));

    {
        SetSpecStringifier p;
        f.contents()->top()->accept(p);
        EXPECT_EQ("( >=bar/baz-1.23 set ) ", stringify(p.s.str()));
    }

    f.add("foo/bar");
    f.add("moo/oink");
    f.add("settee");
    f.add("couch");
    {
        SetSpecStringifier p;
        f.contents()->top()->accept(p);
        EXPECT_EQ("( >=bar/baz-1.23 set moo/oink couch ) ", stringify(p.s.str()));
    }

    f.rewrite();

    {
        SafeIFStream ff(FSPath("set_file_TEST_dir/paludisconf1"));
        ASSERT_TRUE(ff);
        std::string g((std::istreambuf_iterator<char>(ff)), std::istreambuf_iterator<char>());
        EXPECT_EQ("# this is a comment\n\n? foo/bar\n* >=bar/baz-1.23\n\n* set\n? settee\n\n# the end\n* moo/oink\n* couch\n", g);
    }

    EXPECT_TRUE(f.remove(">=bar/baz-1.23"));
    EXPECT_TRUE(! f.remove("bar/cow"));
    EXPECT_TRUE(f.remove("set"));

    {
        SetSpecStringifier p;
        f.contents()->top()->accept(p);
        EXPECT_EQ("( moo/oink couch ) ", stringify(p.s.str()));
    }

    f.rewrite();

    {
        SafeIFStream ff(FSPath("set_file_TEST_dir/paludisconf1"));
        ASSERT_TRUE(ff);
        std::string g((std::istreambuf_iterator<char>(ff)), std::istreambuf_iterator<char>());
        EXPECT_EQ("# this is a comment\n\n? foo/bar\n\n? settee\n\n# the end\n* moo/oink\n* couch\n", g);
    }
}

TEST(SetFile, Overrides)
{
    using namespace std::placeholders;

    TestEnvironment env;

    SetFile f(make_named_values<SetFileParams>(
                n::environment() = static_cast<Environment *>(0),
                n::file_name() = FSPath("set_file_TEST_dir/override"),
                n::parser() = std::bind(&parse_user_package_dep_spec, _1, &env, UserPackageDepSpecOptions(), filter::All()),
                n::set_operator_mode() = sfsmo_natural,
                n::type() = sft_paludis_conf
                ));

    {
        SetSpecStringifier p;
        f.contents()->top()->accept(p);
        EXPECT_EQ("( >=bar/bar-1.23 set set2* ) ", stringify(p.s.str()));
    }

    SetFile fstar(make_named_values<SetFileParams>(
                n::environment() = static_cast<Environment *>(0),
                n::file_name() = FSPath("set_file_TEST_dir/override"),
                n::parser() = std::bind(&parse_user_package_dep_spec, _1, &env, UserPackageDepSpecOptions(), filter::All()),
                n::set_operator_mode() = sfsmo_star,
                n::type() = sft_paludis_conf
                ));

    {
        SetSpecStringifier p;
        fstar.contents()->top()->accept(p);
        EXPECT_EQ("( foo/foo >=bar/bar-1.23 >=baz/baz-1.23 set* set2* ) ", stringify(p.s.str()));
    }

}

