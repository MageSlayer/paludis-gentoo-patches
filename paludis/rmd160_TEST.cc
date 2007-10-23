/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh
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

#include <paludis/rmd160.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace test;
using namespace paludis;

namespace
{
    /**
     * \name Test utilities
     * \{
     */

    unsigned char dehex_c(unsigned char c)
    {
        if (c >= '0' && c <= '9')
            return c - '0';
        else if (c >= 'a' && c <= 'f')
            return c + 10 - 'a';
        else
            throw "meh!";
    }

    std::string dehex(const std::string & s)
    {
        std::string result;
        std::string::size_type p(0);
        while (p < s.length())
        {
            unsigned char c;
            c = (dehex_c(s.at(p)) << 4) + dehex_c(s.at(p + 1));
            result.append(1, c);
            p += 2;
        }
        return result;
    }

    /**
     * \}
     */
}

namespace test_cases
{
    /**
     * \name Test cases for paludis::digests::RMD160
     * \{
     */

    struct RMD160TestCase : TestCase
    {
        std::string data;
        std::string expected;

        RMD160TestCase(const std::string & s, const std::string & d,
                const std::string & e) :
            TestCase("rmd160 " + s),
            data(d),
            expected(e)
        {
        }

        void run()
        {
            std::stringstream ss(data);
            RMD160 s(ss);
            TEST_CHECK_EQUAL(s.hexsum(), expected);
        }
    };

    RMD160TestCase t_0("empty", "", "9c1185a5c5e9fc54612808977ee8f548b2258d31");
    RMD160TestCase t_1("a", "a", "0bdc9d2d256b3ee9daae347be6f4dc835a467ffe");
    RMD160TestCase t_2("abc", "abc", "8eb208f7e05d987a9b044a8e98c6b087f15a0bfc");
    RMD160TestCase t_3("message digest", "message digest", "5d0689ef49d2fae572b881b123a85ffa21595f36");
    RMD160TestCase t_4("a..z", "abcdefghijklmnopqrstuvwxyz",
            "f71c27109c692c1b56bbdceb5b9d2865b3708dbc");
    RMD160TestCase t_5("abcdbcde...nopq",
            "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
            "12a053384a9c0c88e405a06c27dcf49ada62eb2b");
    RMD160TestCase t_6("A...Za...z0...9",
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
            "b0e20b6e3116640286ed3a87a5713079b21f5189");
    RMD160TestCase t_7("8 times 1234567890",
            "12345678901234567890123456789012345678901234567890123456789012345678901234567890",
            "9b752e45573d4b39f4dbd3323cab82bf63326bfb");
    RMD160TestCase t_8("one million times a",
            std::string(1000000, 'a'),
            "52783243c1697bdbe16d37f97f68f08325dc1528");

    /**
     * \}
     */
}

