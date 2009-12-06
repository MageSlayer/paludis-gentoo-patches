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

#include <paludis/util/string_list_stream.hh>
#include <paludis/util/thread.hh>
#include <test/test_runner.hh>
#include <test/test_framework.hh>
#include <tr1/functional>

using namespace test;
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

namespace test_cases
{
    struct StringListStreamTest : TestCase
    {
        StringListStreamTest() : TestCase("string list stream") { }

        void run()
        {
            StringListStream s;
            s << "foo" << std::endl << "bar" << std::endl << "baz" << std::endl;
            s.nothing_more_to_write();

            std::string l;

            TEST_CHECK(std::getline(s, l));
            TEST_CHECK_EQUAL(l, "foo");

            TEST_CHECK(std::getline(s, l));
            TEST_CHECK_EQUAL(l, "bar");

            TEST_CHECK(std::getline(s, l));
            TEST_CHECK_EQUAL(l, "baz");

            TEST_CHECK(! std::getline(s, l));
        }
    } test_string_list_stream;

    struct StringListStreamThreadsTest : TestCase
    {
        StringListStreamThreadsTest() : TestCase("string list stream threads") { }

        void run()
        {
            StringListStream s;
            Thread t(std::tr1::bind(&write_to, std::tr1::ref(s)));

            std::string l;
            for (int n(0) ; n < 100 ; ++n)
            {
                TestMessageSuffix sx(stringify(n));
                TEST_CHECK(std::getline(s, l));
                TEST_CHECK_EQUAL(l, stringify(n));
            }

            TEST_CHECK(! std::getline(s, l));
        }
    } test_string_list_stream_thread;
}

