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

#include <paludis/util/join.hh>
#include <paludis/util/parallel_for_each.hh>
#include <test/test_runner.hh>
#include <test/test_framework.hh>
#include <vector>

using namespace paludis;
using namespace test;

namespace
{
    int factorial(const int i)
    {
        if (i < 2)
            return 1;
        else
            return i * factorial(i - 1);
    }

    void factorialify(int & i)
    {
        i = factorial(i);
    }
}

namespace test_cases
{
    struct ParallelForEachTest : TestCase
    {
        ParallelForEachTest() : TestCase("parallel_for_each") { }

        void run()
        {
            std::vector<int> v;
            v.push_back(2);
            v.push_back(4);
            v.push_back(6);

            parallel_for_each(v.begin(), v.end(), &factorialify);

            TEST_CHECK_EQUAL(join(v.begin(), v.end(), " "), "2 24 720");
        }
    } test_parallel_for_each;
}

