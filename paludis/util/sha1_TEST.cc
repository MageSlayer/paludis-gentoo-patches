/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh
 * Copyright (c) 2008 David Leverton
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

#include <paludis/util/sha1.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace paludis;
using namespace test;

namespace test_cases
{
    /**
     * \name Test cases for paludis::SHA1
     * \{
     */

    struct SHA1TestCase : TestCase
    {
        std::string data;
        std::string expected;

        SHA1TestCase(const std::string & s, const std::string & d,
                const std::string & e) :
            TestCase("sha1 " + s),
            data(d),
            expected(e)
        {
        }

        void run()
        {
            std::stringstream ss(data);
            SHA1 s(ss);
            TEST_CHECK_EQUAL(s.hexsum(), expected);
        }
    };

    SHA1TestCase t_0("empty", "", "da39a3ee5e6b4b0d3255bfef95601890afd80709");
    SHA1TestCase t_1("a", "a", "86f7e437faa5a7fce15d1ddcb9eaeaea377667b8");
    SHA1TestCase t_2("abc", "abc", "a9993e364706816aba3e25717850c26c9cd0d89d");
    SHA1TestCase t_3("message digest", "message digest", "c12252ceda8be8994d5fa0290a47231c1d16aae3");
    SHA1TestCase t_4("a..z", "abcdefghijklmnopqrstuvwxyz",
            "32d10c7b8cf96570ca04ce37f2a19d84240d3a89");
    SHA1TestCase t_6("A...Za...z0...9",
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
            "761c457bf73b14d27e9e9265c46f4b4dda11f940");
    SHA1TestCase t_7("8 times 1234567890",
            "12345678901234567890123456789012345678901234567890123456789012345678901234567890",
            "50abf5706a150990a08b2c5ea40fa0e585554732");
    SHA1TestCase t_8("one million times a",
            std::string(1000000, 'a'),
            "34aa973cd4c4daa4f61eeb2bdbad27316534016f");
    SHA1TestCase t_9("a-db-ec-fd-ge-hf-ig-jh-ki-lj-mk-nl-om-pn-q",
            "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
            "84983e441c3bd26ebaae4aa1f95129e5e54670f1");
    SHA1TestCase t_10("80 times 01234567",
            "0123456701234567012345670123456701234567012345670123456701234567"
            "0123456701234567012345670123456701234567012345670123456701234567"
            "0123456701234567012345670123456701234567012345670123456701234567"
            "0123456701234567012345670123456701234567012345670123456701234567"
            "0123456701234567012345670123456701234567012345670123456701234567"
            "0123456701234567012345670123456701234567012345670123456701234567"
            "0123456701234567012345670123456701234567012345670123456701234567"
            "0123456701234567012345670123456701234567012345670123456701234567"
            "0123456701234567012345670123456701234567012345670123456701234567"
            "0123456701234567012345670123456701234567012345670123456701234567",
            "dea356a2cddd90c7a7ecedc5ebb563934f460452");

    /**
     * \}
     */
}


