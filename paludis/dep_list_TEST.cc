/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#include <deque>
#include <paludis/paludis.hh>
#include <string>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace paludis;
using namespace test;

namespace test_cases
{
    /**
     * Convenience base class used by many of the DepList tests.
     *
     * \ingroup Test
     */
    class DepListTestCaseBase :
        public TestCase
    {
#ifndef DOXYGEN
        protected:
            TestEnvironment env;
            FakeRepository::Pointer repo;
            std::deque<std::string> expected;
            std::string merge_target;
            bool done_populate;
#endif

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
     * \test Test DepList resolution behaviour.
     *
     * \ingroup Test
     */
    struct DepListTestCase18 : DepListTestCase<18>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->set(vmk_depend, "|| ( )");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_18;

    /**
     * \test Test DepList resolution behaviour.
     *
     * \ingroup Test
     */
    struct DepListTestCase19 : DepListTestCase<19>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->set(vmk_depend, "enabled? ( cat/two )");
            repo->add_version("cat", "two", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_19;

    /**
     * \test Test DepList resolution behaviour.
     *
     * \ingroup Test
     */
    struct DepListTestCase20 : DepListTestCase<20>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->set(vmk_depend, "!enabled? ( cat/two )");
            repo->add_version("cat", "two", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_20;

    /**
     * \test Test DepList resolution behaviour.
     *
     * \ingroup Test
     */
    struct DepListTestCase21 : DepListTestCase<21>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->set(vmk_depend, "disabled? ( cat/two )");
            repo->add_version("cat", "two", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_21;

    /**
     * \test Test DepList resolution behaviour.
     *
     * \ingroup Test
     */
    struct DepListTestCase22 : DepListTestCase<22>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->set(vmk_depend, "!disabled? ( cat/two )");
            repo->add_version("cat", "two", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_22;

    /**
     * \test Test DepList resolution behaviour.
     *
     * \ingroup Test
     */
    struct DepListTestCase23 : DepListTestCase<23>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->set(vmk_depend, "|| ( enabled? ( cat/two ) cat/three )");
            repo->add_version("cat", "two", "1");
            repo->add_version("cat", "three", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_23;

    /**
     * \test Test DepList resolution behaviour.
     *
     * \ingroup Test
     */
    struct DepListTestCase24 : DepListTestCase<24>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->set(vmk_depend, "|| ( !enabled? ( cat/two ) cat/three )");
            repo->add_version("cat", "two", "1");
            repo->add_version("cat", "three", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/three-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_24;

    /**
     * \test Test DepList resolution behaviour.
     *
     * \ingroup Test
     */
    struct DepListTestCase25 : DepListTestCase<25>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->set(vmk_depend, "|| ( disabled? ( cat/two ) cat/three )");
            repo->add_version("cat", "two", "1");
            repo->add_version("cat", "three", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/three-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_25;

    /**
     * \test Test DepList resolution behaviour.
     *
     * \ingroup Test
     */
    struct DepListTestCase26 : DepListTestCase<26>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->set(vmk_depend, "|| ( !disabled? ( cat/two ) cat/three )");
            repo->add_version("cat", "two", "1");
            repo->add_version("cat", "three", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_26;

    /**
     * \test Test DepList resolution behaviour.
     *
     * \ingroup Test
     */
    struct DepListTestCase27 : DepListTestCase<27>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->set(
                    vmk_depend, "cat/three || ( enabled? ( cat/two ) cat/three )");
            repo->add_version("cat", "two", "1");
            repo->add_version("cat", "three", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/three-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_27;

    /**
     * \test Test DepList resolution behaviour.
     *
     * \ingroup Test
     */
    struct DepListTestCase28 : DepListTestCase<28>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->set(
                    vmk_depend, "cat/three || ( !enabled? ( cat/two ) cat/three )");
            repo->add_version("cat", "two", "1");
            repo->add_version("cat", "three", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/three-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_28;

    /**
     * \test Test DepList resolution behaviour.
     *
     * \ingroup Test
     */
    struct DepListTestCase29 : DepListTestCase<29>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->set(
                    vmk_depend, "cat/three || ( disabled? ( cat/two ) cat/three )");
            repo->add_version("cat", "two", "1");
            repo->add_version("cat", "three", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/three-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_29;

    /**
     * \test Test DepList resolution behaviour.
     *
     * \ingroup Test
     */
    struct DepListTestCase30 : DepListTestCase<30>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->set(
                    vmk_depend, "cat/three || ( !disabled? ( cat/two ) cat/three )");
            repo->add_version("cat", "two", "1");
            repo->add_version("cat", "three", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/three-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_30;

    /**
     * \test Test DepList resolution behaviour.
     *
     * \ingroup Test
     */
    struct DepListTestCase31 : DepListTestCase<31>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->set(
                    vmk_depend, "cat/three || ( enabled? ( cat/three ) cat/two )");
            repo->add_version("cat", "two", "1");
            repo->add_version("cat", "three", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/three-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_31;

    /**
     * \test Test DepList resolution behaviour.
     *
     * \ingroup Test
     */
    struct DepListTestCase32 : DepListTestCase<32>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->set(
                    vmk_depend, "cat/three || ( !enabled? ( cat/three ) cat/two )");
            repo->add_version("cat", "two", "1");
            repo->add_version("cat", "three", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/three-1:0::repo");
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_32;

    /**
     * \test Test DepList resolution behaviour.
     *
     * \ingroup Test
     */
    struct DepListTestCase33 : DepListTestCase<33>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->set(
                    vmk_depend, "cat/three || ( disabled? ( cat/three ) cat/two )");
            repo->add_version("cat", "two", "1");
            repo->add_version("cat", "three", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/three-1:0::repo");
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_33;

    /**
     * \test Test DepList resolution behaviour.
     *
     * \ingroup Test
     */
    struct DepListTestCase34 : DepListTestCase<34>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->set(
                    vmk_depend, "cat/three || ( !disabled? ( cat/three ) cat/two )");
            repo->add_version("cat", "two", "1");
            repo->add_version("cat", "three", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/three-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_34;

    /**
     * \test Test DepList resolution behaviour.
     *
     * \ingroup Test
     */
    struct DepListTestCase35 : DepListTestCase<35>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->set(
                    vmk_depend, "|| ( enabled1? ( cat/two ) enabled2? ( cat/three ) )");
            repo->add_version("cat", "two", "1");
            repo->add_version("cat", "three", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_35;

    /**
     * \test Test DepList resolution behaviour.
     *
     * \ingroup Test
     */
    struct DepListTestCase36 : DepListTestCase<36>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->set(
                    vmk_depend, "|| ( !enabled1? ( cat/two ) enabled2? ( cat/three ) )");
            repo->add_version("cat", "two", "1");
            repo->add_version("cat", "three", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/three-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_36;

    /**
     * \test Test DepList resolution behaviour.
     *
     * \ingroup Test
     */
    struct DepListTestCase37 : DepListTestCase<37>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->set(
                    vmk_depend, "|| ( !enabled1? ( cat/two ) !enabled2? ( cat/three ) )");
            repo->add_version("cat", "two", "1");
            repo->add_version("cat", "three", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_37;

    /**
     * \test Test DepList resolution behaviour.
     *
     * \ingroup Test
     */
    struct DepListTestCase38 : DepListTestCase<38>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->set(vmk_depend, "!cat/two");
            repo->add_version("cat", "two", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_38;

    /**
     * \test Test DepList resolution behaviour.
     *
     * \ingroup Test
     */
    struct DepListTestCase39 : DepListTestCase<39>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->set(vmk_depend, "cat/two !cat/two");
            repo->add_version("cat", "two", "1");
        }

        void populate_expected()
        {
            merge_target="cat/one";
        }

        void check_lists()
        {
            TEST_CHECK(true);
            DepList d(&env);
            TEST_CHECK_THROWS(d.add(DepParser::parse(merge_target)), DepListError);
            TEST_CHECK(d.begin() == d.end());
        }
    } test_dep_list_39;

    /**
     * \test Test DepList resolution behaviour.
     *
     * \ingroup Test
     */
    struct DepListTestCase40 : DepListTestCase<40>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->set(vmk_depend, "cat/two cat/three");
            repo->add_version("cat", "two", "1");
            repo->add_version("cat", "three", "1")->set(vmk_depend, "!cat/two");
        }

        void populate_expected()
        {
            merge_target="cat/one";
        }

        void check_lists()
        {
            TEST_CHECK(true);
            DepList d(&env);
            TEST_CHECK_THROWS(d.add(DepParser::parse(merge_target)), DepListError);
            TEST_CHECK(d.begin() == d.end());
        }
    } test_dep_list_40;

    /**
     * \test Test DepList resolution behaviour.
     *
     * \ingroup Test
     */
    struct DepListTestCase41 : DepListTestCase<41>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->set(vmk_depend, "cat/three cat/two");
            repo->add_version("cat", "two", "1");
            repo->add_version("cat", "three", "1")->set(vmk_depend, "!cat/two");
        }

        void populate_expected()
        {
            merge_target="cat/one";
            expected.push_back("cat/three-1:0::repo");
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_41;

    /**
     * \test Test DepList resolution behaviour.
     *
     * \ingroup Test
     */
    struct DepListTestCase42 : DepListTestCase<42>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->set(vmk_depend, "|| ( cat/two cat/three )");
            repo->add_version("cat", "two", "1")->set(vmk_depend, "cat/one");
            repo->add_version("cat", "three", "1");
        }

        void populate_expected()
        {
            merge_target="cat/one";
            expected.push_back("cat/three-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_42;

    /**
     * \test Test DepList resolution behaviour.
     *
     * \ingroup Test
     */
    struct DepListTestCase43 : DepListTestCase<43>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->set(vmk_depend, "|| ( cat/two cat/three )");
            repo->add_version("cat", "two", "1")->set(vmk_rdepend, "cat/one");
            repo->add_version("cat", "three", "1");
        }

        void populate_expected()
        {
            merge_target="cat/one";
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_43;

    /**
     * \test Test DepList resolution behaviour.
     *
     * \ingroup Test
     */
    struct DepListTestCase44 : DepListTestCase<44>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->set(vmk_depend, "( cat/two cat/two )");
            repo->add_version("cat", "two", "1");
        }

        void populate_expected()
        {
            merge_target="cat/one";
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_44;

    /**
     * \test Test DepList resolution behaviour.
     *
     * \ingroup Test
     */
    struct DepListTestCase45 : DepListTestCase<45>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->set(vmk_depend, "( cat/two[enabled] )");
            repo->add_version("cat", "two", "1")->set(vmk_iuse, "enabled");
        }

        void populate_expected()
        {
            merge_target="cat/one";
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_45;

    /**
     * \test Test DepList resolution behaviour.
     *
     * \ingroup Test
     */
    struct DepListTestCase46 : DepListTestCase<46>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->set(vmk_depend, "( cat/two[-disabled] )");
            repo->add_version("cat", "two", "1")->set(vmk_iuse, "disabled");
        }

        void populate_expected()
        {
            merge_target="cat/one";
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_46;

    /**
     * \test Test DepList resolution behaviour.
     *
     * \ingroup Test
     */
    struct DepListTestCase47 : DepListTestCase<47>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->set(vmk_depend, "( cat/two[disabled] )");
            repo->add_version("cat", "two", "1")->set(vmk_iuse, "disabled");
        }

        void populate_expected()
        {
            merge_target="cat/one";
        }

        void check_lists()
        {
            TEST_CHECK(true);
            DepList d(&env);
            TEST_CHECK_THROWS(d.add(DepParser::parse(merge_target)), DepListError);
            TEST_CHECK(d.begin() == d.end());
        }
    } test_dep_list_47;

    /**
     * \test Test DepList resolution behaviour.
     *
     * \ingroup Test
     */
    struct DepListTestCase48 : DepListTestCase<48>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->set(vmk_depend, "( cat/two[-enabled] )");
            repo->add_version("cat", "two", "1")->set(vmk_iuse, "enabled");
        }

        void populate_expected()
        {
            merge_target="cat/one";
        }

        void check_lists()
        {
            TEST_CHECK(true);
            DepList d(&env);
            TEST_CHECK_THROWS(d.add(DepParser::parse(merge_target)), DepListError);
            TEST_CHECK(d.begin() == d.end());
        }
    } test_dep_list_48;

    /**
     * \test Test DepList resolution behaviour.
     *
     * \ingroup Test
     */
    struct DepListTestCase49 : DepListTestCase<49>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->set(vmk_depend, "( cat/two cat/two[enabled] )");
            repo->add_version("cat", "two", "1")->set(vmk_iuse, "enabled");
        }

        void populate_expected()
        {
            merge_target="cat/one";
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_49;

    /**
     * \test Test DepList resolution behaviour.
     *
     * \ingroup Test
     */
    struct DepListTestCase50 : DepListTestCase<50>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->set(vmk_depend, "( cat/two cat/two[-disabled] )");
            repo->add_version("cat", "two", "1")->set(vmk_iuse, "disabled");
        }

        void populate_expected()
        {
            merge_target="cat/one";
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_50;

    /**
     * \test Test DepList resolution behaviour.
     *
     * \ingroup Test
     */
    struct DepListTestCase51 : DepListTestCase<51>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->set(vmk_depend, "( cat/two cat/two[disabled] )");
            repo->add_version("cat", "two", "1")->set(vmk_iuse, "disabled");
        }

        void populate_expected()
        {
            merge_target="cat/one";
        }

        void check_lists()
        {
            TEST_CHECK(true);
            DepList d(&env);
            TEST_CHECK_THROWS(d.add(DepParser::parse(merge_target)), DepListError);
            TEST_CHECK(d.begin() == d.end());
        }
    } test_dep_list_51;

    /**
     * \test Test DepList resolution behaviour.
     *
     * \ingroup Test
     */
    struct DepListTestCase52 : DepListTestCase<52>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->set(vmk_depend, "( cat/two cat/two[-enabled] )");
            repo->add_version("cat", "two", "1")->set(vmk_iuse, "enabled");
        }

        void populate_expected()
        {
            merge_target="cat/one";
        }

        void check_lists()
        {
            TEST_CHECK(true);
            DepList d(&env);
            TEST_CHECK_THROWS(d.add(DepParser::parse(merge_target)), DepListError);
            TEST_CHECK(d.begin() == d.end());
        }
    } test_dep_list_52;

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

