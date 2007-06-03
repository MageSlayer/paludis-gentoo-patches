/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include "dep_list_TEST.hh"
#include <paludis/util/visitor-impl.hh>

using namespace paludis;
using namespace test;

namespace test_cases
{
    /**
     * Convenience sub base class used by the numbered DepList tests.
     *
     */
    template <int i_>
    struct DepListTestCase : DepListTestCaseBase
    {
        /**
         * Constructor.
         */
        DepListTestCase() :
            DepListTestCaseBase("dep list " + stringify(i_))
        {
        }
    };

    /**
     * \test Test DepList resolution behaviour.
     *
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
     */
    struct DepListTestCase2 : DepListTestCase<2>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->deps_interface->set_build_depend("cat/two");
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
     */
    struct DepListTestCase3 : DepListTestCase<3>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->deps_interface->set_build_depend("cat/two");
            repo->add_version("cat", "two", "1")->deps_interface->set_build_depend("cat/three");
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
     */
    struct DepListTestCase4 : DepListTestCase<4>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->deps_interface->set_build_depend("cat/two cat/three");
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
     */
    struct DepListTestCase5 : DepListTestCase<5>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->deps_interface->set_build_depend("cat/two cat/three");
            repo->add_version("cat", "two", "1")->deps_interface->set_build_depend("cat/three");
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
     */
    struct DepListTestCase6 : DepListTestCase<6>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->deps_interface->set_build_depend("cat/two cat/three");
            repo->add_version("cat", "two", "1");
            repo->add_version("cat", "three", "1")->deps_interface->set_build_depend("cat/two");
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
     */
    struct DepListTestCase7 : DepListTestCase<7>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->deps_interface->set_build_depend("cat/two cat/three");
            repo->add_version("cat", "two", "1")->deps_interface->set_build_depend("cat/four");
            repo->add_version("cat", "three", "1")->deps_interface->set_build_depend("cat/four");
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
     */
    struct DepListTestCase8 : DepListTestCase<8>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->deps_interface->set_build_depend("cat/two cat/three");
            repo->add_version("cat", "two", "1")->deps_interface->set_build_depend("cat/four cat/three");
            repo->add_version("cat", "three", "1")->deps_interface->set_build_depend("cat/four");
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
     */
    struct DepListTestCase9 : DepListTestCase<9>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->deps_interface->set_build_depend("cat/two cat/three");
            repo->add_version("cat", "two", "1")->deps_interface->set_build_depend("cat/four");
            repo->add_version("cat", "three", "1")->deps_interface->set_build_depend("cat/four cat/two");
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
     */
    struct DepListTestCase10 : DepListTestCase<10>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->deps_interface->set_build_depend("|| ( cat/two cat/three )");
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
     */
    struct DepListTestCase11 : DepListTestCase<11>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->deps_interface->set_build_depend("cat/two cat/three");
            repo->add_version("cat", "two", "1");
            repo->add_version("cat", "three", "1")->deps_interface->set_build_depend("|| ( cat/two cat/four )");
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
     */
    struct DepListTestCase12 : DepListTestCase<12>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->deps_interface->set_build_depend("|| ( ( cat/two cat/three ) cat/four )");
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
     */
    struct DepListTestCase13 : DepListTestCase<13>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->deps_interface->set_build_depend("cat/two cat/three cat/four");
            repo->add_version("cat", "two", "1");
            repo->add_version("cat", "three", "1");
            repo->add_version("cat", "four", "1")->deps_interface->set_build_depend("|| ( ( cat/two cat/three ) cat/five )");
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
     */
    struct DepListTestCase14 : DepListTestCase<14>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->deps_interface->set_build_depend("|| ( cat/two cat/three )");
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
     */
    struct DepListTestCase15 : DepListTestCase<15>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->deps_interface->set_build_depend("|| ( cat/two cat/three )");
            repo->add_version("cat", "two", "1")->deps_interface->set_build_depend("cat/four");
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
     */
    struct DepListTestCase16 : DepListTestCase<16>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->deps_interface->set_build_depend("cat/two:slot2");
            repo->add_version("cat", "two", "1.1")->slot = SlotName("slot1");
            repo->add_version("cat", "two", "1.2")->slot = SlotName("slot2");
            repo->add_version("cat", "two", "1.3")->slot = SlotName("slot3");
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
     */
    struct DepListTestCase17 : DepListTestCase<17>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->deps_interface->set_build_depend("<cat/two-1.2-r2:slot2");
            repo->add_version("cat", "two", "1.1")->slot = SlotName("slot1");
            repo->add_version("cat", "two", "1.2")->slot = SlotName("slot2");
            repo->add_version("cat", "two", "1.2-r1")->slot = SlotName("slot2");
            repo->add_version("cat", "two", "1.2-r2")->slot = SlotName("slot2");
            repo->add_version("cat", "two", "1.3")->slot = SlotName("slot3");
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
     */
    struct DepListTestCase18 : DepListTestCase<18>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->deps_interface->set_build_depend("|| ( )");
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
     */
    struct DepListTestCase19 : DepListTestCase<19>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->deps_interface->set_build_depend("enabled? ( cat/two )");
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
     */
    struct DepListTestCase20 : DepListTestCase<20>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->deps_interface->set_build_depend("!enabled? ( cat/two )");
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
     */
    struct DepListTestCase21 : DepListTestCase<21>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->deps_interface->set_build_depend("disabled? ( cat/two )");
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
     */
    struct DepListTestCase22 : DepListTestCase<22>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->deps_interface->set_build_depend("!disabled? ( cat/two )");
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
     */
    struct DepListTestCase23 : DepListTestCase<23>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->deps_interface->set_build_depend("|| ( enabled? ( cat/two ) cat/three )");
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
     */
    struct DepListTestCase24 : DepListTestCase<24>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->deps_interface->set_build_depend("|| ( !enabled? ( cat/two ) cat/three )");
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
     */
    struct DepListTestCase25 : DepListTestCase<25>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->deps_interface->set_build_depend("|| ( disabled? ( cat/two ) cat/three )");
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
     */
    struct DepListTestCase26 : DepListTestCase<26>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->deps_interface->set_build_depend("|| ( !disabled? ( cat/two ) cat/three )");
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
     */
    struct DepListTestCase27 : DepListTestCase<27>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->deps_interface->set_build_depend("cat/three || ( enabled? ( cat/two ) cat/three )");
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
     */
    struct DepListTestCase28 : DepListTestCase<28>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->deps_interface->set_build_depend("cat/three || ( !enabled? ( cat/two ) cat/three )");
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
     */
    struct DepListTestCase29 : DepListTestCase<29>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->deps_interface->set_build_depend("cat/three || ( disabled? ( cat/two ) cat/three )");
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
     */
    struct DepListTestCase30 : DepListTestCase<30>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->deps_interface->set_build_depend("cat/three || ( !disabled? ( cat/two ) cat/three )");
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
     */
    struct DepListTestCase31 : DepListTestCase<31>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->deps_interface->set_build_depend("cat/three || ( enabled? ( cat/three ) cat/two )");
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
     */
    struct DepListTestCase32 : DepListTestCase<32>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->deps_interface->set_build_depend("cat/three || ( !enabled? ( cat/three ) cat/two )");
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
     */
    struct DepListTestCase33 : DepListTestCase<33>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->deps_interface->set_build_depend("cat/three || ( disabled? ( cat/three ) cat/two )");
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
     */
    struct DepListTestCase34 : DepListTestCase<34>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->deps_interface->set_build_depend("cat/three || ( !disabled? ( cat/three ) cat/two )");
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
     */
    struct DepListTestCase35 : DepListTestCase<35>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->deps_interface->set_build_depend("|| ( enabled1? ( cat/two ) enabled2? ( cat/three ) )");
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
     */
    struct DepListTestCase36 : DepListTestCase<36>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->deps_interface->set_build_depend("|| ( !enabled1? ( cat/two ) enabled2? ( cat/three ) )");
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
     */
    struct DepListTestCase37 : DepListTestCase<37>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->deps_interface->set_build_depend("|| ( !enabled1? ( cat/two ) !enabled2? ( cat/three ) )");
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
     */
    struct DepListTestCase42 : DepListTestCase<42>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->deps_interface->set_build_depend("|| ( cat/two cat/three )");
            repo->add_version("cat", "two", "1")->deps_interface->set_build_depend("cat/one");
            repo->add_version("cat", "three", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/three-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_42;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase43 : DepListTestCase<43>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->deps_interface->set_build_depend("|| ( cat/two cat/three )");
            repo->add_version("cat", "two", "1")->deps_interface->set_post_depend("cat/one");
            repo->add_version("cat", "three", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_43;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase44 : DepListTestCase<44>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->deps_interface->set_build_depend("( cat/two cat/two )");
            repo->add_version("cat", "two", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_44;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase45 : DepListTestCase<45>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->deps_interface->set_build_depend("( cat/two[enabled] )");
            repo->add_version("cat", "two", "1")->ebuild_interface->set_iuse("enabled");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_45;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase46 : DepListTestCase<46>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->deps_interface->set_build_depend("( cat/two[-disabled] )");
            repo->add_version("cat", "two", "1")->ebuild_interface->set_iuse("disabled");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_46;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase47 : DepListTestCase<47>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->deps_interface->set_build_depend("( cat/two[disabled] )");
            repo->add_version("cat", "two", "1")->ebuild_interface->set_iuse("disabled");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
        }

        void check_lists()
        {
            TEST_CHECK(true);
            DepList d(&env, DepListOptions());
            TEST_CHECK_THROWS(d.add(PackageDepSpec(merge_target, pds_pm_permissive),
                        env.default_destinations()), DepListError);
            TEST_CHECK(d.begin() == d.end());
        }
    } test_dep_list_47;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase48 : DepListTestCase<48>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->deps_interface->set_build_depend("( cat/two[-enabled] )");
            repo->add_version("cat", "two", "1")->ebuild_interface->set_iuse("enabled");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
        }

        void check_lists()
        {
            TEST_CHECK(true);
            DepList d(&env, DepListOptions());
            TEST_CHECK_THROWS(d.add(PackageDepSpec(merge_target, pds_pm_permissive),
                        env.default_destinations()), DepListError);
            TEST_CHECK(d.begin() == d.end());
        }
    } test_dep_list_48;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase49 : DepListTestCase<49>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->deps_interface->set_build_depend("( cat/two cat/two[enabled] )");
            repo->add_version("cat", "two", "1")->ebuild_interface->set_iuse("enabled");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_49;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase50 : DepListTestCase<50>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->deps_interface->set_build_depend("( cat/two cat/two[-disabled] )");
            repo->add_version("cat", "two", "1")->ebuild_interface->set_iuse("disabled");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_50;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase51 : DepListTestCase<51>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->deps_interface->set_build_depend("( cat/two cat/two[disabled] )");
            repo->add_version("cat", "two", "1")->ebuild_interface->set_iuse("disabled");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
        }

        void check_lists()
        {
            TEST_CHECK(true);
            DepList d(&env, DepListOptions());
            TEST_CHECK_THROWS(d.add(PackageDepSpec(merge_target, pds_pm_permissive),
                        env.default_destinations()), DepListError);
            TEST_CHECK(d.begin() == d.end());
        }
    } test_dep_list_51;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase52 : DepListTestCase<52>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->deps_interface->set_build_depend("( cat/two cat/two[-enabled] )");
            repo->add_version("cat", "two", "1")->ebuild_interface->set_iuse("enabled");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
        }

        void check_lists()
        {
            TEST_CHECK(true);
            DepList d(&env, DepListOptions());
            TEST_CHECK_THROWS(d.add(PackageDepSpec(merge_target, pds_pm_permissive),
                        env.default_destinations()), DepListError);
            TEST_CHECK(d.begin() == d.end());
        }
    } test_dep_list_52;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase53 : DepListTestCase<53>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->deps_interface->set_post_depend("cat/two");
            repo->add_version("cat", "two", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/one-1:0::repo");
            expected.push_back("cat/two-1:0::repo");
        }
    } test_dep_list_53;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase54 : DepListTestCase<54>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->deps_interface->set_post_depend("cat/two");
            repo->add_version("cat", "two", "1")->deps_interface->set_build_depend("cat/three");
            repo->add_version("cat", "three", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/one-1:0::repo");
            expected.push_back("cat/three-1:0::repo");
            expected.push_back("cat/two-1:0::repo");
        }
    } test_dep_list_54;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase55 : DepListTestCase<55>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->deps_interface->set_post_depend("cat/two");
            repo->add_version("cat", "two", "1")->deps_interface->set_build_depend("cat/one");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/one-1:0::repo");
            expected.push_back("cat/two-1:0::repo");
        }
    } test_dep_list_55;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase56 : DepListTestCase<56>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->deps_interface->set_build_depend("cat/two || ( cat/three cat/four )");
            repo->add_version("cat", "two", "1")->ebuild_interface->set_provide("cat/four");
            repo->add_version("cat", "three", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/four-1:0::virtuals");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_56;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase57 : DepListTestCase<57>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->deps_interface->set_post_depend("cat/two");
            repo->add_version("cat", "two", "1")->deps_interface->set_build_depend("cat/three");
            repo->add_version("cat", "three", "1")->deps_interface->set_build_depend("cat/one");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/one-1:0::repo");
            expected.push_back("cat/three-1:0::repo");
            expected.push_back("cat/two-1:0::repo");
        }
    } test_dep_list_57;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase58 : DepListTestCase<58>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->ebuild_interface->set_provide("cat/two");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/one-1:0::repo");
            expected.push_back("cat/two-1:0::virtuals");
        }
    } test_dep_list_58;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase59 : DepListTestCase<59>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->deps_interface->set_build_depend("|| ( cat/two >=cat/three-2 )");
            repo->add_version("cat", "two", "1");
            repo->add_version("cat", "three", "2");
            installed_repo->add_version("cat", "three", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/three-2:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_59;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase60 : DepListTestCase<60>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->deps_interface->set_build_depend("|| ( cat/two >=cat/three-3 )");
            repo->add_version("cat", "two", "1");
            repo->add_version("cat", "three", "2");
            installed_repo->add_version("cat", "three", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_60;

    struct DepListTestCase61 : DepListTestCase<61>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->ebuild_interface->set_provide("virtual/foo");
            tr1::shared_ptr<VersionMetadata> m(repo->add_version("cat", "two", "2"));
            m->ebuild_interface->set_provide("virtual/foo");
            m->deps_interface->set_build_depend("cat/one");
        }

        void populate_expected()
        {
            merge_target = "cat/two";
            expected.push_back("cat/one-1:0::repo");
            expected.push_back("virtual/foo-1:0::virtuals");
            expected.push_back("cat/two-2:0::repo");
            expected.push_back("virtual/foo-2:0::virtuals");
        }
    } test_dep_list_61;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase62 : DepListTestCase<62>
    {
        void set_options(DepListOptions & opts)
        {
            opts.override_masks += dl_override_tilde_keywords;
        }

        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->deps_interface->set_build_depend("|| ( cat/two cat/three )");
            repo->add_version("cat", "two", "1")->ebuild_interface->set_keywords("~test");
            repo->add_version("cat", "three", "2")->ebuild_interface->set_keywords("~test");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/two-1:0::repo(M)");
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_62;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase63 : DepListTestCase<63>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->deps_interface->set_build_depend("|| ( =cat/two-1 =cat/two-2 )");
            repo->add_version("cat", "two", "1");
            repo->add_version("cat", "two", "2");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/two-2:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_63;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase64 : DepListTestCase<64>
    {
        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->deps_interface->set_build_depend("|| ( =cat/two-1 =cat/two-2 )");
            repo->add_version("cat", "two", "1");
            repo->add_version("cat", "two", "2");
            installed_repo->add_version("cat", "two", "1");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/two-2:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_64;

    /**
     * \test Test DepList resolution behaviour.
     *
     */
    struct DepListTestCase65 : DepListTestCase<65>
    {
        void set_options(DepListOptions & opts)
        {
            opts.override_masks += dl_override_tilde_keywords;
        }

        void populate_repo()
        {
            repo->add_version("cat", "one", "1")->deps_interface->set_build_depend("|| ( =cat/two-2 =cat/two-1 )");
            repo->add_version("cat", "two", "1")->ebuild_interface->set_keywords("~test");
        }

        void populate_expected()
        {
            merge_target = "cat/one";
            expected.push_back("cat/two-1:0::repo(M)");
            expected.push_back("cat/two-1:0::repo");
            expected.push_back("cat/one-1:0::repo");
        }
    } test_dep_list_65;

    /**
     * \test Test DepList transactional add behaviour.
     *
     */
    struct DepListTestCaseTransactionalAdd : TestCase
    {
        DepListTestCaseTransactionalAdd() : TestCase("dep list transactional add") { }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<FakeRepository> repo(new FakeRepository(&env, RepositoryName("repo")));
            env.package_database()->add_repository(1, repo);
            tr1::shared_ptr<FakeInstalledRepository> destination_repo(new FakeInstalledRepository(&env,
                        RepositoryName("installed_repo")));
            env.package_database()->add_repository(2, destination_repo);

            repo->add_version("cat", "one", "1")->deps_interface->set_build_depend("cat/two cat/three");
            repo->add_version("cat", "two", "1")->deps_interface->set_build_depend("cat/four");
            repo->add_version("cat", "three", "1")->deps_interface->set_build_depend("cat/four cat/two");
            repo->add_version("cat", "four", "1");
            repo->add_version("cat", "five", "1")->deps_interface->set_build_depend("cat/six cat/seven");
            repo->add_version("cat", "six", "1");
            repo->add_version("cat", "seven", "1")->deps_interface->set_build_depend("cat/doesnotexist");

            DepList d(&env, DepListOptions());
            d.add(PackageDepSpec("cat/one", pds_pm_permissive), env.default_destinations());
            TEST_CHECK_EQUAL(join(d.begin(), d.end(), " "),
                    "cat/four-1:0::repo cat/two-1:0::repo cat/three-1:0::repo cat/one-1:0::repo");

            TEST_CHECK_THROWS(d.add(PackageDepSpec("cat/five", pds_pm_permissive),
                        env.default_destinations()), DepListError);

            TEST_CHECK_EQUAL(join(d.begin(), d.end(), " "),
                    "cat/four-1:0::repo cat/two-1:0::repo cat/three-1:0::repo cat/one-1:0::repo");
        }
    } test_dep_list_transactional_add;

    /**
     * \test Test DepList transactional add behaviour on PDEPENDs.
     *
     */
    struct DepListTestCaseTransactionalAddPost : TestCase
    {
        DepListTestCaseTransactionalAddPost() : TestCase("dep list transactional add post") { }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<FakeRepository> repo(new FakeRepository(&env, RepositoryName("repo")));
            env.package_database()->add_repository(1, repo);
            tr1::shared_ptr<FakeInstalledRepository> destination_repo(new FakeInstalledRepository(&env,
                        RepositoryName("installed_repo")));
            env.package_database()->add_repository(2, destination_repo);

            repo->add_version("cat", "one", "1")->deps_interface->set_build_depend("cat/two cat/three");
            repo->add_version("cat", "two", "1")->deps_interface->set_build_depend("cat/four");
            repo->add_version("cat", "three", "1")->deps_interface->set_build_depend("cat/four cat/two");
            repo->add_version("cat", "four", "1");
            repo->add_version("cat", "five", "1")->deps_interface->set_build_depend("cat/six cat/seven");
            repo->add_version("cat", "six", "1");
            repo->add_version("cat", "seven", "1")->deps_interface->set_post_depend("cat/doesnotexist");

            DepList d(&env, DepListOptions());
            d.add(PackageDepSpec("cat/one", pds_pm_permissive), env.default_destinations());
            TEST_CHECK_EQUAL(join(d.begin(), d.end(), " "),
                    "cat/four-1:0::repo cat/two-1:0::repo cat/three-1:0::repo cat/one-1:0::repo");

            TEST_CHECK_THROWS(d.add(PackageDepSpec("cat/five", pds_pm_permissive),
                        env.default_destinations()), DepListError);

            TEST_CHECK_EQUAL(join(d.begin(), d.end(), " "),
                    "cat/four-1:0::repo cat/two-1:0::repo cat/three-1:0::repo cat/one-1:0::repo");
        }
    } test_dep_list_transactional_add_post;

    /**
     * \test Test DepList transactional forced downgrade of installed package behaviour.
     *
     */
    struct DepListTestCaseForcedDowngradeOfInstalled : TestCase
    {
        DepListTestCaseForcedDowngradeOfInstalled() : TestCase("dep list forced downgrade of installed") { }

        void run()
        {
            TestEnvironment env;

            tr1::shared_ptr<FakeRepository> repo(new FakeRepository(&env, RepositoryName("repo")));
            env.package_database()->add_repository(1, repo);
            repo->add_version("cat", "one", "1");

            tr1::shared_ptr<FakeInstalledRepository> installed_repo(
                    new FakeInstalledRepository(&env, RepositoryName("installed_repo")));
            env.package_database()->add_repository(2, installed_repo);
            installed_repo->add_version("cat", "one", "2");

            DepList d(&env, DepListOptions());
            d.add(PackageDepSpec("cat/one", pds_pm_permissive), env.default_destinations());
            TEST_CHECK_EQUAL(join(d.begin(), d.end(), " "), "cat/one-1:0::repo");
        }
    } test_dep_list_forced_downgrade_of_installed;

    /**
     * \test Test DepList fall back never.
     */
    struct DepListTestCaseFallBackNever : TestCase
    {
        DepListTestCaseFallBackNever() : TestCase("dep list fall back never") { }

        void run()
        {
            TestEnvironment env;

            tr1::shared_ptr<FakeRepository> repo(new FakeRepository(&env, RepositoryName("repo")));
            env.package_database()->add_repository(1, repo);
            repo->add_version("cat", "one", "1")->deps_interface->set_build_depend("cat/two");

            tr1::shared_ptr<FakeInstalledRepository> installed_repo(
                    new FakeInstalledRepository(&env, RepositoryName("installed_repo")));
            env.package_database()->add_repository(2, installed_repo);
            installed_repo->add_version("cat", "two", "2");

            DepList d(&env, DepListOptions());
            d.options()->fall_back = dl_fall_back_never;
            TEST_CHECK_THROWS(d.add(PackageDepSpec("cat/one", pds_pm_permissive),
                        env.default_destinations()), DepListError);
        }
    } test_dep_list_fall_back_never;

    /**
     * \test Test DepList fall back as needed.
     */
    struct DepListTestCaseFallBackAsNeeded : TestCase
    {
        DepListTestCaseFallBackAsNeeded() : TestCase("dep list fall back as needed") { }

        void run()
        {
            TestEnvironment env;

            tr1::shared_ptr<FakeRepository> repo(new FakeRepository(&env, RepositoryName("repo")));
            env.package_database()->add_repository(1, repo);
            repo->add_version("cat", "one", "1")->deps_interface->set_build_depend("cat/two");

            tr1::shared_ptr<FakeInstalledRepository> installed_repo(
                    new FakeInstalledRepository(&env, RepositoryName("installed_repo")));
            env.package_database()->add_repository(2, installed_repo);
            installed_repo->add_version("cat", "two", "2");

            DepList d(&env, DepListOptions());
            d.options()->fall_back = dl_fall_back_as_needed;
            d.add(PackageDepSpec("cat/one", pds_pm_permissive), env.default_destinations());
            d.add(PackageDepSpec("cat/two", pds_pm_permissive), env.default_destinations());
            TEST_CHECK_EQUAL(join(d.begin(), d.end(), " "), "cat/two-2:0::installed_repo cat/one-1:0::repo");
        }
    } test_dep_list_fall_back_as_needed;

    /**
     * \test Test DepList fall back as needed.
     */
    struct DepListTestCaseFallBackAsNeededNotTargets : TestCase
    {
        DepListTestCaseFallBackAsNeededNotTargets() : TestCase("dep list fall back as needed not targets") { }

        void run()
        {
            TestEnvironment env;

            tr1::shared_ptr<FakeRepository> repo(new FakeRepository(&env, RepositoryName("repo")));
            env.package_database()->add_repository(1, repo);
            repo->add_version("cat", "one", "1")->deps_interface->set_build_depend("cat/two");

            tr1::shared_ptr<FakeInstalledRepository> installed_repo(
                    new FakeInstalledRepository(&env, RepositoryName("installed_repo")));
            env.package_database()->add_repository(2, installed_repo);
            installed_repo->add_version("cat", "two", "2");
            installed_repo->add_version("cat", "three", "3");

            DepList d1(&env, DepListOptions());
            d1.options()->fall_back = dl_fall_back_as_needed_except_targets;
            d1.add(PackageDepSpec("cat/one", pds_pm_permissive), env.default_destinations());
            TEST_CHECK_EQUAL(join(d1.begin(), d1.end(), " "), "cat/two-2:0::installed_repo cat/one-1:0::repo");
            TEST_CHECK_THROWS(d1.add(PackageDepSpec("cat/three", pds_pm_permissive),
                        env.default_destinations()), DepListError);

            DepList d2(&env, DepListOptions());
            d2.options()->fall_back = dl_fall_back_as_needed_except_targets;
            TEST_CHECK_THROWS(d2.add(PackageDepSpec("cat/two", pds_pm_permissive),
                        env.default_destinations()), DepListError);

            DepList d3(&env, DepListOptions());
            d3.options()->fall_back = dl_fall_back_as_needed_except_targets;
            tr1::shared_ptr<ConstTreeSequence<SetSpecTree, AllDepSpec> > t3(new ConstTreeSequence<SetSpecTree, AllDepSpec>(
                        tr1::shared_ptr<AllDepSpec>(new AllDepSpec)));
            t3->add(tr1::shared_ptr<TreeLeaf<SetSpecTree, PackageDepSpec> >(new TreeLeaf<SetSpecTree, PackageDepSpec>(
                            tr1::shared_ptr<PackageDepSpec>(new PackageDepSpec("cat/one", pds_pm_permissive)))));
            t3->add(tr1::shared_ptr<TreeLeaf<SetSpecTree, PackageDepSpec> >(new TreeLeaf<SetSpecTree, PackageDepSpec>(
                            tr1::shared_ptr<PackageDepSpec>(new PackageDepSpec("cat/two", pds_pm_permissive)))));
            TEST_CHECK_THROWS(d3.add(*t3, env.default_destinations()), DepListError);

            DepList d4(&env, DepListOptions());
            tr1::shared_ptr<ConstTreeSequence<SetSpecTree, AllDepSpec> > t4(new ConstTreeSequence<SetSpecTree, AllDepSpec>(
                    tr1::shared_ptr<AllDepSpec>(new AllDepSpec)));
            t4->add(tr1::shared_ptr<TreeLeaf<SetSpecTree, PackageDepSpec> >(new TreeLeaf<SetSpecTree, PackageDepSpec>(
                            tr1::shared_ptr<PackageDepSpec>(new PackageDepSpec("cat/one", pds_pm_permissive)))));
            t4->add(tr1::shared_ptr<TreeLeaf<SetSpecTree, PackageDepSpec> >(new TreeLeaf<SetSpecTree, PackageDepSpec>(
                            tr1::shared_ptr<PackageDepSpec>(new PackageDepSpec("cat/three", pds_pm_permissive)))));
            TEST_CHECK_THROWS(d4.add(*t4, env.default_destinations()), DepListError);
        }
    } test_dep_list_fall_back_as_needed_not_targets;

    /**
     * \test Test DepList upgrade as needed.
     */
    struct DepListTestCaseUpgradeAsNeeded : TestCase
    {
        DepListTestCaseUpgradeAsNeeded() : TestCase("dep list upgrade as needed") { }

        void run()
        {
            TestEnvironment env;

            tr1::shared_ptr<FakeRepository> repo(new FakeRepository(&env, RepositoryName("repo")));
            env.package_database()->add_repository(1, repo);
            repo->add_version("cat", "one", "1")->deps_interface->set_build_depend("cat/two");
            repo->add_version("cat", "two", "2");

            tr1::shared_ptr<FakeInstalledRepository> installed_repo(
                    new FakeInstalledRepository(&env, RepositoryName("installed_repo")));
            env.package_database()->add_repository(2, installed_repo);
            installed_repo->add_version("cat", "two", "0");

            DepList d1(&env, DepListOptions());
            d1.options()->upgrade = dl_upgrade_as_needed;
            d1.add(PackageDepSpec("cat/one", pds_pm_permissive), env.default_destinations());
            TEST_CHECK_EQUAL(join(d1.begin(), d1.end(), " "), "cat/two-0:0::installed_repo cat/one-1:0::repo");

            DepList d2(&env, DepListOptions());
            d2.options()->upgrade = dl_upgrade_as_needed;

            tr1::shared_ptr<ConstTreeSequence<SetSpecTree, AllDepSpec> > t2(new ConstTreeSequence<SetSpecTree, AllDepSpec>(
                        tr1::shared_ptr<AllDepSpec>(new AllDepSpec)));
            t2->add(tr1::shared_ptr<TreeLeaf<SetSpecTree, PackageDepSpec> >(new TreeLeaf<SetSpecTree, PackageDepSpec>(
                            tr1::shared_ptr<PackageDepSpec>(new PackageDepSpec("cat/one", pds_pm_permissive)))));
            t2->add(tr1::shared_ptr<TreeLeaf<SetSpecTree, PackageDepSpec> >(new TreeLeaf<SetSpecTree, PackageDepSpec>(
                            tr1::shared_ptr<PackageDepSpec>(new PackageDepSpec("cat/two", pds_pm_permissive)))));
            d2.add(*t2, env.default_destinations());
            TEST_CHECK_EQUAL(join(d2.begin(), d2.end(), " "), "cat/two-2:0::repo cat/one-1:0::repo");
        }
    } test_dep_list_upgrade_as_needed;

    /**
     * \test Test DepList reinstall scm.
     */
    struct DepListTestCaseReinstallSCM : TestCase
    {
        DepListTestCaseReinstallSCM() : TestCase("dep list reinstall scm") { }

        void run()
        {
            TestEnvironment env;

            tr1::shared_ptr<FakeRepository> repo(new FakeRepository(&env, RepositoryName("repo")));
            env.package_database()->add_repository(1, repo);
            repo->add_version("cat", "zero", "1")->deps_interface->set_build_depend(
                "( cat/one cat/two cat/three-live cat/four-cvs cat/five-svn cat/six-darcs )");
            repo->add_version("cat", "one", "scm");
            repo->add_version("cat", "two", "2");
            repo->add_version("cat", "three-live", "0");
            repo->add_version("cat", "four-cvs", "0");
            repo->add_version("cat", "five-svn", "0");
            repo->add_version("cat", "six-darcs", "0");

            tr1::shared_ptr<FakeInstalledRepository> installed_repo(
                    new FakeInstalledRepository(&env, RepositoryName("installed_repo")));
            env.package_database()->add_repository(2, installed_repo);
            installed_repo->add_version("cat", "one", "scm");
            installed_repo->add_version("cat", "two", "2");
            installed_repo->add_version("cat", "three-live", "0");
            installed_repo->add_version("cat", "four-cvs", "0");
            installed_repo->add_version("cat", "five-svn", "0");
            installed_repo->add_version("cat", "six-darcs", "0");

            DepList d1(&env, DepListOptions());
            d1.options()->reinstall_scm = dl_reinstall_scm_always;
            d1.add(PackageDepSpec("cat/zero", pds_pm_permissive), env.default_destinations());
            TEST_CHECK_EQUAL(join(d1.begin(), d1.end(), " "), "cat/one-scm:0::repo cat/two-2:0::installed_repo "
                    "cat/three-live-0:0::repo cat/four-cvs-0:0::repo cat/five-svn-0:0::repo cat/six-darcs-0:0::repo "
                    "cat/zero-1:0::repo");
        }
    } test_dep_list_upgrade_reinstall_scm;
}

