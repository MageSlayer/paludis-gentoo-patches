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

#include <algorithm>
#include <paludis/util/container_entry.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace test;
using namespace paludis;

namespace test_cases
{
    /**
     * \test Test ContainerEntry on a list.
     *
     * \ingroup Test
     */
    struct ContainerEntryListTest : TestCase
    {
        ContainerEntryListTest() : TestCase("list") { }

        void run()
        {
            std::list<int> list;
            TEST_CHECK(list.empty());
            {
                ContainerEntry<std::list<int> > e1(&list, 5);
                TEST_CHECK(list.begin() != list.end());
                TEST_CHECK_EQUAL(std::distance(list.begin(), list.end()), 1);
                TEST_CHECK(list.end() != std::find(list.begin(), list.end(), 5));

                {
                    ContainerEntry<std::list<int> > e2(&list, 4);
                    TEST_CHECK(list.begin() != list.end());
                    TEST_CHECK_EQUAL(std::distance(list.begin(), list.end()), 2);
                    TEST_CHECK(list.end() != std::find(list.begin(), list.end(), 5));
                    TEST_CHECK(list.end() != std::find(list.begin(), list.end(), 4));
                }

                TEST_CHECK(list.begin() != list.end());
                TEST_CHECK_EQUAL(std::distance(list.begin(), list.end()), 1);
                TEST_CHECK(list.end() != std::find(list.begin(), list.end(), 5));
            }
            TEST_CHECK(list.empty());
        }
    } test_container_entry_list;
}

