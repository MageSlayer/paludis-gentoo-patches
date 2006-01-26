/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

#include "paludis.hh"
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <deque>
#include <string>

using namespace paludis;
using namespace test;

namespace test_cases
{
    struct DepListTestCaseBase : TestCase
    {
        TestEnvironment env;
        FakeRepository::Pointer repo;
        std::deque<std::string> expected;
        std::string merge_target;

        DepListTestCaseBase(const char c) :
            TestCase("dep list " + std::string(1, c)),
            env(),
            repo(new FakeRepository(RepositoryName("repo")))
        {
            env.package_database()->add_repository(repo);
        }

        bool repeatable() const
        {
            return false;
        }

        virtual void populate_repo() = 0;

        virtual void populate_expected() = 0;

        void run()
        {
            populate_repo();
            populate_expected();
            check_lists();
        }

        virtual void check_lists()
        {
            TEST_CHECK(true);
            DepList d(&env);
            d.add(DepParser::parse(merge_target));
            TEST_CHECK(true);

            unsigned n(0);
            std::deque<std::string>::const_iterator exp(expected.begin());
            DepList::Iterator got(d.begin());
            while (true)
            {
                TestMessageSuffix s(stringify(n++), true);

                TEST_CHECK((exp == expected.end()) == (got == d.end()));
                if (got == d.end())
                    break;
                TEST_CHECK_STRINGIFY_EQUAL(*got, *exp);
                ++exp;
                ++got;
            }
        }
    };

    template <char c_>
    struct DepListTestCase : DepListTestCaseBase
    {
        DepListTestCase() :
            DepListTestCaseBase(c_)
        {
        }
    };

    struct DepListTestCaseA : DepListTestCase<'a'>
    {
        void populate_repo()
        {
            repo->add_version("a", "one", "1");
        }

        void populate_expected()
        {
            merge_target = "a/one";
            expected.push_back("a/one-1:0::repo");
        }
    } test_dep_list_a;

    struct DepListTestCaseB : DepListTestCase<'b'>
    {
        void populate_repo()
        {
            repo->add_version("b", "one", "1")->set(vmk_depend, "b/two");
            repo->add_version("b", "two", "1");
        }

        void populate_expected()
        {
            merge_target = "b/one";
            expected.push_back("b/two-1:0::repo");
            expected.push_back("b/one-1:0::repo");
        }
    } test_dep_list_b;

    struct DepListTestCaseC : DepListTestCase<'c'>
    {
        void populate_repo()
        {
            repo->add_version("c", "one", "1")->set(vmk_depend, "c/two");
            repo->add_version("c", "two", "1")->set(vmk_depend, "c/three");
            repo->add_version("c", "three", "1");
        }

        void populate_expected()
        {
            merge_target = "c/one";
            expected.push_back("c/three-1:0::repo");
            expected.push_back("c/two-1:0::repo");
            expected.push_back("c/one-1:0::repo");
        }
    } test_dep_list_c;

    struct DepListTestCaseD : DepListTestCase<'d'>
    {
        void populate_repo()
        {
            repo->add_version("d", "one", "1")->set(vmk_depend, "d/two d/three");
            repo->add_version("d", "two", "1");
            repo->add_version("d", "three", "1");
        }

        void populate_expected()
        {
            merge_target = "d/one";
            expected.push_back("d/two-1:0::repo");
            expected.push_back("d/three-1:0::repo");
            expected.push_back("d/one-1:0::repo");
        }
    } test_dep_list_d;

    struct DepListTestCaseE : DepListTestCase<'e'>
    {
        void populate_repo()
        {
            repo->add_version("e", "one", "1")->set(vmk_depend, "e/two e/three");
            repo->add_version("e", "two", "1")->set(vmk_depend, "e/three");
            repo->add_version("e", "three", "1");
        }

        void populate_expected()
        {
            merge_target = "e/one";
            expected.push_back("e/three-1:0::repo");
            expected.push_back("e/two-1:0::repo");
            expected.push_back("e/one-1:0::repo");
        }
    } test_dep_list_e;

    struct DepListTestCaseF : DepListTestCase<'f'>
    {
        void populate_repo()
        {
            repo->add_version("f", "one", "1")->set(vmk_depend, "f/two f/three");
            repo->add_version("f", "two", "1");
            repo->add_version("f", "three", "1")->set(vmk_depend, "f/two");
        }

        void populate_expected()
        {
            merge_target = "f/one";
            expected.push_back("f/two-1:0::repo");
            expected.push_back("f/three-1:0::repo");
            expected.push_back("f/one-1:0::repo");
        }
    } test_dep_list_f;

    struct DepListTestCaseG : DepListTestCase<'g'>
    {
        void populate_repo()
        {
            repo->add_version("g", "one", "1")->set(vmk_depend, "g/two g/three");
            repo->add_version("g", "two", "1")->set(vmk_depend, "g/four");
            repo->add_version("g", "three", "1")->set(vmk_depend, "g/four");
            repo->add_version("g", "four", "1");
        }

        void populate_expected()
        {
            merge_target = "g/one";
            expected.push_back("g/four-1:0::repo");
            expected.push_back("g/two-1:0::repo");
            expected.push_back("g/three-1:0::repo");
            expected.push_back("g/one-1:0::repo");
        }
    } test_dep_list_g;

    struct DepListTestCaseH : DepListTestCase<'h'>
    {
        void populate_repo()
        {
            repo->add_version("h", "one", "1")->set(vmk_depend, "h/two h/three");
            repo->add_version("h", "two", "1")->set(vmk_depend, "h/four h/three");
            repo->add_version("h", "three", "1")->set(vmk_depend, "h/four");
            repo->add_version("h", "four", "1");
        }

        void populate_expected()
        {
            merge_target = "h/one";
            expected.push_back("h/four-1:0::repo");
            expected.push_back("h/three-1:0::repo");
            expected.push_back("h/two-1:0::repo");
            expected.push_back("h/one-1:0::repo");
        }
    } test_dep_list_h;

    struct DepListTestCaseI : DepListTestCase<'i'>
    {
        void populate_repo()
        {
            repo->add_version("i", "one", "1")->set(vmk_depend, "i/two i/three");
            repo->add_version("i", "two", "1")->set(vmk_depend, "i/four");
            repo->add_version("i", "three", "1")->set(vmk_depend, "i/four i/two");
            repo->add_version("i", "four", "1");
        }

        void populate_expected()
        {
            merge_target = "i/one";
            expected.push_back("i/four-1:0::repo");
            expected.push_back("i/two-1:0::repo");
            expected.push_back("i/three-1:0::repo");
            expected.push_back("i/one-1:0::repo");
        }
    } test_dep_list_i;

    struct DepListTestCaseTransactionalAdd : TestCase
    {
        DepListTestCaseTransactionalAdd() : TestCase("dep list transactional add") { }

        void run()
        {
            TestEnvironment env;
            FakeRepository::Pointer repo(new FakeRepository(RepositoryName("repo")));
            env.package_database()->add_repository(repo);

            repo->add_version("i", "one", "1")->set(vmk_depend, "i/two i/three");
            repo->add_version("i", "two", "1")->set(vmk_depend, "i/four");
            repo->add_version("i", "three", "1")->set(vmk_depend, "i/four i/two");
            repo->add_version("i", "four", "1");
            repo->add_version("i", "five", "1")->set(vmk_depend, "i/six i/seven");
            repo->add_version("i", "six", "1");
            repo->add_version("i", "seven", "1")->set(vmk_depend, "i/doesnotexist");

            DepList d(&env);
            d.add(DepParser::parse("i/one"));
            TEST_CHECK_EQUAL(join(d.begin(), d.end(), " "),
                    "i/four-1:0::repo i/two-1:0::repo i/three-1:0::repo i/one-1:0::repo");

            TEST_CHECK_THROWS(d.add(DepParser::parse("i/five")), DepListError);

            TEST_CHECK_EQUAL(join(d.begin(), d.end(), " "),
                    "i/four-1:0::repo i/two-1:0::repo i/three-1:0::repo i/one-1:0::repo");
        }
    } test_dep_list_transactional_add;
}

