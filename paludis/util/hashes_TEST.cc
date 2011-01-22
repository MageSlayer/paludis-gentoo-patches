/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2011 Ciaran McCreesh
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

#include <paludis/util/hashes.hh>
#include <test/test_runner.hh>
#include <test/test_framework.hh>
#include <set>

using namespace test;
using namespace paludis;

namespace test_cases
{
    struct HashTupleTest : TestCase
    {
        HashTupleTest() : TestCase("hash tuple") { }

        void run()
        {
            std::set<std::size_t> hashes;

            for (int x(1) ; x < 20 ; ++x)
                TEST_CHECK(hashes.insert(Hash<std::tuple<int> >()(std::make_tuple(x))).second);

            for (int x(1) ; x < 20 ; ++x)
                for (int y(1) ; y < 20 ; ++y)
                    TEST_CHECK(hashes.insert(Hash<std::tuple<int, int> >()(std::make_tuple(x, y))).second);

            for (int x(1) ; x < 20 ; ++x)
                for (int y(1) ; y < 20 ; ++y)
                    TEST_CHECK(hashes.insert(Hash<std::tuple<int, int, int> >()(std::make_tuple(x, y, 42))).second);
        }
    } test_hash_tuple;
}

