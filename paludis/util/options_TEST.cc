/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2009, 2010 Ciaran McCreesh
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

#include "options.hh"
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace test;
using namespace paludis;

enum MyOption { mo_one, mo_two, mo_three, mo_four, mo_five, mo_six, mo_seven, mo_eight, mo_nine, mo_ten, last_mo };
typedef Options<MyOption> MyOptions;

namespace test_cases
{
    struct OptionsTest : TestCase
    {
        OptionsTest() : TestCase("options") { }

        void run()
        {
            MyOptions options, second;

            TEST_CHECK(! options.any());

            for (MyOption o(static_cast<MyOption>(0)) ; o < last_mo ; o = static_cast<MyOption>(static_cast<long>(o) + 1))
            {
                TestMessageSuffix so("o:" + stringify(o));
                TEST_CHECK(! options[o]);
            }

            for (MyOption o(static_cast<MyOption>(0)) ; o < last_mo ; o = static_cast<MyOption>(static_cast<long>(o) + 2))
                options = options + o;

            TEST_CHECK(options.any());

            for (MyOption o(static_cast<MyOption>(0)) ; o < last_mo ; o = static_cast<MyOption>(static_cast<long>(o) + 2))
            {
                TestMessageSuffix so("o:" + stringify(o));
                TEST_CHECK(options[o]);
            }

            for (MyOption o(static_cast<MyOption>(1)) ; o < last_mo ; o = static_cast<MyOption>(static_cast<long>(o) + 2))
            {
                TestMessageSuffix so("o:" + stringify(o));
                TEST_CHECK(! options[o]);
            }

            for (MyOption o(static_cast<MyOption>(0)) ; o < last_mo ; o = static_cast<MyOption>(static_cast<long>(o) + 1))
                second += o;

            options |= second;

            for (MyOption o(static_cast<MyOption>(0)) ; o < last_mo ; o = static_cast<MyOption>(static_cast<long>(o) + 1))
            {
                TestMessageSuffix so("o:" + stringify(o));
                TEST_CHECK(options[o]);
            }

            for (MyOption o(static_cast<MyOption>(1)) ; o < last_mo ; o = static_cast<MyOption>(static_cast<long>(o) + 2))
                options -= o;

            TEST_CHECK(options.any());

            for (MyOption o(static_cast<MyOption>(0)) ; o < last_mo ; o = static_cast<MyOption>(static_cast<long>(o) + 2))
                options = options - o;

            TEST_CHECK(! options.any());

            second = MyOptions() + mo_three + mo_eight;
            options |= second;
            options += mo_seven;
            TEST_CHECK(options.any());
            options.subtract(second);
            TEST_CHECK(options.any());
            options -= mo_seven;
            TEST_CHECK(! options.any());
            TEST_CHECK(options.highest_bit() >= static_cast<int>(last_mo));
        }
    } test_options;
}

