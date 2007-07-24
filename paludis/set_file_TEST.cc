/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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
#include <libwrapiter/libwrapiter_forward_iterator.hh>
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

    };
}

namespace test_cases
{
    struct SimpleTest : TestCase
    {
        SimpleTest() : TestCase("simple set file") { }

        void run()
        {
            SetFile f(SetFileParams::create()
                    .file_name(FSEntry("set_file_TEST_dir/simple1"))
                    .type(sft_simple)
                    .parse_mode(pds_pm_eapi_0)
                    .tag(tr1::shared_ptr<DepTag>())
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
            SetFile f(SetFileParams::create()
                    .file_name(FSEntry("set_file_TEST_dir/paludisconf1"))
                    .type(sft_paludis_conf)
                    .parse_mode(pds_pm_eapi_0)
                    .tag(tr1::shared_ptr<DepTag>())
                    .environment(0));

            {
                SetSpecStringifier p;
                f.contents()->accept(p);
                TEST_CHECK_STRINGIFY_EQUAL(p.s.str(), "( >=bar/baz-1.23 ) ");
            }

            f.add("foo/bar");
            f.add("moo/oink");
            {
                SetSpecStringifier p;
                f.contents()->accept(p);
                TEST_CHECK_STRINGIFY_EQUAL(p.s.str(), "( >=bar/baz-1.23 moo/oink ) ");
            }

            f.rewrite();

            {
                std::ifstream ff("set_file_TEST_dir/paludisconf1");
                TEST_CHECK(ff);
                std::string g((std::istreambuf_iterator<char>(ff)), std::istreambuf_iterator<char>());
                TEST_CHECK_EQUAL(g, "# this is a comment\n\n? foo/bar\n* >=bar/baz-1.23\n\n# the end\n* moo/oink\n");
            }

            f.remove(">=bar/baz-1.23");
            f.remove("bar/cow");

            {
                SetSpecStringifier p;
                f.contents()->accept(p);
                TEST_CHECK_STRINGIFY_EQUAL(p.s.str(), "( moo/oink ) ");
            }

            f.rewrite();

            {
                std::ifstream ff("set_file_TEST_dir/paludisconf1");
                TEST_CHECK(ff);
                std::string g((std::istreambuf_iterator<char>(ff)), std::istreambuf_iterator<char>());
                TEST_CHECK_EQUAL(g, "# this is a comment\n\n? foo/bar\n\n# the end\n* moo/oink\n");
            }
        }

        bool repeatable() const
        {
            return false;
        }
    } test_paludis_conf;
}

