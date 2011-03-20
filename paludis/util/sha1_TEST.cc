/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2011 Ciaran McCreesh
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

#include <gtest/gtest.h>

using namespace paludis;

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

    std::string sha1(const std::string & data)
    {
        std::stringstream ss(data);
        SHA1 s(ss);
        return s.hexsum();
    }

    std::string sha1_dehex(const std::string & data)
    {
        std::stringstream ss(dehex(data));
        SHA1 s(ss);
        return s.hexsum();
    }
}

TEST(SHA1, t0)
{
    ASSERT_EQ("da39a3ee5e6b4b0d3255bfef95601890afd80709", sha1(""));
}

TEST(SHA1, t1)
{
    ASSERT_EQ("86f7e437faa5a7fce15d1ddcb9eaeaea377667b8", sha1("a"));
}

TEST(SHA1, t2)
{
    ASSERT_EQ("a9993e364706816aba3e25717850c26c9cd0d89d", sha1("abc"));
}

TEST(SHA1, t3)
{
    ASSERT_EQ("c12252ceda8be8994d5fa0290a47231c1d16aae3", sha1("message digest"));
}

TEST(SHA1, t4)
{
    ASSERT_EQ("32d10c7b8cf96570ca04ce37f2a19d84240d3a89", sha1("abcdefghijklmnopqrstuvwxyz"));
}

TEST(SHA1, t6)
{
    ASSERT_EQ("761c457bf73b14d27e9e9265c46f4b4dda11f940", sha1("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"));
}

TEST(SHA1, t7)
{
    ASSERT_EQ("50abf5706a150990a08b2c5ea40fa0e585554732", sha1("12345678901234567890123456789012345678901234567890123456789012345678901234567890"));
}

TEST(SHA1, t8)
{
    ASSERT_EQ("34aa973cd4c4daa4f61eeb2bdbad27316534016f", sha1(std::string(1000000, 'a')));
}

TEST(SHA1, t9)
{
    ASSERT_EQ("84983e441c3bd26ebaae4aa1f95129e5e54670f1", sha1("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"));
}

TEST(SHA1, t10)
{
    ASSERT_EQ("dea356a2cddd90c7a7ecedc5ebb563934f460452", sha1(
                "0123456701234567012345670123456701234567012345670123456701234567"
                "0123456701234567012345670123456701234567012345670123456701234567"
                "0123456701234567012345670123456701234567012345670123456701234567"
                "0123456701234567012345670123456701234567012345670123456701234567"
                "0123456701234567012345670123456701234567012345670123456701234567"
                "0123456701234567012345670123456701234567012345670123456701234567"
                "0123456701234567012345670123456701234567012345670123456701234567"
                "0123456701234567012345670123456701234567012345670123456701234567"
                "0123456701234567012345670123456701234567012345670123456701234567"
                "0123456701234567012345670123456701234567012345670123456701234567"));
}

TEST(SHA1, t11)
{
    ASSERT_EQ("2fd4e1c67a2d28fced849ee1bb76e7391b93eb12", sha1("The quick brown fox jumps over the lazy dog"));
}

TEST(SHA1, t12)
{
    ASSERT_EQ("de9f2c7fd25e1b3afad3e85a0bd17d9b100db4b3", sha1("The quick brown fox jumps over the lazy cog"));
}

/*
 * the following tests are from:
 *     http://web.archive.org/web/20070205093211/http://csrc.nist.gov/cryptval/shs/shabytetestvectors.zip
 * (append a null byte because the wayback machine sucks)
 */

TEST(SHA1, t8SHA1ShortMsg)
{
    ASSERT_EQ("99f2aa95e36f95c2acb0eaf23998f030638f3f15",
            sha1_dehex("a8"));
}

TEST(SHA1, t16SHA1ShortMsg)
{
    ASSERT_EQ("f944dcd635f9801f7ac90a407fbc479964dec024",
            sha1_dehex("3000"));
}

TEST(SHA1, t24SHA1ShortMsg)
{
    ASSERT_EQ("a444319e9b6cc1e8464c511ec0969c37d6bb2619",
            sha1_dehex("42749e"));
}

TEST(SHA1, t32SHA1ShortMsg)
{
    ASSERT_EQ("16a0ff84fcc156fd5d3ca3a744f20a232d172253",
            sha1_dehex("9fc3fe08"));
}

TEST(SHA1, t40SHA1ShortMsg)
{
    ASSERT_EQ("fec9deebfcdedaf66dda525e1be43597a73a1f93",
            sha1_dehex("b5c1c6f1af"));
}

TEST(SHA1, t48SHA1ShortMsg)
{
    ASSERT_EQ("8ce051181f0ed5e9d0c498f6bc4caf448d20deb5",
            sha1_dehex("e47571e5022e"));
}

TEST(SHA1, t56SHA1ShortMsg)
{
    ASSERT_EQ("67da53837d89e03bf652ef09c369a3415937cfd3",
            sha1_dehex("3e1b28839fb758"));
}

TEST(SHA1, t64SHA1ShortMsg)
{
    ASSERT_EQ("305e4ff9888ad855a78573cddf4c5640cce7e946",
            sha1_dehex("a81350cbb224cb90"));
}

TEST(SHA1, t72SHA1ShortMsg)
{
    ASSERT_EQ("5902b77b3265f023f9bbc396ba1a93fa3509bde7",
            sha1_dehex("c243d167923dec3ce1"));
}

TEST(SHA1, t80SHA1ShortMsg)
{
    ASSERT_EQ("fcade5f5d156bf6f9af97bdfa9c19bccfb4ff6ab",
            sha1_dehex("50ac18c59d6a37a29bf4"));
}

TEST(SHA1, t88SHA1ShortMsg)
{
    ASSERT_EQ("1d20fbe00533c10e3cbd6b27088a5de0c632c4b5",
            sha1_dehex("98e2b611ad3b1cccf634f6"));
}

TEST(SHA1, t96SHA1ShortMsg)
{
    ASSERT_EQ("7e1b7e0f7a8f3455a9c03e9580fd63ae205a2d93",
            sha1_dehex("73fe9afb68e1e8712e5d4eec"));
}

TEST(SHA1, t104SHA1ShortMsg)
{
    ASSERT_EQ("706f0677146307b20bb0e8d6311e329966884d13",
            sha1_dehex("9e701ed7d412a9226a2a130e66"));
}

TEST(SHA1, t112SHA1ShortMsg)
{
    ASSERT_EQ("a7241a703aaf0d53fe142f86bf2e849251fa8dff",
            sha1_dehex("6d3ee90413b0a7cbf69e5e6144ca"));
}

TEST(SHA1, t120SHA1ShortMsg)
{
    ASSERT_EQ("400f53546916d33ad01a5e6df66822dfbdc4e9e6",
            sha1_dehex("fae24d56514efcb530fd4802f5e71f"));
}

TEST(SHA1, t128SHA1ShortMsg)
{
    ASSERT_EQ("fac8ab93c1ae6c16f0311872b984f729dc928ccd",
            sha1_dehex("c5a22dd6eda3fe2bdc4ddb3ce6b35fd1"));
}

TEST(SHA1, t136SHA1ShortMsg)
{
    ASSERT_EQ("fba6d750c18da58f6e2aab10112b9a5ef3301b3b",
            sha1_dehex("d98cded2adabf08fda356445c781802d95"));
}

TEST(SHA1, t144SHA1ShortMsg)
{
    ASSERT_EQ("29d27c2d44c205c8107f0351b05753ac708226b6",
            sha1_dehex("bcc6d7087a84f00103ccb32e5f5487a751a2"));
}

TEST(SHA1, t152SHA1ShortMsg)
{
    ASSERT_EQ("b971bfc1ebd6f359e8d74cb7ecfe7f898d0ba845",
            sha1_dehex("36ecacb1055434190dbbc556c48bafcb0feb0d"));
}

TEST(SHA1, t160SHA1ShortMsg)
{
    ASSERT_EQ("96d08c430094b9fcc164ad2fb6f72d0a24268f68",
            sha1_dehex("5ff9edb69e8f6bbd498eb4537580b7fba7ad31d0"));
}

TEST(SHA1, t168SHA1ShortMsg)
{
    ASSERT_EQ("a287ea752a593d5209e287881a09c49fa3f0beb1",
            sha1_dehex("c95b441d8270822a46a798fae5defcf7b26abace36"));
}

TEST(SHA1, t176SHA1ShortMsg)
{
    ASSERT_EQ("a06c713779cbd88519ed4a585ac0cb8a5e9d612b",
            sha1_dehex("83104c1d8a55b28f906f1b72cb53f68cbb097b44f860"));
}

TEST(SHA1, t184SHA1ShortMsg)
{
    ASSERT_EQ("bff7d52c13a3688132a1d407b1ab40f5b5ace298",
            sha1_dehex("755175528d55c39c56493d697b790f099a5ce741f7754b"));
}

TEST(SHA1, t192SHA1ShortMsg)
{
    ASSERT_EQ("c7566b91d7b6f56bdfcaa9781a7b6841aacb17e9",
            sha1_dehex("088fc38128bbdb9fd7d65228b3184b3faac6c8715f07272f"));
}

TEST(SHA1, t200SHA1ShortMsg)
{
    ASSERT_EQ("ffa30c0b5c550ea4b1e34f8a60ec9295a1e06ac1",
            sha1_dehex("a4a586eb9245a6c87e3adf1009ac8a49f46c07e14185016895"));
}

TEST(SHA1, t208SHA1ShortMsg)
{
    ASSERT_EQ("29e66ed23e914351e872aa761df6e4f1a07f4b81",
            sha1_dehex("8e7c555270c006092c2a3189e2a526b873e2e269f0fb28245256"));
}

TEST(SHA1, t216SHA1ShortMsg)
{
    ASSERT_EQ("b28cf5e5b806a01491d41f69bd9248765c5dc292",
            sha1_dehex("a5f3bfa6bb0ba3b59f6b9cbdef8a558ec565e8aa3121f405e7f2f0"));
}

TEST(SHA1, t224SHA1ShortMsg)
{
    ASSERT_EQ("60224fb72c46069652cd78bcd08029ef64da62f3",
            sha1_dehex("589054f0d2bd3c2c85b466bfd8ce18e6ec3e0b87d944cd093ba36469"));
}

TEST(SHA1, t232SHA1ShortMsg)
{
    ASSERT_EQ("b72c4a86f72608f24c05f3b9088ef92fba431df7",
            sha1_dehex("a0abb12083b5bbc78128601bf1cbdbc0fdf4b862b24d899953d8da0ff3"));
}

TEST(SHA1, t240SHA1ShortMsg)
{
    ASSERT_EQ("73779ad5d6b71b9b8328ef7220ff12eb167076ac",
            sha1_dehex("82143f4cea6fadbf998e128a8811dc75301cf1db4f079501ea568da68eeb"));
}

TEST(SHA1, t248SHA1ShortMsg)
{
    ASSERT_EQ("a09671d4452d7cf50015c914a1e31973d20cc1a0",
            sha1_dehex("9f1231dd6df1ff7bc0b0d4f989d048672683ce35d956d2f57913046267e6f3"));
}

TEST(SHA1, t256SHA1ShortMsg)
{
    ASSERT_EQ("e88cdcd233d99184a6fd260b8fca1b7f7687aee0",
            sha1_dehex("041c512b5eed791f80d3282f3a28df263bb1df95e1239a7650e5670fc2187919"));
}

TEST(SHA1, t264SHA1ShortMsg)
{
    ASSERT_EQ("010def22850deb1168d525e8c84c28116cb8a269",
            sha1_dehex("17e81f6ae8c2e5579d69dafa6e070e7111461552d314b691e7a3e7a4feb3fae418"));
}

TEST(SHA1, t272SHA1ShortMsg)
{
    ASSERT_EQ("aeaa40ba1717ed5439b1e6ea901b294ba500f9ad",
            sha1_dehex("d15976b23a1d712ad28fad04d805f572026b54dd64961fda94d5355a0cc98620cf77"));
}

TEST(SHA1, t280SHA1ShortMsg)
{
    ASSERT_EQ("c6433791238795e34f080a5f1f1723f065463ca0",
            sha1_dehex("09fce4d434f6bd32a44e04b848ff50ec9f642a8a85b37a264dc73f130f22838443328f"));
}

TEST(SHA1, t288SHA1ShortMsg)
{
    ASSERT_EQ("e21e22b89c1bb944a32932e6b2a2f20d491982c3",
            sha1_dehex("f17af27d776ec82a257d8d46d2b46b639462c56984cc1be9c1222eadb8b26594a25c709d"));
}

TEST(SHA1, t296SHA1ShortMsg)
{
    ASSERT_EQ("575323a9661f5d28387964d2ba6ab92c17d05a8a",
            sha1_dehex("b13ce635d6f8758143ffb114f2f601cb20b6276951416a2f94fbf4ad081779d79f4f195b22"));
}

TEST(SHA1, t304SHA1ShortMsg)
{
    ASSERT_EQ("feb44494af72f245bfe68e86c4d7986d57c11db7",
            sha1_dehex("5498793f60916ff1c918dde572cdea76da8629ba4ead6d065de3dfb48de94d234cc1c5002910"));
}

TEST(SHA1, t312SHA1ShortMsg)
{
    ASSERT_EQ("cff2290b3648ba2831b98dde436a72f9ebf51eee",
            sha1_dehex("498a1e0b39fa49582ae688cd715c86fbaf8a81b8b11b4d1594c49c902d197c8ba8a621fd6e3be5"));
}

TEST(SHA1, t320SHA1ShortMsg)
{
    ASSERT_EQ("9b4efe9d27b965905b0c3dab67b8d7c9ebacd56c",
            sha1_dehex("3a36ae71521f9af628b3e34dcb0d4513f84c78ee49f10416a98857150b8b15cb5c83afb4b570376e"));
}

TEST(SHA1, t328SHA1ShortMsg)
{
    ASSERT_EQ("afedb0ff156205bcd831cbdbda43db8b0588c113",
            sha1_dehex("dcc76b40ae0ea3ba253e92ac50fcde791662c5b6c948538cffc2d95e9de99cac34dfca38910db2678f"));
}

TEST(SHA1, t336SHA1ShortMsg)
{
    ASSERT_EQ("8deb1e858f88293a5e5e4d521a34b2a4efa70fc4",
            sha1_dehex("5b5ec6ec4fd3ad9c4906f65c747fd4233c11a1736b6b228b92e90cddabb0c7c2fcf9716d3fad261dff33"));
}

TEST(SHA1, t344SHA1ShortMsg)
{
    ASSERT_EQ("95cbdac0f74afa69cebd0e5c7defbc6faf0cbeaf",
            sha1_dehex("df48a37b29b1d6de4e94717d60cdb4293fcf170bba388bddf7a9035a15d433f20fd697c3e4c8b8c5f590ab"));
}

TEST(SHA1, t352SHA1ShortMsg)
{
    ASSERT_EQ("f0307bcb92842e5ae0cd4f4f14f3df7f877fbef2",
            sha1_dehex("1f179b3b82250a65e1b0aee949e218e2f45c7a8dbfd6ba08de05c55acfc226b48c68d7f7057e5675cd96fcfc"));
}

TEST(SHA1, t360SHA1ShortMsg)
{
    ASSERT_EQ("7b13bb0dbf14964bd63b133ac85e22100542ef55",
            sha1_dehex("ee3d72da3a44d971578972a8e6780ce64941267e0f7d0179b214fa97855e1790e888e09fbe3a70412176cb3b54"));
}

TEST(SHA1, t368SHA1ShortMsg)
{
    ASSERT_EQ("c314d2b6cf439be678d2a74e890d96cfac1c02ed",
            sha1_dehex("d4d4c7843d312b30f610b3682254c8be96d5f6684503f8fbfbcd15774fc1b084d3741afb8d24aaa8ab9c104f7258"));
}

TEST(SHA1, t376SHA1ShortMsg)
{
    ASSERT_EQ("4d0be361e410b47a9d67d8ce0bb6a8e01c53c078",
            sha1_dehex("32c094944f5936a190a0877fb9178a7bf60ceae36fd530671c5b38c5dbd5e6a6c0d615c2ac8ad04b213cc589541cf6"));
}

TEST(SHA1, t384SHA1ShortMsg)
{
    ASSERT_EQ("e5353431ffae097f675cbf498869f6fbb6e1c9f2",
            sha1_dehex("e5d3180c14bf27a5409fa12b104a8fd7e9639609bfde6ee82bbf9648be2546d29688a65e2e3f3da47a45ac14343c9c02"));
}

TEST(SHA1, t392SHA1ShortMsg)
{
    ASSERT_EQ("b8720a7068a085c018ab18961de2765aa6cd9ac4",
            sha1_dehex("e7b6e4b69f724327e41e1188a37f4fe38b1dba19cbf5a7311d6e32f1038e97ab506ee05aebebc1eed09fc0e357109818b9"));
}

TEST(SHA1, t400SHA1ShortMsg)
{
    ASSERT_EQ("b0732181568543ba85f2b6da602b4b065d9931aa",
            sha1_dehex("bc880cb83b8ac68ef2fedc2da95e7677ce2aa18b0e2d8b322701f67af7d5e7a0d96e9e33326ccb7747cfff0852b961bfd475"));
}

TEST(SHA1, t408SHA1ShortMsg)
{
    ASSERT_EQ("9c22674cf3222c3ba921672694aafee4ce67b96b",
            sha1_dehex("235ea9c2ba7af25400f2e98a47a291b0bccdaad63faa2475721fda5510cc7dad814bce8dabb611790a6abe56030b798b75c944"));
}

TEST(SHA1, t416SHA1ShortMsg)
{
    ASSERT_EQ("d128335f4cecca9066cdae08958ce656ff0b4cfc",
            sha1_dehex("07e3e29fed63104b8410f323b975fd9fba53f636af8c4e68a53fb202ca35dd9ee07cb169ec5186292e44c27e5696a967f5e67709"));
}

TEST(SHA1, t424SHA1ShortMsg)
{
    ASSERT_EQ("0b67c57ac578de88a2ae055caeaec8bb9b0085a0",
            sha1_dehex("65d2a1dd60a517eb27bfbf530cf6a5458f9d5f4730058bd9814379547f34241822bf67e6335a6d8b5ed06abf8841884c636a25733f"));
}

TEST(SHA1, t432SHA1ShortMsg)
{
    ASSERT_EQ("c766f912a89d4ccda88e0cce6a713ef5f178b596",
            sha1_dehex("dcc86b3bd461615bab739d8daafac231c0f462e819ad29f9f14058f3ab5b75941d4241ea2f17ebb8a458831b37a9b16dead4a76a9b0e"));
}

TEST(SHA1, t440SHA1ShortMsg)
{
    ASSERT_EQ("9aa3925a9dcb177b15ccff9b78e70cf344858779",
            sha1_dehex("4627d54f0568dc126b62a8c35fb46a9ac5024400f2995e51635636e1afc4373dbb848eb32df23914230560b82477e9c3572647a7f2bb92"));
}

TEST(SHA1, t448SHA1ShortMsg)
{
    ASSERT_EQ("4811fa30042fc076acf37c8e2274d025307e5943",
            sha1_dehex("ba531affd4381168ef24d8b275a84d9254c7f5cc55fded53aa8024b2c5c5c8aa7146fe1d1b83d62b70467e9a2e2cb67b3361830adbab28d7"));
}

TEST(SHA1, t456SHA1ShortMsg)
{
    ASSERT_EQ("6743018450c9730761ee2b130df9b91c1e118150",
            sha1_dehex("8764dcbcf89dcf4282eb644e3d568bdccb4b13508bfa7bfe0ffc05efd1390be22109969262992d377691eb4f77f3d59ea8466a74abf57b2ef4"));
}

TEST(SHA1, t464SHA1ShortMsg)
{
    ASSERT_EQ("71ad4a19d37d92a5e6ef3694ddbeb5aa61ada645",
            sha1_dehex("497d9df9ddb554f3d17870b1a31986c1be277bc44feff713544217a9f579623d18b5ffae306c25a45521d2759a72c0459b58957255ab592f3be4"));
}

TEST(SHA1, t472SHA1ShortMsg)
{
    ASSERT_EQ("a7d9dc68dacefb7d6116186048cb355cc548e11d",
            sha1_dehex("72c3c2e065aefa8d9f7a65229e818176eef05da83f835107ba90ec2e95472e73e538f783b416c04654ba8909f26a12db6e5c4e376b7615e4a25819"));
}

TEST(SHA1, t480SHA1ShortMsg)
{
    ASSERT_EQ("142e429f0522ba5abf5131fa81df82d355b96909",
            sha1_dehex("7cc9894454d0055ab5069a33984e2f712bef7e3124960d33559f5f3b81906bb66fe64da13c153ca7f5cabc89667314c32c01036d12ecaf5f9a78de98"));
}

TEST(SHA1, t488SHA1ShortMsg)
{
    ASSERT_EQ("ef72db70dcbcab991e9637976c6faf00d22caae9",
            sha1_dehex("74e8404d5a453c5f4d306f2cfa338ca65501c840ddab3fb82117933483afd6913c56aaf8a0a0a6b2a342fc3d9dc7599f4a850dfa15d06c61966d74ea59"));
}

TEST(SHA1, t496SHA1ShortMsg)
{
    ASSERT_EQ("f220a7457f4588d639dc21407c942e9843f8e26b",
            sha1_dehex("46fe5ed326c8fe376fcc92dc9e2714e2240d3253b105adfbb256ff7a19bc40975c604ad7c0071c4fd78a7cb64786e1bece548fa4833c04065fe593f6fb10"));
}

TEST(SHA1, t504SHA1ShortMsg)
{
    ASSERT_EQ("ddd2117b6e309c233ede85f962a0c2fc215e5c69",
            sha1_dehex("836dfa2524d621cf07c3d2908835de859e549d35030433c796b81272fd8bc0348e8ddbc7705a5ad1fdf2155b6bc48884ac0cd376925f069a37849c089c8645"));
}

TEST(SHA1, t512SHA1ShortMsg)
{
    ASSERT_EQ("a3054427cdb13f164a610b348702724c808a0dcc",
            sha1_dehex("7e3a4c325cb9c52b88387f93d01ae86d42098f5efa7f9457388b5e74b6d28b2438d42d8b64703324d4aa25ab6aad153ae30cd2b2af4d5e5c00a8a2d0220c6116"));
}

