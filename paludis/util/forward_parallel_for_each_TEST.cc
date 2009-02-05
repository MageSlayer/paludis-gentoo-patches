/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009 Ciaran McCreesh
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

#include <paludis/util/forward_parallel_for_each.hh>
#include <test/test_runner.hh>
#include <test/test_framework.hh>
#include <vector>
#include <algorithm>

using namespace test;
using namespace paludis;

namespace
{
    void inc(int & b) throw ()
    {
        ++b;
    }
}

namespace test_cases
{
    struct ForwardParallelForEachTest : TestCase
    {
        ForwardParallelForEachTest() : TestCase("forward_parallel_for_each") { }

        void run()
        {
            std::vector<int> t(1000, 0);

            forward_parallel_for_each(t.begin(), t.end(), inc, 10, 1);
            TEST_CHECK(1000 == std::count(t.begin(), t.end(), 1));

            forward_parallel_for_each(t.begin(), t.end(), inc, 1, 100);
            TEST_CHECK(1000 == std::count(t.begin(), t.end(), 2));

            forward_parallel_for_each(t.begin(), t.end(), inc, 10, 10);
            TEST_CHECK(1000 == std::count(t.begin(), t.end(), 3));
        }
    } test_forward_parallel_for_each;
}

