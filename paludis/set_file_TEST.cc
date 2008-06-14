/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008 Ciaran McCreesh
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

#include "set_file.hh"
#include <test/test_runner.hh>
#include <test/test_framework.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/dep_spec.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/environments/test/test_environment.hh>
#include <fstream>

using namespace test;
using namespace paludis;

namespace
{
    struct SetSpecStringifier :
        ConstVisitor<SetSpecTree>
    {
        std::ostringstream s;

        void
        visit_sequence(const AllDepSpec &,
                SetSpecTree::ConstSequenceIterator cur,
                SetSpecTree::ConstSequenceIterator end)
        {
            s << "( ";
            std::for_each(cur, end, accept_visitor(*this));
            s << ") ";
        }

        void
        visit_leaf(const PackageDepSpec & p)
        {
            s << p << " ";
        }

        void
        visit_leaf(const NamedSetDepSpec & p)
        {
            s << p << " ";
        }
    };
}

namespace test_cases
{
    struct SimpleTest : TestCase
    {
        SimpleTest() : TestCase("simple set file") { }

        void run()
        {
            using namespace std::tr1::placeholders;

            TestEnvironment env;

            SetFile f(SetFileParams::create()
                    .file_name(FSEntry("set_file_TEST_dir/simple1"))
                    .type(sft_simple)
                    .parser(std::tr1::bind(&parse_user_package_dep_spec, _1, &env, UserPackageDepSpecOptions(),
                            filter::All()))
                    .tag(std::tr1::shared_ptr<DepTag>())
                    .set_operator_mode(sfsmo_natural)
                    .environment(0));

            {
                SetSpecStringifier p;
                f.contents()->accept(p);
                TEST_CHECK_STRINGIFY_EQUAL(p.s.str(), "( foo/bar >=bar/baz-1.23 ) ");
            }

            f.add("foo/bar");
            f.add("moo/oink");
            {
                SetSpecStringifier p;
                f.contents()->accept(p);
                TEST_CHECK_STRINGIFY_EQUAL(p.s.str(), "( foo/bar >=bar/baz-1.23 moo/oink ) ");
            }

            f.rewrite();

            {
                std::ifstream ff("set_file_TEST_dir/simple1");
                TEST_CHECK(ff);
                std::string g((std::istreambuf_iterator<char>(ff)), std::istreambuf_iterator<char>());
                TEST_CHECK_EQUAL(g, "# this is a comment\n\nfoo/bar\n>=bar/baz-1.23\n\n# the end\nmoo/oink\n");
            }

            f.remove(">=bar/baz-1.23");
            f.remove("bar/cow");

            {
                SetSpecStringifier p;
                f.contents()->accept(p);
                TEST_CHECK_STRINGIFY_EQUAL(p.s.str(), "( foo/bar moo/oink ) ");
            }

            f.rewrite();

            {
                std::ifstream ff("set_file_TEST_dir/simple1");
                TEST_CHECK(ff);
                std::string g((std::istreambuf_iterator<char>(ff)), std::istreambuf_iterator<char>());
                TEST_CHECK_EQUAL(g, "# this is a comment\n\nfoo/bar\n\n# the end\nmoo/oink\n");
            }
        }

        bool repeatable() const
        {
            return false;
        }
    } test_simple;

    struct PaludisConfTest : TestCase
    {
        PaludisConfTest() : TestCase("paludis .conf set file") { }

        void run()
        {
            using namespace std::tr1::placeholders;

            TestEnvironment env;

            SetFile f(SetFileParams::create()
                    .file_name(FSEntry("set_file_TEST_dir/paludisconf1"))
                    .type(sft_paludis_conf)
                    .parser(std::tr1::bind(&parse_user_package_dep_spec, _1, &env, UserPackageDepSpecOptions(),
                            filter::All()))
                    .tag(std::tr1::shared_ptr<DepTag>())
                    .set_operator_mode(sfsmo_natural)
                    .environment(0));

            {
                SetSpecStringifier p;
                f.contents()->accept(p);
                TEST_CHECK_STRINGIFY_EQUAL(p.s.str(), "( >=bar/baz-1.23 set ) ");
            }

            f.add("foo/bar");
            f.add("moo/oink");
            f.add("settee");
            f.add("couch");
            {
                SetSpecStringifier p;
                f.contents()->accept(p);
                TEST_CHECK_STRINGIFY_EQUAL(p.s.str(), "( >=bar/baz-1.23 set moo/oink couch ) ");
            }

            f.rewrite();

            {
                std::ifstream ff("set_file_TEST_dir/paludisconf1");
                TEST_CHECK(ff);
                std::string g((std::istreambuf_iterator<char>(ff)), std::istreambuf_iterator<char>());
                TEST_CHECK_EQUAL(g, "# this is a comment\n\n? foo/bar\n* >=bar/baz-1.23\n\n* set\n? settee\n\n# the end\n* moo/oink\n* couch\n");
            }

            f.remove(">=bar/baz-1.23");
            f.remove("bar/cow");
            f.remove("set");

            {
                SetSpecStringifier p;
                f.contents()->accept(p);
                TEST_CHECK_STRINGIFY_EQUAL(p.s.str(), "( moo/oink couch ) ");
            }

            f.rewrite();

            {
                std::ifstream ff("set_file_TEST_dir/paludisconf1");
                TEST_CHECK(ff);
                std::string g((std::istreambuf_iterator<char>(ff)), std::istreambuf_iterator<char>());
                TEST_CHECK_EQUAL(g, "# this is a comment\n\n? foo/bar\n\n? settee\n\n# the end\n* moo/oink\n* couch\n");
            }
        }

        bool repeatable() const
        {
            return false;
        }
    } test_paludis_conf;

    struct OverrideTest : TestCase
    {
        OverrideTest() : TestCase("operator overrides") { }

        void run()
        {
            using namespace std::tr1::placeholders;

            TestEnvironment env;

            SetFile f(SetFileParams::create()
                    .file_name(FSEntry("set_file_TEST_dir/override"))
                    .type(sft_paludis_conf)
                    .parser(std::tr1::bind(&parse_user_package_dep_spec, _1, &env, UserPackageDepSpecOptions(),
                            filter::All()))
                    .tag(std::tr1::shared_ptr<DepTag>())
                    .set_operator_mode(sfsmo_natural)
                    .environment(0));

            {
                SetSpecStringifier p;
                f.contents()->accept(p);
                TEST_CHECK_STRINGIFY_EQUAL(p.s.str(), "( >=bar/bar-1.23 set set2* ) ");
            }

            SetFile fstar(SetFileParams::create()
                    .file_name(FSEntry("set_file_TEST_dir/override"))
                    .type(sft_paludis_conf)
                    .parser(std::tr1::bind(&parse_user_package_dep_spec, _1, &env, UserPackageDepSpecOptions(),
                            filter::All()))
                    .tag(std::tr1::shared_ptr<DepTag>())
                    .set_operator_mode(sfsmo_star)
                    .environment(0));

            {
                SetSpecStringifier p;
                fstar.contents()->accept(p);
                TEST_CHECK_STRINGIFY_EQUAL(p.s.str(), "( foo/foo >=bar/bar-1.23 >=baz/baz-1.23 set* set2* ) ");
            }

        }
    } test_overrides;
}

