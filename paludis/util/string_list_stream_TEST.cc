/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2011, 2012 Ciaran McCreesh
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

#include <paludis/util/string_list_stream.hh>
#include <paludis/util/stringify.hh>

#include <functional>
#include <thread>

#include <gtest/gtest.h>

using namespace paludis;

namespace
{
    void write_to(StringListStream & s)
    {
        for (int n(0) ; n < 100 ; ++n)
        {
            s << n << std::endl;
            usleep(100);
        }

        s.nothing_more_to_write();
    }
}

TEST(StringListStream, Works)
{
    StringListStream s;
    s << "foo" << std::endl << "bar" << std::endl << "baz" << std::endl;
    s.nothing_more_to_write();

    std::string l;

    ASSERT_TRUE(bool(std::getline(s, l)));
    EXPECT_EQ("foo", l);

    ASSERT_TRUE(bool(std::getline(s, l)));
    EXPECT_EQ("bar", l);

    ASSERT_TRUE(bool(std::getline(s, l)));
    EXPECT_EQ("baz", l);

    ASSERT_TRUE(! std::getline(s, l));
}

TEST(StringListStream, Threads)
{
    StringListStream s;
    std::thread t(std::bind(&write_to, std::ref(s)));

    std::string l;
    for (int n(0) ; n < 100 ; ++n)
    {
        ASSERT_TRUE(bool(std::getline(s, l)));
        EXPECT_EQ(stringify(n), l);
    }

    ASSERT_TRUE(! std::getline(s, l));

    t.join();
}

