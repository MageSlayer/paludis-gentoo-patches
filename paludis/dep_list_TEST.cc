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
    /**
     * Convenience base class used by many of the DepList tests.
     *
     * \ingroup Test
     */
    class DepListTestCaseBase : TestCase
    {
        protected:
            TestEnvironment env;
            FakeRepository::Pointer repo;
            std::deque<std::string> expected;
            std::string merge_target;
            bool done_populate;

            /**
             * Constructor.
             */
            DepListTestCaseBase(const int i) :
                TestCase("dep list " + stringify(i)),
                env(),
                repo(new FakeRepository(RepositoryName("repo"))),
                done_populate(false)
            {
                env.package_database()->add_repository(repo);
            }

            /**
             * Populate our repo member.
             */
            virtual void populate_repo() = 0;

            /**
             * Populate our expected member.
             */
            virtual void populate_expected() = 0;

            /**
             * Check expected is what we got.
             */
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

        public:
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
    };

    /**
     * Convenience sub base class used by the numbered DepList tests.
     *
     * \ingroup Test
     */
    template <int i_>
    struct DepListTestCase : DepListTestCaseBase
    {
        /**
         * Constructor.
         */
        DepListTestCase() :
            DepListTestCaseBase(i_)
        {
        }
    };

    /**
     * \test Test DepList resolution behaviour.
     *
     * \ingroup Test
     */
    struct DepListTestCase1 : DepListTestCase<1>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_1;

    /**
     * \test Test DepList resolution behaviour.
     *
     * \ingroup Test
     */
    struct DepListTestCase2 : DepListTestCase<2>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->set(vmk_depend, "cat/two");
            repo->add_version("cat", "two", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_2;

    /**
     * \test Test DepList resolution behaviour.
     *
     * \ingroup Test
     */
    struct DepListTestCase3 : DepListTestCase<3>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->set(vmk_depend, "cat/two");
            repo->add_version("cat", "two", "1")->set(vmk_depend, "cat/three");
            repo->add_version("cat", "three", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/three-1:0::repo");
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_3;

    /**
     * \test Test DepList resolution behaviour.
     *
     * \ingroup Test
     */
    struct DepListTestCase4 : DepListTestCase<4>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->set(vmk_depend, "cat/two cat/three");
            repo->add_version("cat", "two", "1");
            repo->add_version("cat", "three", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/three-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_4;

    /**
     * \test Test DepList resolution behaviour.
     *
     * \ingroup Test
     */
    struct DepListTestCase5 : DepListTestCase<5>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->set(vmk_depend, "cat/two cat/three");
            repo->add_version("cat", "two", "1")->set(vmk_depend, "cat/three");
            repo->add_version("cat", "three", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/three-1:0::repo");
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_5;

    /**
     * \test Test DepList resolution behaviour.
     *
     * \ingroup Test
     */
    struct DepListTestCase6 : DepListTestCase<6>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->set(vmk_depend, "cat/two cat/three");
            repo->add_version("cat", "two", "1");
            repo->add_version("cat", "three", "1")->set(vmk_depend, "cat/two");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/three-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_6;

    /**
     * \test Test DepList resolution behaviour.
     *
     * \ingroup Test
     */
    struct DepListTestCase7 : DepListTestCase<7>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->set(vmk_depend, "cat/two cat/three");
            repo->add_version("cat", "two", "1")->set(vmk_depend, "cat/four");
            repo->add_version("cat", "three", "1")->set(vmk_depend, "cat/four");
            repo->add_version("cat", "four", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/four-1:0::repo");
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/three-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_7;

    /**
     * \test Test DepList resolution behaviour.
     *
     * \ingroup Test
     */
    struct DepListTestCase8 : DepListTestCase<8>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->set(vmk_depend, "cat/two cat/three");
            repo->add_version("cat", "two", "1")->set(vmk_depend, "cat/four cat/three");
            repo->add_version("cat", "three", "1")->set(vmk_depend, "cat/four");
            repo->add_version("cat", "four", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/four-1:0::repo");
            expected.push_back("cat/three-1:0::repo");
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_8;

    /**
     * \test Test DepList resolution behaviour.
     *
     * \ingroup Test
     */
    struct DepListTestCase9 : DepListTestCase<9>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->set(vmk_depend, "cat/two cat/three");
            repo->add_version("cat", "two", "1")->set(vmk_depend, "cat/four");
            repo->add_version("cat", "three", "1")->set(vmk_depend, "cat/four cat/two");
            repo->add_version("cat", "four", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/four-1:0::repo");
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/three-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_9;

    /**
     * \test Test DepList resolution behaviour.
     *
     * \ingroup Test
     */
    struct DepListTestCase10 : DepListTestCase<10>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->set(vmk_depend, "|| ( cat/two cat/three )");
            repo->add_version("cat", "two", "1");
            repo->add_version("cat", "three", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_10;

    /**
     * \test Test DepList resolution behaviour.
     *
     * \ingroup Test
     */
    struct DepListTestCase11 : DepListTestCase<11>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->set(vmk_depend, "cat/two cat/three");
            repo->add_version("cat", "two", "1");
            repo->add_version("cat", "three", "1")->set(vmk_depend, "|| ( cat/two cat/four )");
            repo->add_version("cat", "four", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/three-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_11;

    /**
     * \test Test DepList resolution behaviour.
     *
     * \ingroup Test
     */
    struct DepListTestCase12 : DepListTestCase<12>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->set(vmk_depend, "|| ( ( cat/two cat/three ) cat/four )");
            repo->add_version("cat", "two", "1");
            repo->add_version("cat", "three", "1");
            repo->add_version("cat", "four", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/three-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_12;

    /**
     * \test Test DepList resolution behaviour.
     *
     * \ingroup Test
     */
    struct DepListTestCase13 : DepListTestCase<13>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->set(vmk_depend, "cat/two cat/three cat/four");
            repo->add_version("cat", "two", "1");
            repo->add_version("cat", "three", "1");
            repo->add_version("cat", "four", "1")->set(vmk_depend, "|| ( ( cat/two cat/three ) cat/five )");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/three-1:0::repo");
            expected.push_back("cat/four-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_13;

    /**
     * \test Test DepList resolution behaviour.
     *
     * \ingroup Test
     */
    struct DepListTestCase14 : DepListTestCase<14>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->set(vmk_depend, "|| ( cat/two cat/three )");
            repo->add_version("cat", "three", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/three-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_14;

    /**
     * \test Test DepList resolution behaviour.
     *
     * \ingroup Test
     */
    struct DepListTestCase15 : DepListTestCase<15>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->set(vmk_depend, "|| ( cat/two cat/three )");
            repo->add_version("cat", "two", "1")->set(vmk_depend, "cat/four");
            repo->add_version("cat", "three", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/three-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_15;

    /**
     * \test Test DepList resolution behaviour.
     *
     * \ingroup Test
     */
    struct DepListTestCase16 : DepListTestCase<16>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->set(vmk_depend, "cat/two:slot2");
            repo->add_version("cat", "two", "1.1")->set(vmk_slot, "slot1");
            repo->add_version("cat", "two", "1.2")->set(vmk_slot, "slot2");
            repo->add_version("cat", "two", "1.3")->set(vmk_slot, "slot3");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/two-1.2:slot2::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_16;

    /**
     * \test Test DepList resolution behaviour.
     *
     * \ingroup Test
     */
    struct DepListTestCase17 : DepListTestCase<17>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->set(vmk_depend, "<cat/two-1.2-r2:slot2");
            repo->add_version("cat", "two", "1.1")->set(vmk_slot, "slot1");
            repo->add_version("cat", "two", "1.2")->set(vmk_slot, "slot2");
            repo->add_version("cat", "two", "1.2-r1")->set(vmk_slot, "slot2");
            repo->add_version("cat", "two", "1.2-r2")->set(vmk_slot, "slot2");
            repo->add_version("cat", "two", "1.3")->set(vmk_slot, "slot3");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/two-1.2-r1:slot2::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_17;

    /**
     * \test Test DepList transactional add behaviour.
     *
     * \ingroup Test
     */
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

    /**
     * \test Test DepList transactional add behaviour on PDEPENDs.
     *
     * \ingroup Test
     */
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

