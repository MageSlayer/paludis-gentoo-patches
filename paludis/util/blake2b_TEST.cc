/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2011 Ciaran McCreesh
 * Copyright (c) 2008, 2011 David Leverton
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

#include <paludis/util/blake2b.hh>

#include <gtest/gtest.h>

using namespace paludis;

namespace
{
    std::string Blake(const std::string & data)
    {
        std::stringstream ss(data);
        Blake2b s(ss);
        return s.hexsum();
    }
}

TEST(Blake, t0)
{
    ASSERT_EQ("786a02f742015903c6c6fd852552d272912f4740e15847618a86e217f71f5419d25e1031afee585313896444934eb04b903a685b1448b755d56f701afe9be2ce",
            Blake(""));
}

TEST(Blake, t1)
{
    ASSERT_EQ("333fcb4ee1aa7c115355ec66ceac917c8bfd815bf7587d325aec1864edd24e34d5abe2c6b1b5ee3face62fed78dbef802f2a85cb91d455a8f5249d330853cb3c",
            Blake("a"));
}

TEST(Blake, t2)
{
    ASSERT_EQ("ba80a53f981c4d0d6a2797b69f12f6e94c212f14685ac4b74b12bb6fdbffa2d17d87c5392aab792dc252d5de4533cc9518d38aa8dbf1925ab92386edd4009923",
            Blake("abc"));
}

TEST(Blake, t3)
{
    ASSERT_EQ("3c26ce487b1c0f062363afa3c675ebdbf5f4ef9bdc022cfbef91e3111cdc283840d8331fc30a8a0906cff4bcdbcd230c61aaec60fdfad457ed96b709a382359a",
            Blake("message digest"));
}

TEST(Blake, t4)
{
    ASSERT_EQ("c68ede143e416eb7b4aaae0d8e48e55dd529eafed10b1df1a61416953a2b0a5666c761e7d412e6709e31ffe221b7a7a73908cb95a4d120b8b090a87d1fbedb4c",
            Blake("abcdefghijklmnopqrstuvwxyz"));
}

TEST(Blake, t5)
{
    ASSERT_EQ("99964802e5c25e703722905d3fb80046b6bca698ca9e2cc7e49b4fe1fa087c2edf0312dfbb275cf250a1e542fd5dc2edd313f9c491127c2e8c0c9b24168e2d50",
            Blake("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"));
}

TEST(Blake, t6)
{
    ASSERT_EQ("686f41ec5afff6e87e1f076f542aa466466ff5fbde162c48481ba48a748d842799f5b30f5b67fc684771b33b994206d05cc310f31914edd7b97e41860d77d282",
            Blake("12345678901234567890123456789012345678901234567890123456789012345678901234567890"));
}

TEST(Blake, t7)
{
    ASSERT_EQ("0b8a4e505bb0246cb9ab1320bd98dac9f7c8d91ea191e2edeb4d93d7be5700a8561af91d12fcd03a3b3b8d98a22daa27586d85954500247528ca108bb632dfa5",
            Blake("abcdbcdecdefdefgefghfghighijhijk"));
}

TEST(Blake, t8)
{
    ASSERT_EQ("98fb3efb7206fd19ebf69b6f312cf7b64e3b94dbe1a17107913975a793f177e1d077609d7fba363cbba00d05f7aa4e4fa8715d6428104c0a75643b0ff3fd3eaf",
            Blake(std::string(1000000, 'a')));
}

TEST(Blake, z8)
{
    ASSERT_EQ("2fa3f686df876995167e7c2e5d74c4c7b6e48f8068fe0e44208344d480f7904c36963e44115fe3eb2a3ac8694c28bcb4f5a0f3276f2e79487d8219057a506e4b",
            Blake(std::string(1, '\0')));
}

TEST(Blake, z16)
{
    ASSERT_EQ("5ba7f7e4ade7e5803c59d184326420823f7f860effcfba0bb896d568f59b8d85181cfff25929d40b18e01069c2ef5c31754f1d821a1f3f80f896f4dde374a2f1",
            Blake(std::string(2, '\0')));
}

TEST(Blake, z24)
{
    ASSERT_EQ("1d76566758a5b6bfc561f1c936d8fc86b5b42ea22ab1dabf40d249d27dd906401fde147e53f44c103dd02a254916be113e51de1077a946a3a0c1272b9b348437",
            Blake(std::string(3, '\0')));
}

TEST(Blake, z32)
{
    ASSERT_EQ("204980ffebcb7eb3bfdd22c1d06cd384ba2bdeddce296483002ee55b14d294fe70c1740a1d6f9979b4b30dcd3fe503830cb292b8be50b1f0201080b54cf87b97",
            Blake(std::string(4, '\0')));
}

TEST(Blake, z40)
{
    ASSERT_EQ("bed1b15ae0dadc94819524fd393a15e725d7c7b937d3c74ac468ca3bc5a0aa3e279235881466b5850a490e8cfcf0d7f18fd6fa6d4816260f2f900059832249bd",
            Blake(std::string(5, '\0')));
}

TEST(Blake, z48)
{
    ASSERT_EQ("3d2bb36e7ba2c8c744e91d59552d270adcae69bd1a91b5661d88613bc5dbeba3bc67ec824f4fd83bb181353fb1072b1ef0ab52accee8e2fac1c19be8e8926756",
            Blake(std::string(6, '\0')));
}

TEST(Blake, z56)
{
    ASSERT_EQ("99a85f3ecdb219f7523c32afddd331f7aef06476ee4a1de32a374a6f0e1be52e8d6971ff96cf4a492d9fcf677981628d98f21c14bfaf4169b328526d9c6d1f41",
            Blake(std::string(7, '\0')));
}

TEST(Blake, z64)
{
    ASSERT_EQ("482ae5a29fbe856c7272f2071b8b0f0359ee2d89ff392b8a900643fbd0836eccd067b8bf41909e206c90d45d6e7d8b6686b93ecaee5fe1a9060d87b672101310",
            Blake(std::string(8, '\0')));
}


TEST(Blake, z1016)
{
    ASSERT_EQ("93cac6a4bedd751e1c145f8e76fec88fec246675898475585603bd228f883bcf4ebcc68ead8fa5f27890a243fa938bd7323ad41f9f06048a732cce2070b212c3",
            Blake(std::string(127, '\0')));
}
