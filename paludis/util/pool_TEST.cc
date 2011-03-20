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

#include <gtest/gtest.h>

using namespace paludis;

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

TEST(Pool, Monkey)
{
    auto a(Pool<Monkey>::get_instance()->create(std::string("alexander")));
    auto b(Pool<Monkey>::get_instance()->create(std::string("gunther")));
    auto c(Pool<Monkey>::get_instance()->create(std::string("alexander")));

    EXPECT_TRUE(a->name == "alexander");
    EXPECT_TRUE(b->name == "gunther");
    EXPECT_TRUE(c->name == "alexander");

    EXPECT_TRUE(a == c);
}

TEST(Pool, Weasel)
{
    auto a(Pool<Weasel>::get_instance()->create(std::string("william"), 8));
    auto b(Pool<Weasel>::get_instance()->create(std::string("tony"), 5));
    auto c(Pool<Weasel>::get_instance()->create(std::string("william"), 8));
    auto d(Pool<Weasel>::get_instance()->create(std::string("tony"), 10));

    EXPECT_TRUE(a->name == "william");
    EXPECT_TRUE(a->viciousness == 8);

    EXPECT_TRUE(b->name == "tony");
    EXPECT_TRUE(b->viciousness == 5);

    EXPECT_TRUE(c->name == "william");
    EXPECT_TRUE(c->viciousness == 8);

    EXPECT_TRUE(d->name == "tony");
    EXPECT_TRUE(d->viciousness == 10);

    EXPECT_TRUE(a == c);
    EXPECT_TRUE(b != d);
}

