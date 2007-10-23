/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#include "fast_unique_copy.hh"
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <paludis/util/join.hh>
#include <vector>

using namespace test;
using namespace paludis;

namespace test_cases
{
    struct FastUniqueCopySimpleSequenceTest : TestCase
    {
        FastUniqueCopySimpleSequenceTest() : TestCase("fast_unique_copy simple sequence") { }

        void run()
        {
            for (unsigned sz = 0 ; sz < 20 ; ++sz)
            {
                TestMessageSuffix s("sz=" + stringify(sz));
                std::vector<unsigned> v;
                for (unsigned x = 0 ; x < sz ; ++x)
                    v.push_back(x);

                std::vector<unsigned> r;
                fast_unique_copy(v.begin(), v.end(), std::back_inserter(r), std::less<int>());

                TestMessageSuffix vs("v=" + join(v.begin(), v.end(), ","));
                TestMessageSuffix rs("r=" + join(r.begin(), r.end(), ","));

                TEST_CHECK_EQUAL(r.size(), sz);
                for (unsigned x = 0 ; x < sz ; ++x)
                    TEST_CHECK_EQUAL(r[x], x);
            }
        }

    } test_fast_unique_copy_simple_sequence;

    struct FastUniqueCopyRepeatedElementTest : TestCase
    {
        FastUniqueCopyRepeatedElementTest() : TestCase("fast_unique_copy single repeated element") { }

        void run()
        {
            for (unsigned sz = 0 ; sz < 20 ; ++sz)
            {
                TestMessageSuffix s("sz=" + stringify(sz));
                std::vector<unsigned> v;
                for (unsigned x = 0 ; x < sz ; ++x)
                    v.push_back(42);

                std::vector<unsigned> r;
                fast_unique_copy(v.begin(), v.end(), std::back_inserter(r));

                TestMessageSuffix vs("v=" + join(v.begin(), v.end(), ","));
                TestMessageSuffix rs("r=" + join(r.begin(), r.end(), ","));

                if (sz == 0)
                    TEST_CHECK_EQUAL(r.size(), std::size_t(0));
                else
                {
                    TEST_CHECK_EQUAL(r.size(), std::size_t(1));
                    TEST_CHECK_EQUAL(r[0], 42u);
                }
            }
        }

    } test_fast_unique_copy_repeated_element;

    struct FastUniqueCopyNxNTest : TestCase
    {
        FastUniqueCopyNxNTest() : TestCase("fast_unique_copy nxn") { }

        void run()
        {
            for (unsigned sz = 0 ; sz < 20 ; ++sz)
            {
                TestMessageSuffix s("sz=" + stringify(sz));
                std::vector<unsigned> v;
                for (unsigned x = 0 ; x < sz ; ++x)
                    for (unsigned y = 0 ; y < x ; ++y)
                        v.push_back(x);

                std::vector<unsigned> r;
                fast_unique_copy(v.begin(), v.end(), std::back_inserter(r));

                TestMessageSuffix vs("v=" + join(v.begin(), v.end(), ","));
                TestMessageSuffix rs("r=" + join(r.begin(), r.end(), ","));
                if (sz == 0)
                    TEST_CHECK_EQUAL(r.size(), std::size_t(0));
                else
                {
                    TEST_CHECK_EQUAL(r.size(), sz - 1);
                    for (unsigned x = 0 ; x < sz - 1 ; ++x)
                        TEST_CHECK_EQUAL(r[x], x + 1);
                }
            }
        }

    } test_fast_unique_copy_nxn;
}

