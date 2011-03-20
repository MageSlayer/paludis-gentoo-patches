/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2011 Ciaran McCreesh
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

#include <paludis/util/action_queue.hh>
#include <paludis/util/join.hh>

#include <list>

#include <gtest/gtest.h>

using namespace paludis;

namespace
{
    void push_back_to_list(std::list<int> * const l, int x)
    {
        l->push_back(x);
    }
}

TEST(ActionQueue, Works)
{
    std::list<int> l;
    {
        ActionQueue q;
        for (int x(0) ; x < 100 ; ++x)
            q.enqueue(std::bind(&push_back_to_list, &l, x));
    }

    std::list<int>::const_iterator i(l.begin());
    for (int x(0) ; x < 100 ; ++x)
    {
        ASSERT_TRUE(i != l.end());
        EXPECT_EQ(x, *i);
        ++i;
    }
    EXPECT_TRUE(i == l.end());
}

