/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010 Ciaran McCreesh
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
#include <paludis/util/make_named_values.hh>
#include <paludis/dep_spec.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/environments/test/test_environment.hh>
#include <paludis/util/safe_ifstream.hh>

#include <paludis/util/indirect_iterator-impl.hh>

using namespace test;
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

namespace test_cases
{
    struct SimpleTest : TestCase
    {
        SimpleTest() : TestCase("simple set file") { }

        void run()
        {
            using namespace std::tr1::placeholders;

            TestEnvironment env;

            SetFile f(make_named_values<SetFileParams>(
                        value_for<n::environment>(static_cast<Environment *>(0)),
                        value_for<n::file_name>(FSEntry("set_file_TEST_dir/simple1")),
                        value_for<n::parser>(std::tr1::bind(&parse_user_package_dep_spec, _1, &env, UserPackageDepSpecOptions(), filter::All())),
                        value_for<n::set_operator_mode>(sfsmo_natural),
                        value_for<n::tag>(std::tr1::shared_ptr<DepTag>()),
                        value_for<n::type>(sft_simple)
                        ));

            {
                SetSpecStringifier p;
                f.contents()->root()->accept(p);
                TEST_CHECK_STRINGIFY_EQUAL(p.s.str(), "( foo/bar >=bar/baz-1.23 ) ");
            }

            f.add("foo/bar");
            f.add("moo/oink");
            {
                SetSpecStringifier p;
                f.contents()->root()->accept(p);
                TEST_CHECK_STRINGIFY_EQUAL(p.s.str(), "( foo/bar >=bar/baz-1.23 moo/oink ) ");
            }

            f.rewrite();

            {
                SafeIFStream ff(FSEntry("set_file_TEST_dir/simple1"));
                TEST_CHECK(ff);
                std::string g((std::istreambuf_iterator<char>(ff)), std::istreambuf_iterator<char>());
                TEST_CHECK_EQUAL(g, "# this is a comment\n\nfoo/bar\n>=bar/baz-1.23\n\n# the end\nmoo/oink\n");
            }

            TEST_CHECK(f.remove(">=bar/baz-1.23"));
            TEST_CHECK(! f.remove("bar/cow"));

            {
                SetSpecStringifier p;
                f.contents()->root()->accept(p);
                TEST_CHECK_STRINGIFY_EQUAL(p.s.str(), "( foo/bar moo/oink ) ");
            }

            f.rewrite();

            {
                SafeIFStream ff(FSEntry("set_file_TEST_dir/simple1"));
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

            SetFile f(make_named_values<SetFileParams>(
                        value_for<n::environment>(static_cast<Environment *>(0)),
                        value_for<n::file_name>(FSEntry("set_file_TEST_dir/paludisconf1")),
                        value_for<n::parser>(std::tr1::bind(&parse_user_package_dep_spec, _1, &env, UserPackageDepSpecOptions(), filter::All())),
                        value_for<n::set_operator_mode>(sfsmo_natural),
                        value_for<n::tag>(std::tr1::shared_ptr<DepTag>()),
                        value_for<n::type>(sft_paludis_conf)
                        ));

            {
                SetSpecStringifier p;
                f.contents()->root()->accept(p);
                TEST_CHECK_STRINGIFY_EQUAL(p.s.str(), "( >=bar/baz-1.23 set ) ");
            }

            f.add("foo/bar");
            f.add("moo/oink");
            f.add("settee");
            f.add("couch");
            {
                SetSpecStringifier p;
                f.contents()->root()->accept(p);
                TEST_CHECK_STRINGIFY_EQUAL(p.s.str(), "( >=bar/baz-1.23 set moo/oink couch ) ");
            }

            f.rewrite();

            {
                SafeIFStream ff(FSEntry("set_file_TEST_dir/paludisconf1"));
                TEST_CHECK(ff);
                std::string g((std::istreambuf_iterator<char>(ff)), std::istreambuf_iterator<char>());
                TEST_CHECK_EQUAL(g, "# this is a comment\n\n? foo/bar\n* >=bar/baz-1.23\n\n* set\n? settee\n\n# the end\n* moo/oink\n* couch\n");
            }

            TEST_CHECK(f.remove(">=bar/baz-1.23"));
            TEST_CHECK(! f.remove("bar/cow"));
            TEST_CHECK(f.remove("set"));

            {
                SetSpecStringifier p;
                f.contents()->root()->accept(p);
                TEST_CHECK_STRINGIFY_EQUAL(p.s.str(), "( moo/oink couch ) ");
            }

            f.rewrite();

            {
                SafeIFStream ff(FSEntry("set_file_TEST_dir/paludisconf1"));
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

            SetFile f(make_named_values<SetFileParams>(
                        value_for<n::environment>(static_cast<Environment *>(0)),
                        value_for<n::file_name>(FSEntry("set_file_TEST_dir/override")),
                        value_for<n::parser>(std::tr1::bind(&parse_user_package_dep_spec, _1, &env, UserPackageDepSpecOptions(), filter::All())),
                        value_for<n::set_operator_mode>(sfsmo_natural),
                        value_for<n::tag>(std::tr1::shared_ptr<DepTag>()),
                        value_for<n::type>(sft_paludis_conf)
                        ));

            {
                SetSpecStringifier p;
                f.contents()->root()->accept(p);
                TEST_CHECK_STRINGIFY_EQUAL(p.s.str(), "( >=bar/bar-1.23 set set2* ) ");
            }

            SetFile fstar(make_named_values<SetFileParams>(
                        value_for<n::environment>(static_cast<Environment *>(0)),
                        value_for<n::file_name>(FSEntry("set_file_TEST_dir/override")),
                        value_for<n::parser>(std::tr1::bind(&parse_user_package_dep_spec, _1, &env, UserPackageDepSpecOptions(), filter::All())),
                        value_for<n::set_operator_mode>(sfsmo_star),
                        value_for<n::tag>(std::tr1::shared_ptr<DepTag>()),
                        value_for<n::type>(sft_paludis_conf)
                        ));

            {
                SetSpecStringifier p;
                fstar.contents()->root()->accept(p);
                TEST_CHECK_STRINGIFY_EQUAL(p.s.str(), "( foo/foo >=bar/bar-1.23 >=baz/baz-1.23 set* set2* ) ");
            }

        }
    } test_overrides;
}

