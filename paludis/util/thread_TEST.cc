/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008 Ciaran McCreesh
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

#include <paludis/util/thread.hh>
#include <test/test_runner.hh>
#include <test/test_framework.hh>
#include <time.h>

using namespace test;
using namespace paludis;

namespace
{
    void make_true(bool & b) throw ()
    {
        b = true;
    }
}

namespace test_cases
{
    struct ThreadTest : TestCase
    {
        ThreadTest() : TestCase("thread") { }

        void run()
        {
            bool x(false);
            {
                Thread t(std::bind(&make_true, std::ref(x)));
            }
            TEST_CHECK(x);
        }
    } test_thread;
}

