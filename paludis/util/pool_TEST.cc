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

#include <paludis/util/pool-impl.hh>
#include <paludis/util/singleton-impl.hh>
#include <paludis/util/pimp-impl.hh>
#include <test/test_runner.hh>
#include <test/test_framework.hh>

using namespace paludis;
using namespace test;

namespace
{
    struct Monkey
    {
        std::string name;

        explicit Monkey(const std::string & n) :
            name(n)
        {
        }
    };

    struct Weasel
    {
        std::string name;
        int viciousness;

        explicit Weasel(const std::string & n, int v) :
            name(n),
            viciousness(v)
        {
        }
    };
}

namespace test_cases
{
    struct PoolMonkeyTest : TestCase
    {
        PoolMonkeyTest() : TestCase("pool monkey test") { }

        void run()
        {
            auto a(Pool<Monkey>::get_instance()->create(std::string("alexander")));
            auto b(Pool<Monkey>::get_instance()->create(std::string("gunther")));
            auto c(Pool<Monkey>::get_instance()->create(std::string("alexander")));

            TEST_CHECK(a->name == "alexander");
            TEST_CHECK(b->name == "gunther");
            TEST_CHECK(c->name == "alexander");

            TEST_CHECK(a == c);
        }
    } test_pool_monkey;

    struct PoolWeaselTest : TestCase
    {
        PoolWeaselTest() : TestCase("pool weasel test") { }

        void run()
        {
            auto a(Pool<Weasel>::get_instance()->create(std::string("william"), 8));
            auto b(Pool<Weasel>::get_instance()->create(std::string("tony"), 5));
            auto c(Pool<Weasel>::get_instance()->create(std::string("william"), 8));
            auto d(Pool<Weasel>::get_instance()->create(std::string("tony"), 10));

            TEST_CHECK(a->name == "william");
            TEST_CHECK(a->viciousness == 8);

            TEST_CHECK(b->name == "tony");
            TEST_CHECK(b->viciousness == 5);

            TEST_CHECK(c->name == "william");
            TEST_CHECK(c->viciousness == 8);

            TEST_CHECK(d->name == "tony");
            TEST_CHECK(d->viciousness == 10);

            TEST_CHECK(a == c);
            TEST_CHECK(b != d);
        }
    } test_pool_weasel;
}

