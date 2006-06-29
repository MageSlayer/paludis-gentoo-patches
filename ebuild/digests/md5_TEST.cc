/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#include "md5.hh"
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace test;

namespace
{
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
}

namespace test_cases
{
    struct MD5TestCase : TestCase
    {
        std::string data;
        std::string expected;

        MD5TestCase(const std::string & s, const std::string & d,
                const std::string & e) :
            TestCase("md5 " + s),
            data(d),
            expected(e)
        {
        }

        void run()
        {
            std::stringstream ss(data);
            md5::MD5 s(ss);
            TEST_CHECK_EQUAL(s.hexsum(), expected);
        }
    };

    MD5TestCase t_0("empty", "", "d41d8cd98f00b204e9800998ecf8427e");
    MD5TestCase t_1("a", "a", "0cc175b9c0f1b6a831c399e269772661");
    MD5TestCase t_2("abc", "abc", "900150983cd24fb0d6963f7d28e17f72");
    MD5TestCase t_3("message digest", "message digest", "f96b697d7cb7938d525a2f31aaf161d0");
    MD5TestCase t_4("a..z", "abcdefghijklmnopqrstuvwxyz",
            "c3fcd3d76192e4007dfb496cca67e13b");
    MD5TestCase t_6("A...Za...z0...9",
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
            "d174ab98d277d9f5a5611c2c9f419d9f");
    MD5TestCase t_7("8 times 1234567890",
            "12345678901234567890123456789012345678901234567890123456789012345678901234567890",
            "57edf4a22be3c955ac49da2e2107b67a");
    MD5TestCase t_8("one million times a",
            std::string(1000000, 'a'),
            "7707d6ae4e027c70eea2a935c2296f21");
}


