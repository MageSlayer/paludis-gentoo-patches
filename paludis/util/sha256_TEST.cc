/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2011 Ciaran McCreesh
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

#include <paludis/util/sha256.hh>

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

    std::string sha256(const std::string & data)
    {
        std::stringstream ss(data);
        SHA256 s(ss);
        return s.hexsum();
    }

    std::string sha256_dehex(const std::string & data)
    {
        std::stringstream ss(dehex(data));
        SHA256 s(ss);
        return s.hexsum();
    }
}

TEST(SHA256, Works)
{
    EXPECT_EQ("e3b0c442""98fc1c14""9afbf4c8""996fb924""27ae41e4""649b934c""a495991b""7852b855",
            sha256(""));

    EXPECT_EQ("ba7816bf""8f01cfea""414140de""5dae2223""b00361a3""96177a9c""b410ff61""f20015ad",
            sha256("abc"));

    EXPECT_EQ("248d6a61""d20638b8""e5c02693""0c3e6039""a33ce459""64ff2167""f6ecedd4""19db06c1",
            sha256("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"));
}

/* the following tests are from: http://csrc.ncsl.nist.gov/cryptval/ */

TEST(SHA256, t11SHA256ShortMsg)
{
    EXPECT_EQ("68325720aabd7c82f30f554b313d0570c95accbb7dc4b5aae11204c08ffe732b",
            sha256_dehex("bd"));
}

TEST(SHA256, t14SHA256ShortMsg)
{
    EXPECT_EQ("7c4fbf484498d21b487b9d61de8914b2eadaf2698712936d47c3ada2558f6788",
            sha256_dehex("5fd4"));
}

TEST(SHA256, t17SHA256ShortMsg)
{
    EXPECT_EQ("4096804221093ddccfbf46831490ea63e9e99414858f8d75ff7f642c7ca61803",
            sha256_dehex("b0bd69"));
}

TEST(SHA256, t20SHA256ShortMsg)
{
    EXPECT_EQ("7abc22c0ae5af26ce93dbb94433a0e0b2e119d014f8e7f65bd56c61ccccd9504",
            sha256_dehex("c98c8e55"));
}

TEST(SHA256, t23SHA256ShortMsg)
{
    EXPECT_EQ("7516fb8bb11350df2bf386bc3c33bd0f52cb4c67c6e4745e0488e62c2aea2605",
            sha256_dehex("81a723d966"));
}

TEST(SHA256, t26SHA256ShortMsg)
{
    EXPECT_EQ("0eb0281b27a4604709b0513b43ad29fdcff9a7a958554abc689d7fe35af703e4",
            sha256_dehex("c97a2db566e5"));
}

TEST(SHA256, t29SHA256ShortMsg)
{
    EXPECT_EQ("dee684641421d1ba5a65c71f986a117cbb3d619a052a0b3409306c629575c00f",
            sha256_dehex("f53210aa6ed72e"));
}

TEST(SHA256, t32SHA256ShortMsg)
{
    EXPECT_EQ("47f527210d6e8f940b5082fec01b7305908fa2b49ea3ae597c19a3986097153c",
            sha256_dehex("0df1cd526b5a4edd"));
}

TEST(SHA256, t35SHA256ShortMsg)
{
    EXPECT_EQ("c60d239cc6da3ad31f4de0c2d58a73ccf3f9279e504fa60ad55a31dcf686f3ca",
            sha256_dehex("b80233e2c53ab32cc3"));
}

TEST(SHA256, t38SHA256ShortMsg)
{
    EXPECT_EQ("e0164d90dbfcf173bb88044fac596ccd03b8d247c79907aaa5701767fad7b576",
            sha256_dehex("5d54ed5b52d879aeb5dd"));
}

TEST(SHA256, t41SHA256ShortMsg)
{
    EXPECT_EQ("dc990ef3109a7bcf626199db9ab7801213ceb0ad2ee398963b5061e39c05c7b5",
            sha256_dehex("df866ecb67ab00515f6247"));
}

TEST(SHA256, t44SHA256ShortMsg)
{
    EXPECT_EQ("c1c9a4daadcc8678835872c7f1f8824376ac7b412e1fc2285069b41afd51397e",
            sha256_dehex("0757de9485a2eaea51126077"));
}

TEST(SHA256, t47SHA256ShortMsg)
{
    EXPECT_EQ("6840619417b4d8ecaa7902f8eaf2e82be2638dec97cb7e8fcc377007cc176718",
            sha256_dehex("7c66f5d443c11cfb39dd0aa715"));
}

TEST(SHA256, t50SHA256ShortMsg)
{
    EXPECT_EQ("0f5308ff22b828e18bd65afbc427e3c1a678962832519df5f2f803f68f55e10b",
            sha256_dehex("329624fed35639fe54957b7d47a9"));
}

TEST(SHA256, t53SHA256ShortMsg)
{
    EXPECT_EQ("0fdf1604ac0d717ec9587b4de5444aaade807589d90eb326eaf6acb58a051e79",
            sha256_dehex("c34e59652acc043873ecf6a4ab1060"));
}

TEST(SHA256, t56SHA256ShortMsg)
{
    EXPECT_EQ("b01ae16eed3b4a770f127b98469ba26fe3d8e9f59d8a2983214afe6cff0e6b6c",
            sha256_dehex("fdf4700984ee11b70af1880d0e0fefd4"));
}

TEST(SHA256, t59SHA256ShortMsg)
{
    EXPECT_EQ("36157bbe61931d58a3a644953eaf131bbc2591c673a1f20353f51ca5054fc1c2",
            sha256_dehex("ea40aadbefedb0e0d78d067c6cd65c2c87"));
}

TEST(SHA256, t62SHA256ShortMsg)
{
    EXPECT_EQ("67fbf35d360d72b101410794ccf197106c0e784afa9c80206a550b600dbf1f16",
            sha256_dehex("6d1092004670efab3af483d265d8e7b3da73"));
}

TEST(SHA256, t65SHA256ShortMsg)
{
    EXPECT_EQ("cbe7965513af46dfd596dc5839cb82a5c6c7328034b1dd0042a9f4b71fb14430",
            sha256_dehex("55a10148ae7b09ac4e71df438135bc70e873eb"));
}

TEST(SHA256, t68SHA256ShortMsg)
{
    EXPECT_EQ("ddfce4e8c7b38845e2a81b7fc27a06366467a9e111316014013f9701e2413ce0",
            sha256_dehex("a03f8fcd777bd933b4b0af8c5ce3d61308565649"));
}

TEST(SHA256, t71SHA256ShortMsg)
{
    EXPECT_EQ("92f678a3e59d0dd3610eec3222b8c6ebd28eead530723fbd226747534da22b6c",
            sha256_dehex("8e5d6cba8d4b206381e33ca7339bec504f3d6119"
                "ba"));
}

TEST(SHA256, t74SHA256ShortMsg)
{
    EXPECT_EQ("725bab4457c789d6a4cc4736b9c2c662cda18407150844d74d6aa4efd72dbb05",
            sha256_dehex("96db1b62eed85f2628d0c25da534401fe80d13d0"
                "9beb"));
}

TEST(SHA256, t77SHA256ShortMsg)
{
    EXPECT_EQ("6523f24f225b996aad1a8b317e6e0f8e97673dcff3fd62a27ff9f3888ea1302d",
            sha256_dehex("1c482a45dfbcda549729126b533477edfaf7476f"
                "de498f"));
}

TEST(SHA256, t80SHA256ShortMsg)
{
    EXPECT_EQ("44acbbc6b48bf37ee088b9c8546fc46e5a5f0d637b5e444f628de186144087fd",
            sha256_dehex("0f677d8e4c6d6a057492670d99adb870adf68a36"
                "ead37919"));
}

TEST(SHA256, t83SHA256ShortMsg)
{
    EXPECT_EQ("f4baeaef70588a0820d63c2401dd84f98adf7366782d196f8698d7dfd3db1c29",
            sha256_dehex("c09056d597816542bffe4bb33e475dfb2d629301"
                "6906ddc18c"));
}

TEST(SHA256, t86SHA256ShortMsg)
{
    EXPECT_EQ("cfa67aa52fd675fca985f69f9ca58af62baead8c39723bb6bfbae8a5d4bb9beb",
            sha256_dehex("72f313fdcf52d0749c9937cc2e53f50b44d65a54"
                "4876bab7d2f8"));
}

TEST(SHA256, t89SHA256ShortMsg)
{
    EXPECT_EQ("657633891dc6274d6aeda78e7313dfb960eac9a24d29293a057b9746a18de4ec",
            sha256_dehex("09f6fe6cbe6744149f792a4a827e4e8909627abf"
                "75301bf7bbd7f5"));
}

TEST(SHA256, t92SHA256ShortMsg)
{
    EXPECT_EQ("930058dd21cb48b2cf90eaca55322ddf48582687838a584928440504a2fde578",
            sha256_dehex("9e1cfeb335bc331744247df4bbd56876a7f69298"
                "aaf6b9e7a8731889"));
}

TEST(SHA256, t95SHA256ShortMsg)
{
    EXPECT_EQ("a0eb0b7fad1d1b6de4f9096724a621720538a9c3f2f6d11134d68cb9ee52fc88",
            sha256_dehex("b8913001efb1b7f4bd975e349c5b2cbe66045bf0"
                "d2fb019b3bc0f059a4"));
}

TEST(SHA256, t98SHA256ShortMsg)
{
    EXPECT_EQ("10aad5cd4484387373577a881974f1a550782108bc88b4e2e8085e9c3e938bbb",
            sha256_dehex("8f08537d50928c911a68b071d65b9e8f038264d3"
                "b62c5f33de18a484cde9"));
}

TEST(SHA256, t101SHA256ShortMsg)
{
    EXPECT_EQ("c13ba769aea0e478816f2f608b5cec3fe14672ea033088a8641cfe69b4ff57cb",
            sha256_dehex("fd846162c4da936d004ffe0cbe844d940f1c2953"
                "157cf4765dceba2a6f4c64"));
}

TEST(SHA256, t104SHA256ShortMsg)
{
    EXPECT_EQ("56059e8cb3c2978b198208bf5ca1e1ea5659b737a506324b7cec75b5ebaf057d",
            sha256_dehex("8cf53d90077df9a043bf8d10b470b144784411c9"
                "3a4d504556834dae3ea4a5bb"));
}

TEST(SHA256, t107SHA256ShortMsg)
{
    EXPECT_EQ("d973b5dcdae4cf2599f4db4068e4aa354f22d8901adc463ca3938c465578147b",
            sha256_dehex("1bbc2b15253c126e301f9f64b97be4ce13e96337"
                "687e2e78fbfd4c8daf4a5fa1cd"));
}

TEST(SHA256, t110SHA256ShortMsg)
{
    EXPECT_EQ("57844e1d762e6b7bb86dbfcc5c5a59578d39cc665d1ddbe4de03a61778061af1",
            sha256_dehex("c1bdb3bfc65dfe9a393331266c58d05fb9c8b747"
                "6bb717dadc29bc43dabd91504fc9"));
}

TEST(SHA256, t113SHA256ShortMsg)
{
    EXPECT_EQ("73dc27bd45daccd0f811381230cf7f2a1d3ed1202e9a770af733146b1e166315",
            sha256_dehex("26eb621a45bd9c9c764ccbb672b99f2a8379c7bb"
                "f4fb07eec58a8b0ea4747b72196ccf"));
}

TEST(SHA256, t116SHA256ShortMsg)
{
    EXPECT_EQ("682c474799f5103252c3e2efef7f747783e514b54e93b8303b0e07ee4218f78e",
            sha256_dehex("7e3e3986109162e0c56357048bbd86ff49b93644"
                "b7fb064e7280968650978466f02c9adf"));
}

TEST(SHA256, t119SHA256ShortMsg)
{
    EXPECT_EQ("54d6cb2b09825eab064c8952113b9897a3344737cd186a8e6be0a0b258da3e57",
            sha256_dehex("763c1a9ea50bd72bfc516989ddf3eff2f208f64f"
                "ccea3cf0ca8dba7f3d10e237c99226510f"));
}

TEST(SHA256, t122SHA256ShortMsg)
{
    EXPECT_EQ("83baa80caade404c446833ecef2e595bba6dce2cb7f7422fad2972a9fe327aca",
            sha256_dehex("e1a7ffea8417e7cd49b96e355fd44f3f7a150fab"
                "6dd8343dfba3b262eaf3a6175a3c4607552b"));
}

TEST(SHA256, t125SHA256ShortMsg)
{
    EXPECT_EQ("0c0c6a6b27a6d7a7a5130d70db3b8bc1bd8001d103efe72f45b082cadbd03742",
            sha256_dehex("692a18effad8317a11a5cddb917f7389e1be6dba"
                "34572a300e52e056047e758bc363a0be53784c"));
}

TEST(SHA256, t128SHA256ShortMsg)
{
    EXPECT_EQ("9878f8804e00828b39261843f2b3eda19a7e9b9ff4cc2e23f7ea1f62f4491ff2",
            sha256_dehex("73fda1e1cb7dc9a9ece858d040d7105cc126eab1"
                "53fb0bb55703f4317dfff97bd980f4523aee3a09"));
}

TEST(SHA256, t131SHA256ShortMsg)
{
    EXPECT_EQ("f1bd3a8a74c8f0093038499ef63794d86fc6d82602a802a435718e61e7b396cc",
            sha256_dehex("2321d88c19e3e6a8309a09a5428c01991e164468"
                "23f13b2f0db4ade30e9a7c3521868fb99b440f48"
                "02"));
}

TEST(SHA256, t134SHA256ShortMsg)
{
    EXPECT_EQ("ea43ec91285145d8f29915b227a0e35c89f90d968f9a14332dad275cfd52d619",
            sha256_dehex("b9eaebda29172b052bcc1e3a9c7f2eced43c084a"
                "86f89f61e7237425137c167aac29e4cac4071afa"
                "fd3f"));
}

TEST(SHA256, t137SHA256ShortMsg)
{
    EXPECT_EQ("a573959ba6b1c3bebfd6288c806b72a65650d23bd46d123816a2a6a0e47d1e66",
            sha256_dehex("332daf07d3a6775b18572549a6e12b8a27d81b7c"
                "4abcc5bd0b2b9ff936546b0026af131cd3ecd8a1"
                "0c29ab"));
}

TEST(SHA256, t140SHA256ShortMsg)
{
    EXPECT_EQ("c0c3f40d34e711bfadf517b3a78140e379fba5f7edf2c1bc3ce82469dae4d2d5",
            sha256_dehex("30ac7eace1f2e41034c25a3d3e2db979c23dfaa7"
                "a4914b0da147625b3e1f12e9fedc1c41d8ee47dd"
                "e84fb332"));
}

TEST(SHA256, t143SHA256ShortMsg)
{
    EXPECT_EQ("c13c622bf08a3d3cf1fd6fa5e26e505e551b1643bc5a0f59ed29541235218f77",
            sha256_dehex("02c3964c4ad9c4af97d373099302c2cd770ad06c"
                "7d8bd11c970161d861e917a854265e223da28031"
                "ee38041534"));
}

TEST(SHA256, t146SHA256ShortMsg)
{
    EXPECT_EQ("6ac64caaeda4763d28a44b363823a6b819285410fb4162af6ca657396f6028d0",
            sha256_dehex("b9eed82edcf0c7ba69f6f6ac5722cb61daecaf30"
                "437511582117ad36ad410ebc6582511ef6e32dce"
                "5f7a30ab543c"));
}

TEST(SHA256, t149SHA256ShortMsg)
{
    EXPECT_EQ("4c839e8f8f373c25a9a3351257c6152258ff8e6a88dad42f30f2bbecab56c20b",
            sha256_dehex("b574865024828bf651df070ac0cec1849aa64709"
                "01d2e30fa01dcb43862d9827344cf900f46fa9ef"
                "6d709e5e759f84"));
}

TEST(SHA256, t152SHA256ShortMsg)
{
    EXPECT_EQ("c117b9dce689c399ec99008788cd5d24d8396fab7d96315c4f3fe6d56da63bb3",
            sha256_dehex("eebcf5cd6b12c90db64ff71a0e08ccd956e170a5"
                "0dad769480d6b1fb3eff4934cde90f9e9b930ee6"
                "37a66285c10f4e8a"));
}

TEST(SHA256, t155SHA256ShortMsg)
{
    EXPECT_EQ("0b42cfc3dd3d3198f06c30e087837ec6a6dd35d08e54e886c682709f8f42457a",
            sha256_dehex("1b7a73770d168da45bf2e512eee45153e02f4dfe"
                "3b42e50304a3d63d7826f0469562be8fdc6569b0"
                "56a7dafcd53d1f597c"));
}

TEST(SHA256, t158SHA256ShortMsg)
{
    EXPECT_EQ("217cf25b8b343c28336b1c1e9bed29e0c96045bc93daf426e490b608b0905c90",
            sha256_dehex("0072ae2f3bda67736b9c66e2130260b3a4847bc3"
                "968e037cb6835efcc2014273336725cd5a94f592"
                "aef20a0a65b459a4415b"));
}

TEST(SHA256, t161SHA256ShortMsg)
{
    EXPECT_EQ("3ea59e2e79513679a22e962f22408306f7e8f6e562c2f1f210e279fad8eaacc6",
            sha256_dehex("2ac748680f3bc1bf098c4be38c7194643b0d009e"
                "51c43630404cdfaf9807aa9b299094916c9466c3"
                "1fe37fa630c6d3eadc9434"));
}

TEST(SHA256, t164SHA256ShortMsg)
{
    EXPECT_EQ("f7808e03e5d5af43c2bffb66e35d1ecbd79f4d8fec44f821f73a235d17c70a89",
            sha256_dehex("893d1a8863d234ee50e5a8c7650a4de047230ad0"
                "3d268dde8921401ff97b79dfb97cf2426b0f782b"
                "79c7e75daa2155e1f4098ea7"));
}

TEST(SHA256, t167SHA256ShortMsg)
{
    EXPECT_EQ("9bdb7cf0492ace4620a47660acd127f951767b0738b5504451d6ed56e4fa3cbd",
            sha256_dehex("cf673b96eaf241cfa3e262dc6fe65f08bcc2be56"
                "d8a2c9710eaddae212ded6859f0ff83e5e57d0e8"
                "0a968b8ed24e74defeb5bbdad6"));
}

TEST(SHA256, t170SHA256ShortMsg)
{
    EXPECT_EQ("ad53e0db7e63211c8b00947908ce29660c4376e244e19cd30a659af65dc6f1fe",
            sha256_dehex("0d545be1f47b966214691c21278704e89a17d52d"
                "d96aeeeacc5325a9a1ddafdecd39407a4dfa72bd"
                "32856b4c5cc2ba838618830c8399"));
}

TEST(SHA256, t173SHA256ShortMsg)
{
    EXPECT_EQ("83eeed2dfeb8d2604ab5ec1ac9b5dcab8cc2222518468bc5c24c16ce72e70687",
            sha256_dehex("9eabfcd3603337df3dcd119d6287a9bc8bb94d65"
                "0ef29bcf1b32e60d425adc2a35e06577d0c7ce24"
                "56cf260efee9e8d8aeeddb3d068f37"));
}

TEST(SHA256, t176SHA256ShortMsg)
{
    EXPECT_EQ("6ef7e9f12267ebc4901267da147effdcdebcd6ec5393c7f62ec4c4f06ca72649",
            sha256_dehex("2fc7b9e8b8dcaac64ecef4c5f91877543ac36ae4"
                "94d9faf84b1d347b6cf925570db84043d6f500dc"
                "c153cef81d6f2437d913f3dbffad42d9"));
}

TEST(SHA256, t179SHA256ShortMsg)
{
    EXPECT_EQ("3e5854169da065407fa465a4694f3fcb1d141480a8f84c970a0f63364ec8f590",
            sha256_dehex("cf95929ab732f9ef5e8c3e6b4ed753852ee74e4f"
                "ddf31b56c29a6ec95d23fcde2209eb7288b787f0"
                "5d9036735c32ae2f01fc650d9cce4995a5"));
}

TEST(SHA256, t182SHA256ShortMsg)
{
    EXPECT_EQ("5b506b823ef6658939aca22f52bbe5a4b849c31b8fa1d09139352e501137bc04",
            sha256_dehex("826378013988684c40f4d917c7ed8b72aba66fd6"
                "8f085d0b2eb20948ef3f349dbbc71f8e0ba84501"
                "4586495a48902ee44505c673d2f76d473950"));
}

TEST(SHA256, t185SHA256ShortMsg)
{
    EXPECT_EQ("92943076cda4c46718e55df64d7580e12b8fb2c2911e87851246ccf6791fa3e6",
            sha256_dehex("0cab6d38ce9849fcbd589f7235a6d2c2cb933e26"
                "e1ca6f4e78189104452c280c069b024e16276937"
                "3f409d5cd0cb8160f0239418325d23ee6ad1bd"));
}

TEST(SHA256, t188SHA256ShortMsg)
{
    EXPECT_EQ("8e90da3eb146935264576f874fcc5a64b7a90ab6c8a36c15d855b0179f52f899",
            sha256_dehex("3fb4a8c5b57c14731179256608614c95c9725dda"
                "d5fbfa99111d4fa319d3015ad830601556e8e4c6"
                "d012d7da0e2c4f60f1605f6e4c058ec0f46988a3"));
}

TEST(SHA256, t191SHA256ShortMsg)
{
    EXPECT_EQ("03c516677735ae83dbe5a7e4c22c1ac1bfedcd46e7dd785f8bfe38e148eda632",
            sha256_dehex("9050a6d002c90f6036c592b0f6b866713e7894d2"
                "9645f4a19e0858b3ebd8078711c26d2601ca104d"
                "962dc6ce6ae92634ee7f3ca6baf8810e2126097a"
                "09"));
}

TEST(SHA256, t194SHA256ShortMsg)
{
    EXPECT_EQ("fff2852957a0eeb577e73fd7d827f650261dfb9a8a65f52df4bbbc9b2d0ae50e",
            sha256_dehex("d659ec136bacfa0b5c906aabedc93c01c5f1efa3"
                "f370a1432ea8778461703f0c67c454da12bac2da"
                "73b8abb755e5eaf10bddf52f6ca908d61bee80da"
                "0c64"));
}

TEST(SHA256, t197SHA256ShortMsg)
{
    EXPECT_EQ("bfbbf242f79bff4ae0aafb4ccf69b24fdca4342d83db1dfd1822c74a9e218e8d",
            sha256_dehex("b498555658332b197bc5cb7adc5c1997aabbdcf1"
                "f7ffcc2b6b82eb0f350019d247f8e399c3559d3b"
                "b04eb049f28b344c7989c24db83f839b59028dc8"
                "2fa670"));
}

TEST(SHA256, t200SHA256ShortMsg)
{
    EXPECT_EQ("105a60865830ac3a371d3843324d4bb5fa8ec0e02ddaa389ad8da4f10215c454",
            sha256_dehex("3592ecfd1eac618fd390e7a9c24b656532509367"
                "c21a0eac1212ac83c0b20cd896eb72b801c4d212"
                "c5452bbbf09317b50c5c9fb1997553d2bbc29bb4"
                "2f5748ad"));
}

