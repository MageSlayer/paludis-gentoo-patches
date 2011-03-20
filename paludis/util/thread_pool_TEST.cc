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

#include <paludis/util/thread_pool.hh>

#include <vector>
#include <algorithm>

#include <gtest/gtest.h>

using namespace paludis;

namespace
{
    void make_one(int & b) throw ()
    {
        b = 1;
    }
}

TEST(ThreadPool, Works)
{
    const int n_threads = 10;
    std::vector<int> t(n_threads, 0);
    {
        ThreadPool p;
        for (int x(0) ; x < n_threads ; ++x)
            p.create_thread(std::bind(&make_one, std::ref(t[x])));
    }
    ASSERT_TRUE(n_threads == std::count(t.begin(), t.end(), 1));
}

