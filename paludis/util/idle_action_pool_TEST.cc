/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#include <paludis/util/idle_action_pool.hh>
#include <test/test_runner.hh>
#include <test/test_framework.hh>
#include <sys/time.h>

using namespace test;
using namespace paludis;

namespace
{
    IdleActionResult make_true(bool & b) throw ()
    {
        b = true;
        return iar_success;
    }
}

namespace test_cases
{
    struct IdleActionPoolTest : TestCase
    {
        IdleActionPoolTest() : TestCase("idle action pool") { }

        void run()
        {
            bool x(false);
            IdleActionPool::get_instance()->required_idle_action(tr1::bind(&make_true, tr1::ref(x)));
            while (true)
            {
                TEST_CHECK(true);
                if (x)
                    break;

                struct timespec t;
                t.tv_sec = 0;
                t.tv_nsec = 1000;
                nanosleep(&t, 0);
            }
        }
    } test_action_queue;
}


