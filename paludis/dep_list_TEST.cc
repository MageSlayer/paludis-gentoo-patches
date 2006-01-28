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
        bool done_populate;

        DepListTestCaseBase(const char c) :
            TestCase("dep list " + std::string(1, c)),
            env(),
            repo(new FakeRepository(RepositoryName("repo"))),
            done_populate(false)
        {
            env.package_database()->add_repository(repo);
        }

        virtual void populate_repo() = 0;

        virtual void populate_expected() = 0;

        void run()
        {
            if (! done_populate)
            {
                populate_repo();
                populate_expected();
                done_populate = true;
            }
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

    struct DepListTestCaseJ : DepListTestCase<'j'>
    {
        void populate_repo()
        {
            repo->add_version("j", "one", "1")->set(vmk_depend, "|| ( j/two j/three )");
            repo->add_version("j", "two", "1");
            repo->add_version("j", "three", "1");
        }

        void populate_expected()
        {
            merge_target = "j/one";
            expected.push_back("j/two-1:0::repo");
            expected.push_back("j/one-1:0::repo");
        }
    } test_dep_list_j;

    struct DepListTestCaseK : DepListTestCase<'k'>
    {
        void populate_repo()
        {
            repo->add_version("k", "one", "1")->set(vmk_depend, "k/two k/three");
            repo->add_version("k", "two", "1");
            repo->add_version("k", "three", "1")->set(vmk_depend, "|| ( k/two k/four )");
            repo->add_version("k", "four", "1");
        }

        void populate_expected()
        {
            merge_target = "k/one";
            expected.push_back("k/two-1:0::repo");
            expected.push_back("k/three-1:0::repo");
            expected.push_back("k/one-1:0::repo");
        }
    } test_dep_list_k;

    struct DepListTestCaseL : DepListTestCase<'l'>
    {
        void populate_repo()
        {
            repo->add_version("l", "one", "1")->set(vmk_depend, "|| ( ( l/two l/three ) l/four )");
            repo->add_version("l", "two", "1");
            repo->add_version("l", "three", "1");
            repo->add_version("l", "four", "1");
        }

        void populate_expected()
        {
            merge_target = "l/one";
            expected.push_back("l/two-1:0::repo");
            expected.push_back("l/three-1:0::repo");
            expected.push_back("l/one-1:0::repo");
        }
    } test_dep_list_l;

    struct DepListTestCaseM : DepListTestCase<'m'>
    {
        void populate_repo()
        {
            repo->add_version("m", "one", "1")->set(vmk_depend, "m/two m/three m/four");
            repo->add_version("m", "two", "1");
            repo->add_version("m", "three", "1");
            repo->add_version("m", "four", "1")->set(vmk_depend, "|| ( ( m/two m/three ) m/five )");
        }

        void populate_expected()
        {
            merge_target = "m/one";
            expected.push_back("m/two-1:0::repo");
            expected.push_back("m/three-1:0::repo");
            expected.push_back("m/four-1:0::repo");
            expected.push_back("m/one-1:0::repo");
        }
    } test_dep_list_m;

    struct DepListTestCaseN : DepListTestCase<'n'>
    {
        void populate_repo()
        {
            repo->add_version("n", "one", "1")->set(vmk_depend, "|| ( n/two n/three )");
            repo->add_version("n", "three", "1");
        }

        void populate_expected()
        {
            merge_target = "n/one";
            expected.push_back("n/three-1:0::repo");
            expected.push_back("n/one-1:0::repo");
        }
    } test_dep_list_n;

    struct DepListTestCaseO : DepListTestCase<'o'>
    {
        void populate_repo()
        {
            repo->add_version("o", "one", "1")->set(vmk_depend, "|| ( o/two o/three )");
            repo->add_version("o", "two", "1")->set(vmk_depend, "o/four");
            repo->add_version("o", "three", "1");
        }

        void populate_expected()
        {
            merge_target = "o/one";
            expected.push_back("o/three-1:0::repo");
            expected.push_back("o/one-1:0::repo");
        }
    } test_dep_list_o;

    struct DepListTestCaseTransactionalAdd : TestCase
    {
        DepListTestCaseTransactionalAdd() : TestCase("dep list transactional add") { }

        void run()
        {
            TestEnvironment env;
            FakeRepository::Pointer repo(new FakeRepository(RepositoryName("repo")));
            env.package_database()->add_repository(repo);

            repo->add_version("cat", "one", "1")->set(vmk_depend, "cat/two cat/three");
            repo->add_version("cat", "two", "1")->set(vmk_depend, "cat/four");
            repo->add_version("cat", "three", "1")->set(vmk_depend, "cat/four cat/two");
            repo->add_version("cat", "four", "1");
            repo->add_version("cat", "five", "1")->set(vmk_depend, "cat/six cat/seven");
            repo->add_version("cat", "six", "1");
            repo->add_version("cat", "seven", "1")->set(vmk_depend, "cat/doesnotexist");

            DepList d(&env);
            d.add(DepParser::parse("cat/one"));
            TEST_CHECK_EQUAL(join(d.begin(), d.end(), " "),
                    "cat/four-1:0::repo cat/two-1:0::repo cat/three-1:0::repo cat/one-1:0::repo");

            TEST_CHECK_THROWS(d.add(DepParser::parse("cat/five")), DepListError);

            TEST_CHECK_EQUAL(join(d.begin(), d.end(), " "),
                    "cat/four-1:0::repo cat/two-1:0::repo cat/three-1:0::repo cat/one-1:0::repo");
        }
    } test_dep_list_transactional_add;

    struct DepListTestCaseTransactionalAddPost : TestCase
    {
        DepListTestCaseTransactionalAddPost() : TestCase("dep list transactional add post") { }

        void run()
        {
            TestEnvironment env;
            FakeRepository::Pointer repo(new FakeRepository(RepositoryName("repo")));
            env.package_database()->add_repository(repo);

            repo->add_version("cat", "one", "1")->set(vmk_depend, "cat/two cat/three");
            repo->add_version("cat", "two", "1")->set(vmk_depend, "cat/four");
            repo->add_version("cat", "three", "1")->set(vmk_depend, "cat/four cat/two");
            repo->add_version("cat", "four", "1");
            repo->add_version("cat", "five", "1")->set(vmk_depend, "cat/six cat/seven");
            repo->add_version("cat", "six", "1");
            repo->add_version("cat", "seven", "1")->set(vmk_pdepend, "cat/doesnotexist");

            DepList d(&env);
            d.add(DepParser::parse("cat/one"));
            TEST_CHECK_EQUAL(join(d.begin(), d.end(), " "),
                    "cat/four-1:0::repo cat/two-1:0::repo cat/three-1:0::repo cat/one-1:0::repo");

            TEST_CHECK_THROWS(d.add(DepParser::parse("cat/five")), DepListError);

            TEST_CHECK_EQUAL(join(d.begin(), d.end(), " "),
                    "cat/four-1:0::repo cat/two-1:0::repo cat/three-1:0::repo cat/one-1:0::repo");
        }
    } test_dep_list_transactional_add_post;
}

