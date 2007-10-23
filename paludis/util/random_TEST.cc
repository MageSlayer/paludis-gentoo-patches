/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh
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

#include "random.hh"
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <vector>
#include <algorithm>

/** \file
 * Test cases for paludis::Random.
 *
 */

using namespace paludis;
using namespace test;

namespace
{
    struct RandomUpTo
    {
        Random random;
        unsigned max;

        RandomUpTo(const Random & r, const unsigned m) :
            random(r),
            max(m)
        {
        }

        int operator() ()
        {
            return random(max);
        }
    };

    inline double square(double v)
    {
        return v * v;
    }
}

namespace test_cases
{
    /**
     * \test Test Random distibutions using counts.
     *
     */
    struct RandomDistributionCountsTest : TestCase
    {
        RandomDistributionCountsTest() : TestCase("distribution (counts)") { }

        void run()
        {
            std::vector<int> v;
            v.reserve(10000);
            std::generate_n(std::back_inserter(v), 10000, RandomUpTo(Random(), 10));

            TEST_CHECK_EQUAL(0, *std::min_element(v.begin(), v.end()));
            TEST_CHECK_EQUAL(9, *std::max_element(v.begin(), v.end()));
            for (int i(0) ; i < 10 ; ++i)
            {
                TEST_CHECK(std::count(v.begin(), v.end(), i) > 1);
                TEST_CHECK(std::count(v.begin(), v.end(), i) < 3000);
            }
        }
    } test_random_counts;

    /**
     * \test Test Random distibutions using chi square.
     *
     * This is a chi square test, so it could theoretically fail
     * occasionally. See \ref TaoCP2 3.3.1 for details.
     */
    struct RandomDistributionChiSquaredTest : TestCase
    {
        RandomDistributionChiSquaredTest() : TestCase("distribution (chi square)") { }

        void run()
        {
            int failures(0);

            for (int attempts(0) ; attempts < 3 ; ++ attempts)
            {
                std::vector<int> v;
                v.reserve(10000);
                std::generate_n(std::back_inserter(v), 10000, RandomUpTo(Random(), 10));

                double a(0);
                for (int i(0) ; i <= 9 ; ++i)
                    a += (square(std::count(v.begin(), v.end(), i) - (10000 / 10)) / (10000 / 10));

                TestMessageSuffix suffix("a=" + stringify(a), true);
                TEST_CHECK(true);
                if ((a < 2.088) || (a > 21.67))
                    ++failures;
            }

            TEST_CHECK(failures <= 1);
        }
    } test_random_chi_square;
}

