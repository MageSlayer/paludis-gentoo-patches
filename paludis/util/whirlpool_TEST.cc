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

#include <paludis/util/whirlpool.hh>

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

    std::string whirlpool(const std::string & data)
    {
        std::stringstream ss(data);
        Whirlpool s(ss);
        return s.hexsum();
    }
}

TEST(Whirlpool, t0)
{
    ASSERT_EQ("19fa61d75522a4669b44e39c1d2e1726c530232130d407f89afee0964997f7a73e83be698b288febcf88e3e03c4f0757ea8964e59b63d93708b138cc42a66eb3",
            whirlpool(""));
}

TEST(Whirlpool, t1)
{
    ASSERT_EQ("8aca2602792aec6f11a67206531fb7d7f0dff59413145e6973c45001d0087b42d11bc645413aeff63a42391a39145a591a92200d560195e53b478584fdae231a",
            whirlpool("a"));
}

TEST(Whirlpool, t2)
{
    ASSERT_EQ("4e2448a4c6f486bb16b6562c73b4020bf3043e3a731bce721ae1b303d97e6d4c7181eebdb6c57e277d0e34957114cbd6c797fc9d95d8b582d225292076d4eef5",
            whirlpool("abc"));
}

TEST(Whirlpool, t3)
{
    ASSERT_EQ("378c84a4126e2dc6e56dcc7458377aac838d00032230f53ce1f5700c0ffb4d3b8421557659ef55c106b4b52ac5a4aaa692ed920052838f3362e86dbd37a8903e",
            whirlpool("message digest"));
}

TEST(Whirlpool, t4)
{
    ASSERT_EQ("f1d754662636ffe92c82ebb9212a484a8d38631ead4238f5442ee13b8054e41b08bf2a9251c30b6a0b8aae86177ab4a6f68f673e7207865d5d9819a3dba4eb3b",
            whirlpool("abcdefghijklmnopqrstuvwxyz"));
}

TEST(Whirlpool, t5)
{
    ASSERT_EQ("dc37e008cf9ee69bf11f00ed9aba26901dd7c28cdec066cc6af42e40f82f3a1e08eba26629129d8fb7cb57211b9281a65517cc879d7b962142c65f5a7af01467",
            whirlpool("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"));
}

TEST(Whirlpool, t6)
{
    ASSERT_EQ("466ef18babb0154d25b9d38a6414f5c08784372bccb204d6549c4afadb6014294d5bd8df2a6c44e538cd047b2681a51a2c60481e88c5a20b2c2a80cf3a9a083b",
            whirlpool("12345678901234567890123456789012345678901234567890123456789012345678901234567890"));
}

TEST(Whirlpool, t7)
{
    ASSERT_EQ("2a987ea40f917061f5d6f0a0e4644f488a7a5a52deee656207c562f988e95c6916bdc8031bc5be1b7b947639fe050b56939baaa0adff9ae6745b7b181c3be3fd",
            whirlpool("abcdbcdecdefdefgefghfghighijhijk"));
}

TEST(Whirlpool, t8)
{
    ASSERT_EQ("0c99005beb57eff50a7cf005560ddf5d29057fd86b20bfd62deca0f1ccea4af51fc15490eddc47af32bb2b66c34ff9ad8c6008ad677f77126953b226e4ed8b01",
            whirlpool(std::string(1000000, 'a')));
}

TEST(Whirlpool, z8)
{
    ASSERT_EQ("4d9444c212955963d425a410176fccfb74161e6839692b4c11fde2ed6eb559efe0560c39a7b61d5a8bcabd6817a3135af80f342a4942ccaae745abddfb6afed0",
            whirlpool(std::string(1, '\0')));
}

TEST(Whirlpool, z16)
{
    ASSERT_EQ("8bdc9d4471d0dabd8812098b8cbdf5090beddb3d582917a61e176e3d22529d753fed9a37990ca18583855efbc4f26e88f62002f67722eb05f74c7ea5e07013f5",
            whirlpool(std::string(2, '\0')));
}

TEST(Whirlpool, z24)
{
    ASSERT_EQ("86aabfd4a83c3551cc0a63185616acb41cdfa96118f1ffb28376b41067efa25fb6c889662435bfc11a4f0936be6bcc2c3e905c4686db06159c40e4dd67dd983f",
            whirlpool(std::string(3, '\0')));
}

TEST(Whirlpool, z32)
{
    ASSERT_EQ("4ed6b52e915f09a803677c3131f7b34655d32817505d89a8cc07ed073ca3fedddd4a57cc53696027e824ab9822630087657c6fc6a28836cf1f252ed204bca576",
            whirlpool(std::string(4, '\0')));
}

TEST(Whirlpool, z40)
{
    ASSERT_EQ("4a1d1d8380f38896b6fc5788c559f92727acfd4dfa7081c72302b17e1ed437b30a24cfd75a16fd71b6bf5aa7ae5c7084594e3003a0b71584dc993681f902df6f",
            whirlpool(std::string(5, '\0')));
}

TEST(Whirlpool, z48)
{
    ASSERT_EQ("a2a379b0900a3c51809f4216aa3347fec486d50ec7376553349c5cf2a767049a87bf1ac4642185144924259ecf6b5c3b48b55a20565de289361e8ae5eafc5802",
            whirlpool(std::string(6, '\0')));
}

TEST(Whirlpool, z56)
{
    ASSERT_EQ("23eb3e26a1543558672f29e196304cd778ea459cc8e38d199de0cc748bd32d58090fadb39e7c7b6954322de990556d001ba457061c4367c6fa5d6961f1046e2f",
            whirlpool(std::string(7, '\0')));
}

TEST(Whirlpool, z64)
{
    ASSERT_EQ("7207be34fee657189af2748358f46c23175c1dcdd6741a9bdb139aeb255b618b711775fc258ac0fa53c350305862415ea121c65bf9e2fae06cbd81355d928ad7",
            whirlpool(std::string(8, '\0')));
}

TEST(Whirlpool, z72)
{
    ASSERT_EQ("fef7d0be035d1860e95644864b199c3a94eb23ab7920134b73239a320eb7cae450092bc4ba8b9809e20c33937c37c52b52ca90241657ffd0816420c01f4fada8",
            whirlpool(std::string(9, '\0')));
}

TEST(Whirlpool, z80)
{
    ASSERT_EQ("caafb557aef0fae9f20bcccda7f3dc769c478a70508f4f2d180303598276934c410bd3d17627159a9c55b5265b516ba7f3eef67fbb08d9f22e585bc45964c4d1",
            whirlpool(std::string(10, '\0')));
}

TEST(Whirlpool, z88)
{
    ASSERT_EQ("8fe2b488ca099db8e421768e1e7e0193ffaa3000e8281403795575fe7d03bd87298c4f64b1c4311093e43de4d80049645782ee268c3653c7a5c13da3773d5564",
            whirlpool(std::string(11, '\0')));
}

TEST(Whirlpool, z96)
{
    ASSERT_EQ("2e065509d50d6135e4c4bd238b0e4138391c98082a596e76bdf1fb925b5069adca9548f833296da968573b965f79cd806624fb8c7e06f728aa04f24185878933",
            whirlpool(std::string(12, '\0')));
}

TEST(Whirlpool, z104)
{
    ASSERT_EQ("51fdcea18fdb55a7497e06f33c74194daf0138411075749ed5f00eba59e7f850da6d36b7ee851beabcb449537910a73b8878c88a868a7168c677ac4601ac71d8",
            whirlpool(std::string(13, '\0')));
}

TEST(Whirlpool, z112)
{
    ASSERT_EQ("2e19c1695b708d20f9b2e4e3cc2df40b9ca4dc8c709a148b1f0da5934116050d12c4ab44d0cc6cc82e86c0f140ddd4a7a1152b2c42d150965224fdd3fe2c5a94",
            whirlpool(std::string(14, '\0')));
}

TEST(Whirlpool, z120)
{
    ASSERT_EQ("b501bc7a16fc812eda69ad0f8453ddc57278478cb1a704b1977e22090e82e7a280c1bf20bf130b98e7b3b2f4eda87ef03a4900dc7c0427da4e28e422186beecc",
            whirlpool(std::string(15, '\0')));
}

TEST(Whirlpool, z128)
{
    ASSERT_EQ("a1f020a9b16c1318660d573d7e5203c14438196f247a55f09a6bf67211bcfdae71077cdf5b0e243a61ce10ffc561da4c0462600e11f176c2537da22bfc9386b9",
            whirlpool(std::string(16, '\0')));
}

TEST(Whirlpool, z136)
{
    ASSERT_EQ("a4e587d1bdf7c7a9897dc2a002e0c3240aac073dabd4b04b2cec2ae8a5751d3059b12d78efcb1b9b7f18bb69ab7f7c9f89f6a1c5e59325bc5e827a1cdcf990f5",
            whirlpool(std::string(17, '\0')));
}

TEST(Whirlpool, z144)
{
    ASSERT_EQ("9a7e6b5e34a4ff26b990a3bd844e059b39b7a58eefe37ef96c595cf6a52eb697f8263cdb34566ffa3387cc7895cab74196ef0e42162fde8e2b3aaf435aa1d07b",
            whirlpool(std::string(18, '\0')));
}

TEST(Whirlpool, z152)
{
    ASSERT_EQ("a979eefd43c26fe934399d26e18714b59d53572f9dc265937c1a24b48480ccd7c409f8a0322156cc602afa2a3df8ba5e1c86fc00de2a420b9412aa48af7f10d0",
            whirlpool(std::string(19, '\0')));
}

TEST(Whirlpool, z160)
{
    ASSERT_EQ("212d82efab4a9c99b4559ed090830db4e70f469d98927453d8ca2956afb4f92fa79174a552e7a17c1c87e640117d67b0ed377548ec64df6e534593c218369ef0",
            whirlpool(std::string(20, '\0')));
}

TEST(Whirlpool, z168)
{
    ASSERT_EQ("fce5e68f1e2ae9aea93514e4cb2b29eddb4c6a3955244d554f99aea42c61db3e761f9bead5c05470c20cfbf605c1ea4b0f3877487cac9b8177bd3df3aa67cc3a",
            whirlpool(std::string(21, '\0')));
}

TEST(Whirlpool, z176)
{
    ASSERT_EQ("c8f99a4fc37587d3cf36c65fc0249648866fe4c2a99ae47f0ee0f3d3270c3134a487182847afd3d056a15a9e35c5a2fd76947ac1627514a8d1f2ecc057cfcd5e",
            whirlpool(std::string(22, '\0')));
}

TEST(Whirlpool, z184)
{
    ASSERT_EQ("98b42ee3e98849bf1f6d8589f129ab12f055883c520e6001f536e67ea6bd34db4bd610b415f455867b6ab1c31a9ddc37330a57fb6b0f1f20efba12ff560a2e16",
            whirlpool(std::string(23, '\0')));
}

TEST(Whirlpool, z192)
{
    ASSERT_EQ("4b8e81860b0035eb2189eae73e23c9f34b3d993c3387ab276c2f47aada04391d609c70611b3420a72af1af12d4733f40457c0f75ac656c4e798e8b29556a8497",
            whirlpool(std::string(24, '\0')));
}

TEST(Whirlpool, z200)
{
    ASSERT_EQ("3c0863d6bd033ff0ea6801d7c11a7f4f89c3f0678ebc1981d0e98ef9f435429dd25e1238aacfb47bfd6bdd4016981a2832c8b5687c654c12dba0c40c806d6a79",
            whirlpool(std::string(25, '\0')));
}

TEST(Whirlpool, z208)
{
    ASSERT_EQ("1243171d17b72affa1beb5fa7a57cbd6432447476da2e785dd85946ba7d1836d81fa7048d7870d10072c56970d65ba2b759181f6115693deb2cf88ddf13878dd",
            whirlpool(std::string(26, '\0')));
}

TEST(Whirlpool, z216)
{
    ASSERT_EQ("09b029e928fe1ebda47fd79e32e3201ded573890305fa7193c185d741c1dede443e55ee21c1ea5d608d03bf04e1f29832e70c22bc4962ff56d24b333f272dbc4",
            whirlpool(std::string(27, '\0')));
}

TEST(Whirlpool, z224)
{
    ASSERT_EQ("97d10c6aa678886e186551df174bd9bfda299c1cfa0a926eda92290f99fc05d9e23ec4897686ea7eb576686a5c27b10c23c76e303410bbaed0e165e2a8d362dd",
            whirlpool(std::string(28, '\0')));
}

TEST(Whirlpool, z232)
{
    ASSERT_EQ("799324597820515a94d854031e00312b91ac8f0aeb7ec79cc87503e2fbec48ddc1179f0f4d25326a59ec82a84057c04534b032b66924b925520a8ce0e4449649",
            whirlpool(std::string(29, '\0')));
}

TEST(Whirlpool, z240)
{
    ASSERT_EQ("b5dadbdd162118d69ca21825c05d4fa4ee9195cbac15d1f649c235043c9bc1e3ff8920920b2843503d70deb087d9698dd6436698e1dc1f54216bdc6188f13acf",
            whirlpool(std::string(30, '\0')));
}

TEST(Whirlpool, z248)
{
    ASSERT_EQ("3e3f188f8febbeb17a933feaf7fe53a4858d80c915ad6a1418f0318e68d49b4e459223cd414e0fbc8a57578fd755d86e827abef4070fc1503e25d99e382f72ba",
            whirlpool(std::string(31, '\0')));
}

TEST(Whirlpool, z256)
{
    ASSERT_EQ("961b5f299f750f880fca004bdf2882e2fe1b491b0c0ee7e2b514c5dfdd53292dbdbee17e6d3bb5824cdec1867cc7090963be8fff0c1d8ed5864e07cacb50d68a",
            whirlpool(std::string(32, '\0')));
}

TEST(Whirlpool, z264)
{
    ASSERT_EQ("54a9c8784dbf6c2618ca32057b76b9d6733c19f4a377cb7e892d057bf73e83fbaf6ac147a65fef0991dc296955440ad0b793f81c5cf71e29669ce3f19195aaf7",
            whirlpool(std::string(33, '\0')));
}

TEST(Whirlpool, z272)
{
    ASSERT_EQ("7fd61031451a6635f4a8bf56f9e3b6c699c77d573bbe2f1f4938a0630b7d6fb64f202f5e8c4dafb23d5b4481089bc198d0324178a3c9a625e101744ac517b681",
            whirlpool(std::string(34, '\0')));
}

TEST(Whirlpool, z280)
{
    ASSERT_EQ("bb00019c930b4f8b3c035861174b2e8cdb94cdfa4f5b08082944f86505cd6eb18f7c95f1e031ab42c510187927f9fedae3085a8e1918fb1acde7a0fc6cf3c62f",
            whirlpool(std::string(35, '\0')));
}

TEST(Whirlpool, z288)
{
    ASSERT_EQ("9e74ee84973257cd3507387da7f4388ba46c55fa55ec0b991ec0ef01c8ed629eabd003f585c62d46d70a760b51ffba48f4aa1d10519ddb09ad95e536f5d9faac",
            whirlpool(std::string(36, '\0')));
}

TEST(Whirlpool, z296)
{
    ASSERT_EQ("012cd71c3d938304bd65dcc705f1ea511ff1981aaf6b641f9e3714b1fec010ae8bedd39fb497cc1affcde1dd33569eb970c3dd7bb073ecd130028bb0f76b356c",
            whirlpool(std::string(37, '\0')));
}

TEST(Whirlpool, z304)
{
    ASSERT_EQ("f7c0484614245f5cf54dc03d14e00ac30a86de8fe9e3de1741c75d3c7202570f2a5b70618fbb1c851285ec2c4cb961254ddade9ba7639993b12efa1d66e7c61d",
            whirlpool(std::string(38, '\0')));
}

TEST(Whirlpool, z312)
{
    ASSERT_EQ("25f7cf0ceef2a61a657d852140daf9f19567534b34e1928c26fc6499a29e896c8c7a5c2bb2393fcca4c1ef811ef530954b758fbe2120303be63394fa340ae544",
            whirlpool(std::string(39, '\0')));
}

TEST(Whirlpool, z320)
{
    ASSERT_EQ("4a9ce34ecdcfb9a5082ded6f56c24ae155ab3225dfd6ad39fa79df6dc9b58fbe346c40b1aa58af7b836e0e6346bcd0b0e686eba4ee86c4cc77eb7de1d16d43f0",
            whirlpool(std::string(40, '\0')));
}

TEST(Whirlpool, z328)
{
    ASSERT_EQ("eeabdb9655fa9d652cce83fc697b8534a3d92cfa0c2cfabcf3393be1ad9d6589fd6eeebceac9bf3ef3251a7811b793fd03f17d2fa00edb17c423394dc6bedda1",
            whirlpool(std::string(41, '\0')));
}

TEST(Whirlpool, z336)
{
    ASSERT_EQ("9ce6a4a20a719186c8946d824fb2be78260fed469a0a9946a44cfe3a43d5e11c642407b473fe72ef087c45edaeb2c137ff379a7f60747895ae03f6fc337cacd0",
            whirlpool(std::string(42, '\0')));
}

TEST(Whirlpool, z344)
{
    ASSERT_EQ("af2d773502eef6e86448d5b465d056b9530877ffa1a97bbedc17c119e48bb3be0c79df54940d10163436c8841fbbd565ddb7a8de850445ef93ab5d2e8c19d4c1",
            whirlpool(std::string(43, '\0')));
}

TEST(Whirlpool, z352)
{
    ASSERT_EQ("e67ab8b825252076477d3282e69b006f10d76a841318dcba82de290928f46ebbcc5a6e3ebfb8aefc8f997928f2adf351376c214eae23cd8e1632b7acae102959",
            whirlpool(std::string(44, '\0')));
}

TEST(Whirlpool, z360)
{
    ASSERT_EQ("cf1476329ec7be76c0808249e39a9c0d05b0010eda8d9f21ce4a0794698d3016f28830fe1628061a9c2db8e0155092b3c5c606e2c3900f8112cf2f03cca96edd",
            whirlpool(std::string(45, '\0')));
}

TEST(Whirlpool, z368)
{
    ASSERT_EQ("b0d5b4e4caa9f3360d016ed86fdb2b8ba579ae7df45c63231774cbd369aedb065cd32cac6914b629a76242d8bae5ddcec8decad1ccc3cf2fff044d1f84415975",
            whirlpool(std::string(46, '\0')));
}

TEST(Whirlpool, z376)
{
    ASSERT_EQ("894e242d8ac2bffe0bb2f5be206796dd7666bf8c113547c064e6b7755e0b27af4126ec1301e8ddab0fd1ca202ffcd71287bb7fdbcba73f87fcfb90fea846129b",
            whirlpool(std::string(47, '\0')));
}

TEST(Whirlpool, z384)
{
    ASSERT_EQ("2de58b719aaf6c07d8d4c3d2f61a3950ed877a8619328177b2a3f1deae6122797bfeb0ad190845805a129eacd11be90d63367d34f19734909a1af19a951e8359",
            whirlpool(std::string(48, '\0')));
}

TEST(Whirlpool, z392)
{
    ASSERT_EQ("dc40979db11829c0e8513c23e92de628fd53d03a35c281393f35f5952c3f2e062100e1bf077f4ca0c24f26d250829daaaad9f33b5caa9c081ba2b4910648810a",
            whirlpool(std::string(49, '\0')));
}

TEST(Whirlpool, z400)
{
    ASSERT_EQ("3cf2523477a56d6e8c8d2de34f574ce2ca1d13104bc209df6e0b2f587680ef038713216b4b869bb92e8d13293b8861b75a1fba392793f0b58dd21f65c06fb8b4",
            whirlpool(std::string(50, '\0')));
}

TEST(Whirlpool, z408)
{
    ASSERT_EQ("4ce343439d4f8a3af5b2027fad0764d28ba561c7ff02ed53fe1d1de47a6b7e89f3f3f259c7c46968fe0c065e25ac91f0ad493b28f662792f1d561fce029bfb5b",
            whirlpool(std::string(51, '\0')));
}

TEST(Whirlpool, z416)
{
    ASSERT_EQ("e1c06f1951ee384bea3a2ee974b81fd4674e65e89d09e42c0309d2df9e2aa004ccecd3dba2ae5e31d56f83cf7d84036b081713dd70e36881f2acf7312e6119b0",
            whirlpool(std::string(52, '\0')));
}

TEST(Whirlpool, z424)
{
    ASSERT_EQ("586b6f60d778cd32d7fd4f6166d6c06e2f730477787f8f0259264d121b9aeb3963b38a9250adbcb6ac7029051acfaa141406efc19560b3ed33e86fab7ac1fea2",
            whirlpool(std::string(53, '\0')));
}

TEST(Whirlpool, z432)
{
    ASSERT_EQ("edc165f52b5e493fa4b39ad06339be79b1afdb204bfc792011200ac951468eedfe6a73da03331ee95beccf01e2260a30d2d36679e184d6e6417fc01647731076",
            whirlpool(std::string(54, '\0')));
}

TEST(Whirlpool, z440)
{
    ASSERT_EQ("d64ab30797e2d5e986b7b6fe99fc5aa79918d4423a5808d64f8042a9489485e6619f57d2865091b363a3b9b788b16690fba45e8be352fc9517b58a05937383e7",
            whirlpool(std::string(55, '\0')));
}

TEST(Whirlpool, z448)
{
    ASSERT_EQ("0b3f5378ebed2bf4d7be3cfd818c1b03b6bb03d346948b04f4f40c726f0758702a0f1e225880e38dd5f6ed6de9b1e961e49fc1318d7cb74822f3d0e2e9a7e7b0",
            whirlpool(std::string(56, '\0')));
}

TEST(Whirlpool, z456)
{
    ASSERT_EQ("8e2f3c36f961d08f0ee389412b434595914821829f21a213366f898516ec97bda3d13814fcd8c5fa9ab9ed92246c73824d54a36cb19a9c986f4b7e41ebe12ed3",
            whirlpool(std::string(57, '\0')));
}

TEST(Whirlpool, z464)
{
    ASSERT_EQ("a516c5c831f4e56bf8ed0b7c193c967536f1063f23085fb3ff7098dd3070dc7fa6f9b26c5c26942cd4ff8189f777b66bff9385e6136df6a2b0d5bcd3c8ad2878",
            whirlpool(std::string(58, '\0')));
}

TEST(Whirlpool, z472)
{
    ASSERT_EQ("30e347272c63d340ed0781cca5270894db39e7a97fb5f5e9ee2ad988a127eea4c7eee0f1c8a33dd5ed8d01f91e18cb433434a35d82bc4fde5f2fcd91a657d84e",
            whirlpool(std::string(59, '\0')));
}

TEST(Whirlpool, z480)
{
    ASSERT_EQ("9417f6af2a8325268a0998adb4196189d7140797fb757d6f081defc5b5e4d37d8f45830eec029e2e45bb6b9e019fac97bd3ee79db9a8a4fdede69d66d63fa009",
            whirlpool(std::string(60, '\0')));
}

TEST(Whirlpool, z488)
{
    ASSERT_EQ("d059f4847864c24d11511e5589005c1d5d550111e30c557be0b1f02ab7f2495e44799253724e19026bfa6b0e544d35747af75044237a25ee9a2315cc9e3e18d1",
            whirlpool(std::string(61, '\0')));
}

TEST(Whirlpool, z496)
{
    ASSERT_EQ("6119c33347ad30067e75d33f5990b4ecdae1fc4e42aaf17888d2526ace673223a483875e1854f9f60aef51678be2d52b6b4880b7e07395431845b6736881ec45",
            whirlpool(std::string(62, '\0')));
}

TEST(Whirlpool, z504)
{
    ASSERT_EQ("0fac8ef35b4f7be6e1ac1580eef1a8a90f81e3043cc45e918eda3543d9d4e9e3aa16c8e5e71bf792f81ca67f625ff98f9ca1eab8fa9b78c9d2e771cf249768dd",
            whirlpool(std::string(63, '\0')));
}

TEST(Whirlpool, z512)
{
    ASSERT_EQ("15cfa7c1df8e0d6753d9a9aed0642867e26bb3cf11df7dac96f60c274e060fda941ec41eaff5f7375f3839632516ae9a831d9f2fbe2bd0ff02e9cf16e99ebd03",
            whirlpool(std::string(64, '\0')));
}

TEST(Whirlpool, z520)
{
    ASSERT_EQ("85e124c4415bcf4319543e3a63ff571d09354ceebee1e325308c9069f43e2ae4d0e51d4eb1e8642870194e9530d8d8af6589d1bf6949ddf90a7f1208623795b9",
            whirlpool(std::string(65, '\0')));
}

TEST(Whirlpool, z528)
{
    ASSERT_EQ("fce5e522e77e3f85b81565c43beb03b63231db8ea7940251bb5b33484353be66c8d6c314a661a89cfd9088b4e54762a21c246b7c662903ad2de5ab7c12a1e842",
            whirlpool(std::string(66, '\0')));
}

TEST(Whirlpool, z536)
{
    ASSERT_EQ("75e11b045a3ea2af6b5cb47a602836ad1e3c8b72d242acca4be83cfc48826b4554c2f3f804b29181e71bed658ff366f66879f2fea08bda0c224f41db6b144871",
            whirlpool(std::string(67, '\0')));
}

TEST(Whirlpool, z544)
{
    ASSERT_EQ("70b34639476a82db9086fad0810a0610cd778d1432e58a173f60d0480992ed1f04ae4dbffd8471efce2cc39576b6367c79863e844d198763c2e1a5f63a1747aa",
            whirlpool(std::string(68, '\0')));
}

TEST(Whirlpool, z552)
{
    ASSERT_EQ("d2d92a49569825acec8a67abc1f71adaba3d1bef1137660e23e8950bc9f42ab571bdef48835e488c02d2a8a80bf94fd175f99e0b27db4bdf9596d64b39b1cf8d",
            whirlpool(std::string(69, '\0')));
}

TEST(Whirlpool, z560)
{
    ASSERT_EQ("edf51be6cf0ae98337dd78747f7d0809585be1ee3c7b09dc439b513c7b2a0387d1950294ec73ba68308e30e2d4bdb679849a054a16b05333036da3cd59b76329",
            whirlpool(std::string(70, '\0')));
}

TEST(Whirlpool, z568)
{
    ASSERT_EQ("e3ba063cb8efcdf32b0b38d929b88f28dd64a9a88df9958db2fd8074c24597c981b358882ff09a501b1b7037047fb96b716e5047f3630ddb4e5a39aa862c23f8",
            whirlpool(std::string(71, '\0')));
}

TEST(Whirlpool, z576)
{
    ASSERT_EQ("a75bd3d61e3a0820fdb151a65fbe42bfb2e247fdba142a516374ce8305e4830049a69f37048b4c098a4415745e753bdd679b019b40cf93bbead892045bd6fa3d",
            whirlpool(std::string(72, '\0')));
}

TEST(Whirlpool, z584)
{
    ASSERT_EQ("d45072f92f621082b1c5775f25e8cf8c93dd853219c7039355d308e9cc981794f2a593a71aea900487858e93860dea4898510e4ea48053404a9075b677e64531",
            whirlpool(std::string(73, '\0')));
}

TEST(Whirlpool, z592)
{
    ASSERT_EQ("6a9d999c3cb074e3ea1605f68c01fa5b402b382f6953a5c973da9e888e0354c815d26fd72448aeb9aaab0a24685dbf8c096841f8d89606eabd58cd14ca6fbe74",
            whirlpool(std::string(74, '\0')));
}

TEST(Whirlpool, z600)
{
    ASSERT_EQ("05b9648219a7e9d839084aaa64c4b2b74db42f8bd435a5b69079df2335ec24547b867ef9b567c43acd70fa0d010c48e4695833da57fe0c60ad9111958a3b4116",
            whirlpool(std::string(75, '\0')));
}

TEST(Whirlpool, z608)
{
    ASSERT_EQ("6c243e768b3f7c4fc5c4dcb3bf6fa4fb44767615ebaeda0a572cd12624c29e508e4c814bea8f21f0f2eb69d0045297960e1ef5d3bee86858590f55d303cf1d87",
            whirlpool(std::string(76, '\0')));
}

TEST(Whirlpool, z616)
{
    ASSERT_EQ("c98ebbaae2df1fcbb406b6dd831bb68ca317924a7563152a2ce61dd101bfef277a0f7be5fa040c9f41952052c58c3e32e2b07d7c0d39fbd20950a149595c82f8",
            whirlpool(std::string(77, '\0')));
}

TEST(Whirlpool, z624)
{
    ASSERT_EQ("f739c74b2aea5c89781c5de3b9ee9cae1ccb71e6a8824b1c42e4ba0f35efb0a479ade849196af1a5ea74d3ba6ab1e05d330da469a96bfcbce7f9097fbfd3ed08",
            whirlpool(std::string(78, '\0')));
}

TEST(Whirlpool, z632)
{
    ASSERT_EQ("cc5c707f98cfb307084d2202e8183f43014fc73b54d04592ae7ef6a3d95fc25ca04c8004f4d9fc4975cb5fb5921ee9e812c720474b5e4ba3dfdac77b9bd62872",
            whirlpool(std::string(79, '\0')));
}

TEST(Whirlpool, z640)
{
    ASSERT_EQ("036b2bf483d53db440673faa192985c305b5f35c6c4f14d2bea22be44a843eaf81f620f47493d29d47a99292b26706e3d82db753ff1eff72e5912e904c652b01",
            whirlpool(std::string(80, '\0')));
}

TEST(Whirlpool, z648)
{
    ASSERT_EQ("005f1d907c6be7a0063dc782426103d4bc64b12fa12addb2fe3129738349adb31dd92eb5fbf76538d1e16b4d7e0dea61863218231907e280d0ca9ab8b1264444",
            whirlpool(std::string(81, '\0')));
}

TEST(Whirlpool, z656)
{
    ASSERT_EQ("b61902fc169b85a006698fbf8798ba6af1bf8ddbdb4300a70e5de4bfa35fb083e9a19387731c288215ce2bd7e4bd6548957c4e09a14afed1d5f91020fead2401",
            whirlpool(std::string(82, '\0')));
}

TEST(Whirlpool, z664)
{
    ASSERT_EQ("6bc92cd769704e459dcee1b0a37a04ca79babfb5a6aa75ac16eb63ddb1f6207886942715c46daf871a6cb64357f19c7726ef7ed1fc36b15a95c05e4c4f221d43",
            whirlpool(std::string(83, '\0')));
}

TEST(Whirlpool, z672)
{
    ASSERT_EQ("f61a9fc5ee251e87ea50a0031f132a2a8ec40755e0cb5b440e92c71869556b62fe91d6867dbc170a7a89b28c316c6930d7351f0269647205746a8ab0a65c79b8",
            whirlpool(std::string(84, '\0')));
}

TEST(Whirlpool, z680)
{
    ASSERT_EQ("8fe667be1ae384ef73f1c12bb95b1e4378e54a70f8d0d329633d59ce6c24fe1c46807d94f44b3348d362f1b64ad811914fec30dbac19aeaa34468cc7e2888517",
            whirlpool(std::string(85, '\0')));
}

TEST(Whirlpool, z688)
{
    ASSERT_EQ("95948460b4f1b29b463aa07b7be6e43af59e41dfd95b1bd93696c5094420715fcab72288a4d573d76349f0a97e67058828906db4ecf61d8210f5ff4a0a1df8f1",
            whirlpool(std::string(86, '\0')));
}

TEST(Whirlpool, z696)
{
    ASSERT_EQ("a16153c7051b55e7926e75cbff374bec74f2e92d745b485809aa3d11b40cb367d1da142918bced6e015ca04293417116b688fda90b08a5c3170f9dd1fa9ac161",
            whirlpool(std::string(87, '\0')));
}

TEST(Whirlpool, z704)
{
    ASSERT_EQ("c9b020a35ef82512f8adf2f3d839646c43e47cc385beefa1c553f4d153f1c9f98ff6658c19ff0ef30d02b255f0134701e1b8a9d5012047b1597182f2bcedd860",
            whirlpool(std::string(88, '\0')));
}

TEST(Whirlpool, z712)
{
    ASSERT_EQ("a570fa22948572ea4e18a1c86550a5dab7c12259b65ee9fe5d32de799d7249e356fad35bd694435aef45a2483fe26ff15b83d5468ef34bcb932a26a3d801b658",
            whirlpool(std::string(89, '\0')));
}

TEST(Whirlpool, z720)
{
    ASSERT_EQ("83c9bef94a2041a920df5b4665004202e4b3cc9373c881ee61ca402e765ebca6d9c6c4d9a521115ef427430a56ec4c1d007e56f170891ace1a672ed369a86a4f",
            whirlpool(std::string(90, '\0')));
}

TEST(Whirlpool, z728)
{
    ASSERT_EQ("3cdb51acb9f8e1578de697cc0930450f31c20f92e719b6ec4a6ba9bc4b6ac8bb9d848f35d00a69090b3a7261a5314157596c737a88509a29e0881407ce5e3bcf",
            whirlpool(std::string(91, '\0')));
}

TEST(Whirlpool, z736)
{
    ASSERT_EQ("dcc2cad3a902af3b3e6d5cf383b0181d9132f39a754961f3b3f670f4b196728de2fb4b595d18f74337911633ac1624a3aa3a33199e95e0cf7815f9b6d656c685",
            whirlpool(std::string(92, '\0')));
}

TEST(Whirlpool, z744)
{
    ASSERT_EQ("e6ddf6e43856374276aaee041dbbe63655f52cd176363ee956c98202e0eb3fbee8f2fb9989a4a72cc494fa9f6f9759a3e4b0d33be24a9dc37e02ec1fbeebec04",
            whirlpool(std::string(93, '\0')));
}

TEST(Whirlpool, z752)
{
    ASSERT_EQ("1c17c03ac300744c417447afe4fc8db2b814836dc7450c4b621cb147b5f96e2172b133ef62db323bdd8ad9b45d25932ad5feafbdb36ebd38c8d711b706bbf283",
            whirlpool(std::string(94, '\0')));
}

TEST(Whirlpool, z760)
{
    ASSERT_EQ("3392d35211e8e70f2f10426e8b53e900a1d463d21970c0e82aff0d8e41b1abbd672d4d5650cb2c24d347f25f4a07242a2f2d5522f9215c09dfe01502f80e91e9",
            whirlpool(std::string(95, '\0')));
}

TEST(Whirlpool, z768)
{
    ASSERT_EQ("84c17bd37a65ecac894c644bb7f1141d40352569dbd03b774a119d0d27e663db243108e1834225f1b37d573ee36dabdb9b015729800a2fcd6fdfcaa0e0ef519d",
            whirlpool(std::string(96, '\0')));
}

TEST(Whirlpool, z776)
{
    ASSERT_EQ("ca3e0545fa1445d2c92f1efb0c7e3b094984b81a9f08f2c27b572531151dce8556caa8d532061794b3c58a2dde54ad43eaed1c2a04503cb985227a93f954ca57",
            whirlpool(std::string(97, '\0')));
}

TEST(Whirlpool, z784)
{
    ASSERT_EQ("a7d14d84e4fdf8514f27a16a2d91af7044e2c1782971e9f790a06058659389ca333cd3f4e6de8845d3875ed478ae33b9392999ada377483e1adfaeebd4b33a58",
            whirlpool(std::string(98, '\0')));
}

TEST(Whirlpool, z792)
{
    ASSERT_EQ("541e0b0add60b7af7fe95130f9e21a871397f13d206d76924b16e912afbd699243507e325c05431604514723bed8b9ca39491c6a9074a130f7629efc2b9056b3",
            whirlpool(std::string(99, '\0')));
}

TEST(Whirlpool, z800)
{
    ASSERT_EQ("e4e08244a13e99f363fdcc35e6a90ff7dabc13be436b7fca2409fd1a8465e0607a9b160413a46ab7d8daade1b8527c3c15e66235dd868ec927fdc9c159203959",
            whirlpool(std::string(100, '\0')));
}

TEST(Whirlpool, z808)
{
    ASSERT_EQ("3ebb7109dd79e67fef25bc0a21bb40a12eb37224f27319609c73dca5c3dad25879a97e5aabc6bdb55d9e521615de4d4a814185eb49e51133c59bb3dffc655162",
            whirlpool(std::string(101, '\0')));
}

TEST(Whirlpool, z816)
{
    ASSERT_EQ("84cbd41d122c7df738ca360362d99152bf7239f74282fb7192c7d6d7e68f2560583163f7f68ea8fe8733607d4dd67934ffc5723b6525be04496498e82ba92221",
            whirlpool(std::string(102, '\0')));
}

TEST(Whirlpool, z824)
{
    ASSERT_EQ("9a571a2982af27a6c11ca7015935bf58192e09cb15f725389709e8844083319e501bf32bff52dff8e5ec1e8c27bad05c143699eb19db1a514a5e127dc3c03379",
            whirlpool(std::string(103, '\0')));
}

TEST(Whirlpool, z832)
{
    ASSERT_EQ("7af60a5d97c40d4f83eafa97c598c57964b4a25fa43784ffd0b4a8c76e76d97549facba985d1319987f3819fa7249f399eb2047844c9a5e008abfc292aead796",
            whirlpool(std::string(104, '\0')));
}

TEST(Whirlpool, z840)
{
    ASSERT_EQ("311e6fe802eef5a6dc36a26f5cc831afa5a8874e9df044d5dd57ad721d00d8a3616a12f3767e5580a8653af82f14265d734d83e160be24b35d03d32eba14a902",
            whirlpool(std::string(105, '\0')));
}

TEST(Whirlpool, z848)
{
    ASSERT_EQ("44f9aceeb3816d0e6c4f36a27a3989aa3e937e0d75720029832683999e87d8773f0fa8c29a71b62ddb744216c8e7e43993fedc32ce2902c0ac1f53f47a7dfa61",
            whirlpool(std::string(106, '\0')));
}

TEST(Whirlpool, z856)
{
    ASSERT_EQ("a752bd35b98a170c04ae436354074b91c6f0ec0b42f20934bc3bc3c9ab117f8b1f7169b4c471a8c87a9236253645a25f2084ca002b584c7c12fb06633eb8c6db",
            whirlpool(std::string(107, '\0')));
}

TEST(Whirlpool, z864)
{
    ASSERT_EQ("9c0c2d06c86ca051af80f32ee17ec2ea0756f83ec27660a38b85e1783422e07d4d331322fdbf7671849bda1b9b63c293c52bca566abe1e1d264bb8e48f76ca22",
            whirlpool(std::string(108, '\0')));
}

TEST(Whirlpool, z872)
{
    ASSERT_EQ("7140bc6850b4ceda301e6fa9ece5a0f9e56cedc52720a3a80a6e8688abbc8fc633154672da0db25f95a252014f8a60cd1f3803a5d1d29686cf2d8d2cef096522",
            whirlpool(std::string(109, '\0')));
}

TEST(Whirlpool, z880)
{
    ASSERT_EQ("84bbeb1d94f26cbcedfbc2e88b2db243f9c9609f541d2318af4d87c9045ac76e86743e000ac2aadbebf85d48a1135652e908c2bfa16cdbd1ef4caae26afe65a2",
            whirlpool(std::string(110, '\0')));
}

TEST(Whirlpool, z888)
{
    ASSERT_EQ("58058d5811cb118fa2480b0e76c00cb217c6f7b40dc786f884194ad2ce2e4620862ebacf758afe3e304a16f6dd57503890d8c6609fb5b26df2f19a461ef2e4b1",
            whirlpool(std::string(111, '\0')));
}

TEST(Whirlpool, z896)
{
    ASSERT_EQ("e7a337a2189f07031e4b68e6114df7e1f0979970e90a999390b6a374bbdeb37d5842c6c2a691d6d9f600626c57c73f4b70082ca6770daec1ede1dd28c984964e",
            whirlpool(std::string(112, '\0')));
}

TEST(Whirlpool, z904)
{
    ASSERT_EQ("fab9052917bf1feaa1989d0bd79a6d4bbeae2bd4ecfdcd61607815d6c6857aaa38be4b27fc9ea6520cc0f0935ebb61012d1d580c5b04f6dca1ec973ebad2254e",
            whirlpool(std::string(113, '\0')));
}

TEST(Whirlpool, z912)
{
    ASSERT_EQ("9855fe16ff0349c62e7d582ac20045f338d37fd70f62c860b83fcccea80b48c614e1ee443466f4e5ceffb7f27f40b749db5d6391d84981ed85c82c9c45b4cdf1",
            whirlpool(std::string(114, '\0')));
}

TEST(Whirlpool, z920)
{
    ASSERT_EQ("c8079a4221f11ead657c380953ef46c87336604630266add73afb30f3010a1d7f3250fcb7e83f467bdbb1ddc11ea27f61364af0b7d334fc48525740cae208de7",
            whirlpool(std::string(115, '\0')));
}

TEST(Whirlpool, z928)
{
    ASSERT_EQ("9e7d24de7b1f616085056466268d30cc306c618c775d4550570a0c51d6840627fd784c79141a9a4f77d9f938bfdcd8c69db2ce4f8733924e646113344407e1cf",
            whirlpool(std::string(116, '\0')));
}

TEST(Whirlpool, z936)
{
    ASSERT_EQ("5e72bbed520a7b8ed4168157cd16d673742f2aef38be106a8a461b878af563d2f78fa6ca55ccbef568ff3cc735ab4d1c48d8169783910bf52488cd9ffd9f8cd4",
            whirlpool(std::string(117, '\0')));
}

TEST(Whirlpool, z944)
{
    ASSERT_EQ("7aa748a47b62072cf213afce5f8212c7b2c2c7d90ac9a4ff3e964350a76eabe400379036c33474e1a2fa0d128f3cfb59249cf01fda102b5db8519f0b9a2b4f52",
            whirlpool(std::string(118, '\0')));
}

TEST(Whirlpool, z952)
{
    ASSERT_EQ("4003434d77081d59ef43f826de060d4776d94ce8c9b7e0d11d3c7a159addcaaed691843793ff0a6482c1f9eb5740765bd47720cbd7660ea0e310a76e09bc38cd",
            whirlpool(std::string(119, '\0')));
}

TEST(Whirlpool, z960)
{
    ASSERT_EQ("2adee7400ca8b2d5ee3cc614b7de1a94d70246ca6b29b7ecd7b01a59cea0592730aad9fee593c9d3d809d47d74b901f897e8c7901ddfc68ca5a1971c41853415",
            whirlpool(std::string(120, '\0')));
}

TEST(Whirlpool, z968)
{
    ASSERT_EQ("b2bd8fb6dac755aee5cb7e834a697ef606b45652537961e587a11529995d588f11c28f7728999e19edcb07cedb18b98f843d240e537ed64b60c3f5497fe4d661",
            whirlpool(std::string(121, '\0')));
}

TEST(Whirlpool, z976)
{
    ASSERT_EQ("d2cfec1e66e0ae55e01e17bdbc35e00c4f18588385433da3c9e823ea17fef5d17970dcbdb6108cc0dfea4dd39a30f23646473426d03566c622eb1da0b8b3a4dc",
            whirlpool(std::string(122, '\0')));
}

TEST(Whirlpool, z984)
{
    ASSERT_EQ("065f8a62b8ec8d3f3172015e32a5714c30627b53533daae508ec85fb90048de708a2199d9ea4efa6949139f7526cbd8bab6402cbda92de82fad802409ca2e772",
            whirlpool(std::string(123, '\0')));
}

TEST(Whirlpool, z992)
{
    ASSERT_EQ("a11ea682ea8b07efe7c1290988a8deffe453aaffae4f3dd6619d96e496f28f5fe3556c3999fe9fcdb5a19f0631580e9b1bcb74f5003f512b92bc068508cb279c",
            whirlpool(std::string(124, '\0')));
}

TEST(Whirlpool, z1000)
{
    ASSERT_EQ("4648c011888388ec85f19bd3a5247dc69ef1883d4472d6f7bfd5fdcaef3e35770c65ec0c32cadebb91471301225c21d4d64a520071b5ecc3bee8e0d21d6f6d3b",
            whirlpool(std::string(125, '\0')));
}

TEST(Whirlpool, z1008)
{
    ASSERT_EQ("f62d5b8a06576ede68c5d8454d448202ce0b1c85ce20a139739ce78e3cbb4810679ba2b4e211a2b0c03d064e3164b96ffb94f04bfd4d504657a9866b0eed1d5b",
            whirlpool(std::string(126, '\0')));
}

TEST(Whirlpool, z1016)
{
    ASSERT_EQ("cf73ab693e86e45fc33f9dc174443e7ea4e8acb131257f5ceac4503d9c7a1138342e2b80e6c4fddb3b47b00c990283903039cb5622ac905b3b9c1ed7c9982194",
            whirlpool(std::string(127, '\0')));
}

