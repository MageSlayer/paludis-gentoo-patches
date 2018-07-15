#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir e_repository_TEST_dir || exit 1
cd e_repository_TEST_dir || exit 1

mkdir -p root/etc

mkdir -p vdb
touch vdb/THISISTHEVDB

mkdir -p build
ln -s build symlinked_build

mkdir -p distdir
echo "already fetched" > distdir/already-fetched.txt || exit 1
cat <<END > distdir/expatch-success-1.patch || exit 1
--- a/bar
+++ b/bar
@@ -1 +1,3 @@
 foo
+bar
+baz
END

mkdir -p fetchable
echo "one" > fetchable/fetchable-1.txt || exit 1
echo "two" > fetchable/fetchable-2.txt || exit 1

mkdir -p repo1/{eclass,distfiles,profiles/profile} || exit 1
cd repo1 || exit 1
echo "test-repo-1" > profiles/repo_name || exit 1
cat <<END > profiles/categories || exit 1
cat-one
cat-two
cat-three
END
cat <<END > profiles/profile/make.defaults
ARCH=test
END
cd ..

mkdir -p repo2/{eclass,distfiles,profiles/profile} || exit 1
cd repo2 || exit 1
cat <<END > profiles/categories || exit 1
END
cat <<END > profiles/profile/make.defaults
ARCH=test
END
cd ..


mkdir -p repo3/{eclass,distfiles,profiles/profile} || exit 1
cd repo3 || exit 1
echo "# test-repo-3" > profiles/repo_name || exit 1
cat <<END > profiles/categories || exit 1
END
cat <<END > profiles/profile/make.defaults
ARCH=test
END
cd ..


mkdir -p repo4/{eclass,distfiles,profiles/profile} || exit 1
mkdir -p repo4/{cat-one/{pkg-one,pkg-both},cat-two/{pkg-two,pkg-both}} || exit 1
cd repo4 || exit 1
echo "test-repo-4" > profiles/repo_name || exit 1
cat <<END > profiles/categories || exit 1
cat-one
cat-two
END
cat <<END > profiles/profile/make.defaults
ARCH=test
END
cat <<END > cat-one/pkg-one/pkg-one-1.ebuild || exit 1
END
cat <<END > cat-one/pkg-one/pkg-one-1.1-r1.ebuild || exit 1
END
cat <<END > cat-one/pkg-both/pkg-both-3.45.ebuild || exit 1
END
cat <<END > cat-two/pkg-two/pkg-two-2.ebuild || exit 1
END
cat <<END > cat-two/pkg-both/pkg-both-1.23.ebuild || exit 1
END
cd ..


mkdir -p repo5/{eclass,distfiles,profiles/profile} || exit 1
mkdir -p repo5/cat-one/{pkg-one,pkg-1,pkg.one} || exit 1
cd repo5 || exit 1
echo "test-repo-5" > profiles/repo_name || exit 1
cat <<END > profiles/categories || exit 1
cat-one
END
cat <<END > profiles/profile/make.defaults
ARCH=test
END
cat <<END > cat-one/pkg-one/pkg-one-1.ebuild || exit 1
END
cat <<END > cat-one/pkg-1/pkg-1-1.ebuild || exit 1
END
cat <<END > cat-one/pkg.one/pkg.one-1.ebuild || exit 1
END
cd ..


mkdir -p repo7/{eclass,distfiles,profiles/profile} || exit 1
mkdir -p repo7/cat-one/pkg-{one,two} || exit 1
mkdir -p repo7/metadata/cache/cat-{one,two}
cd repo7 || exit 1
echo "test-repo-7" > profiles/repo_name || exit 1
cat <<END > profiles/categories || exit 1
cat-one
END
cat <<END > profiles/profile/make.defaults
ARCH=test
END
cat <<END > cat-one/pkg-one/pkg-one-1.ebuild || exit 1
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"
DEPEND="foo/bar"
END
cat <<"END" > cat-one/pkg-one/pkg-one-2.ebuild || exit 1
inherit mine
DESCRIPTION="dquote \" squote ' backslash \\ dollar \$"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"
DEPEND="foo/bar"
END
cat <<"END" > cat-one/pkg-one/pkg-one-3.ebuild || exit 1
EAPI="exheres-0"
SUMMARY="This is the short description"
DESCRIPTION="This is the long description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS=""
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"
BUGS_TO="fred@example.com joe@example.com"
UPSTREAM_CHANGELOG="http://example.com/foo"
UPSTREAM_RELEASE_NOTES="http://example.com/bar"
UPSTREAM_DOCUMENTATION="http://example.com/baz"
REMOTE_IDS="freshmeat:fnord"
END
cat <<END > eclass/mine.eclass
DEPEND="bar/baz"
END
cat <<END > cat-one/pkg-two/pkg-two-1.ebuild || exit 1
i am a fish
END
cd ..

mkdir -p repo8/{eclass,distfiles,profiles/profile} || exit 1
mkdir -p repo8/{cat-one/{pkg-one,pkg-both},cat-two/{pkg-two,pkg-both}} || exit 1
cd repo8 || exit 1
echo "test-repo-8" > profiles/repo_name || exit 1
cat <<END > profiles/categories || exit 1
cat-one
cat-two
END
cat <<END > profiles/profile/make.defaults
ARCH=test
END
cat <<END > cat-one/pkg-one/pkg-one-1.ebuild || exit 1
END
cat <<END > cat-one/pkg-one/pkg-one-1-monkey.ebuild || exit 1
END
cat <<END > cat-one/pkg-one/pkg-one-1.1-r1.ebuild || exit 1
END
cat <<END > cat-one/pkg-both/pkg-both-3.45.ebuild || exit 1
END
cat <<END > cat-one/pkg-both/pkg-both-3.45_r1.ebuild || exit 1
END
cat <<END > cat-two/pkg-two/pkg-two-2.ebuild || exit 1
END
cat <<END > cat-two/pkg-both/pkg-both-1.23.ebuild || exit 1
END
cat <<END > cat-two/pkg-both/pkg-both-.ebuild || exit 1
END
cd ..


mkdir -p repo9/{eclass,distfiles,profiles/{profile,child},cat-one/pkg-one,cat-two/pkg-two} || exit 1
cd repo9 || exit 1
echo "test-repo-9" > profiles/repo_name || exit 1
cat <<END >profiles/categories || exit 1
cat-one
cat-two
END
cat <<END > profiles/arch.list || exit 1
test
test2
END
cat <<END >profiles/profile/make.defaults || exit 1
ARCH=test
USE="flag1 flag2 flag3 -flag4 -flag5 -enabled2 disabled2"
USE_EXPAND="NOT_IN_IUSE"
NOT_IN_IUSE="ennobled"
END
cat <<END >profiles/profile/use.mask || exit 1
flag2
enabled3
not_in_iuse_masked
END
cat <<END >profiles/profile/use.force || exit 1
flag4
disabled3
not_in_iuse_forced
END
cat <<END >profiles/profile/package.use || exit 1
cat-one/pkg-one not_in_iuse_ennobled_package
cat-two/pkg-two -not_in_iuse_disabled_package -not_in_iuse_ennobled
END
cat <<END >profiles/profile/package.use.mask || exit 1
cat-two/pkg-two flag3
>=cat-one/pkg-one-2 flag3
cat-one/pkg-one not_in_iuse_masked_package
END
cat <<END >profiles/child/package.use.force || exit 1
cat-two/pkg-two flag5
cat-two/pkg-two not_in_iuse_forced_package
END
cat <<END >profiles/child/parent || exit 1
../profile
END
cat <<END > cat-one/pkg-one/pkg-one-1.ebuild || exit 1
EAPI=1
IUSE="flag1 flag2 flag3 flag4 flag5 enabled +disabled enabled2 disabled2 enabled3 disabled3"
SLOT="0"
END
cat <<END > cat-one/pkg-one/pkg-one-2.ebuild || exit 1
EAPI=1
IUSE="-flag1 flag2 flag3 flag4 +flag5 flag6"
SLOT="0"
END
cat <<END > cat-two/pkg-two/pkg-two-1.ebuild || exit 1
EAPI=1
IUSE="flag1 flag2 flag3 flag4 flag5 +flag6"
SLOT="0"
END
cd ..

mkdir -p repo9a/{eclass,distfiles,profiles/{profile,eapi5,eapi5/child},cat/stable,cat/unstable,cat/missing} || exit 1
cd repo9a || exit 1
echo "test-repo-9a" > profiles/repo_name || exit 1
cat <<END >profiles/categories || exit 1
cat
END
cat <<END > profiles/arch.list || exit 1
test
END
cat <<END >profiles/profile/eapi || exit 1
4
END
cat <<END >profiles/profile/make.defaults || exit 1
ARCH=test
END
cat <<END >profiles/profile/use.stable.mask || exit 1
notstmask
END
cat <<END >profiles/profile/package.use.stable.mask || exit 1
=cat/stable-1 notpkgstmask
END
cat <<END >profiles/profile/use.stable.force || exit 1
notstforce
END
cat <<END >profiles/profile/package.use.stable.force || exit 1
=cat/stable-1 notpkgstforce
END
cat <<END >profiles/eapi5/eapi || exit 1
5
END
cat <<END >profiles/eapi5/parent || exit 1
../profile
END
cat <<END >profiles/eapi5/child/eapi || exit 1
5
END
cat <<END >profiles/eapi5/child/parent || exit 1
..
END
cat <<END >profiles/eapi5/use.mask || exit 1
mask-stunmask
-unmask-stmask
END
cat <<END >profiles/eapi5/use.stable.mask || exit 1
stmask
-mask-stunmask
unmask-stmask
stmask-pkgunmask
-stunmask-pkgmask
END
cat <<END >profiles/eapi5/package.use.mask || exit 1
=cat/stable-1 -stmask-pkgunmask stunmask-pkgmask
=cat/stable-1 pkgmask-pkgstunmask -pkgunmask-pkgstmask
END
cat <<END >profiles/eapi5/package.use.stable.mask || exit 1
=cat/stable-1 pkgstmask
=cat/stable-1 -pkgmask-pkgstunmask pkgunmask-pkgstmask
=cat/stable-1 pkgstmask-chunmask -pkgstunmask-chmask
~cat/stable-1 pkgstmask-chpkgstunmask -pkgstunmask-chpkgstmask
END
cat <<END >profiles/eapi5/child/use.mask || exit 1
-pkgstmask-chunmask
pkgstunmask-chmask
END
cat <<END >profiles/eapi5/child/package.use.stable.mask || exit 1
=cat/stable-1 -pkgstmask-chpkgstunmask pkgstunmask-chpkgstmask
END
cat <<END >profiles/eapi5/use.force || exit 1
force-stunforce
-unforce-stforce
END
cat <<END >profiles/eapi5/use.stable.force || exit 1
stforce
-force-stunforce
unforce-stforce
stforce-pkgunforce
-stunforce-pkgforce
END
cat <<END >profiles/eapi5/package.use.force || exit 1
=cat/stable-1 -stforce-pkgunforce stunforce-pkgforce
=cat/stable-1 pkgforce-pkgstunforce -pkgunforce-pkgstforce
END
cat <<END >profiles/eapi5/package.use.stable.force || exit 1
=cat/stable-1 pkgstforce
=cat/stable-1 -pkgforce-pkgstunforce pkgunforce-pkgstforce
=cat/stable-1 pkgstforce-chunforce -pkgstunforce-chforce
~cat/stable-1 pkgstforce-chpkgstunforce -pkgstunforce-chpkgstforce
END
cat <<END >profiles/eapi5/child/use.force || exit 1
-pkgstforce-chunforce
pkgstunforce-chforce
END
cat <<END >profiles/eapi5/child/package.use.stable.force || exit 1
=cat/stable-1 -pkgstforce-chpkgstunforce pkgstunforce-chpkgstforce
END
cat <<END > cat/stable/stable-1.ebuild || exit 1
KEYWORDS="test"
IUSE="
notstmask notpkgstmask notstforce notpkgstforce
stmask pkgstmask stforce pkgstforce
mask-stunmask unmask-stmask
stmask-pkgunmask stunmask-pkgmask
pkgmask-pkgstunmask pkgunmask-pkgstmask
pkgstmask-chunmask pkgstunmask-chmask
pkgstmask-chpkgstunmask pkgstunmask-chpkgstmask
force-stunforce unforce-stforce
stforce-pkgunforce stunforce-pkgforce
pkgforce-pkgstunforce pkgunforce-pkgstforce
pkgstforce-chunforce pkgstunforce-chforce
pkgstforce-chpkgstunforce pkgstunforce-chpkgstforce
"
SLOT="0"
END
cp cat/stable/stable-{1,1-r1}.ebuild || exit 1
cp cat/stable/stable-{1,2}.ebuild || exit 1
sed -e '/KEYWORDS/s/test/~test/' cat/stable/stable-1.ebuild > cat/unstable/unstable-1.ebuild || exit 1
sed -e '/KEYWORDS/s/test/detest/' cat/stable/stable-1.ebuild > cat/missing/missing-1.ebuild || exit 1
cd ..

mkdir -p repo10/{eclass,distfiles,profiles/profile/subprofile,cat/masked,cat/not_masked,cat/was_masked} || exit 1
cd repo10 || exit 1
echo "test-repo-10" > profiles/repo_name || exit 1
cat <<END >profiles/profiles.desc || exit 1
test profile stable
test profile/subprofile stable
END
cat <<END >profiles/profile/make.defaults || exit 1
ARCH=test
USE="flag1 flag2 flag3 -flag4 -flag5"
END
cat <<END >profiles/categories || exit 1
cat
END
cat <<END >profiles/profile/package.mask
cat/masked
cat/was_masked
END
cat <<END >profiles/profile/subprofile/package.mask
-cat/was_masked
END
cat <<END >profiles/profile/subprofile/parent
..
END
cat <<END > cat/masked/masked-0.ebuild
KEYWORDS="test"
END
cat <<END > cat/not_masked/not_masked-0.ebuild
KEYWORDS="test"
END
cat <<END > cat/was_masked/was_masked-0.ebuild
KEYWORDS="test"
END
cd ..

mkdir -p repo11/{eclass,distfiles,metadata,profiles/profile} || exit 1
mkdir -p repo11/category/package{,-b}/files || exit 1
cd repo11 || exit 1
echo "manifest-hashes = RMD160 SHA1 SHA256" >> metadata/layout.conf || exit 1
echo "thin-manifests = false" >> metadata/layout.conf || exit 1
echo "test-repo-11" >> profiles/repo_name || exit 1
echo "category" >> profiles/categories || exit 1
cat <<END > profiles/profile/make.defaults
ARCH=test
END
cat <<END > category/package/package-1.ebuild || exit 1
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI="foo"
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"
DEPEND=""
END
cat <<END > category/package/package-2.ebuild || exit 1
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI="bar"
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"
DEPEND=""
END
cat <<END > category/package/ChangeLog || exit 1
The times, they are-a changin'...
END
cat <<END > category/package/metadata.xml || exit
This isn't valid xml & I don't care!
END
cat <<END > category/package/files/some.patch || exit 1
+ Manifest2
END
echo "something" > distfiles/foo || exit 1
echo "for nothing" > distfiles/bar || exit 1
cat <<END > Manifest_correct || exit 1
AUX some.patch 12 RMD160 e7f53a2bea1265ef55ae5494ef6050ff7fd1a900 SHA1 eaa98a50a0db46d8ab1b02aacbfe511ea2273234 SHA256 26955b4e2d4f60561b8002b72c34ce266f534a4f32f13a29aa33875d39d31cc4
DIST bar 12 RMD160 90bd2a71cf9d8cf744b0afc0e9a00b999bb59f72 SHA1 f27a44461791182539e3d22516148b1e5289fdce SHA256 27cd06afc317a809116e7730736663b9f09dd863fcc37b69d32d4f5eb58708b2
DIST foo 10 RMD160 9e19cc1527a061585aa02dae8b7f4047dcd16275 SHA1 50a4e988380c09d290acdab4bd53d24ee7b497df SHA256 4bc453b53cb3d914b45f4b250294236adba2c0e09ff6f03793949e7e39fd4cc1
EBUILD package-1.ebuild 134 RMD160 0a59df8f246cd5d9f83b89b36026701f1bfe235b SHA1 ba3e8f6f99abdd220fc9d51c6370e4f29f8c74af SHA256 4d58e5622889397ff6a257d87652a8220585c4d97efbf0a42bf59b3f75d19e03
EBUILD package-2.ebuild 134 RMD160 d8149a3828ea05849c7033c431d3df5c6eaab67d SHA1 8ae9fc2476191906c8827b42ca86279d6fa0aead SHA256 3fb00f77d96c3e6576c2d424d31023958b507bdf20eb6555e89a135b37a54c07
MISC ChangeLog 34 RMD160 64ae4731e1de8dc8d81f0504c22e586358a5b6f0 SHA1 4de9269f6cdd0cf97c11c4a552cf3764301f40c0 SHA256 a8dfbbc187c93c0731fa9722aff87c437a4b9f59b1786d62651fb104b0c3ed97
MISC metadata.xml 37 RMD160 52a6de8e54eeea3b5e3e8357a400fbc6d3f4062b SHA1 071fb2cfe08b9fa1816ec14d65d2e9099810b552 SHA256 ba3b181b832c002612fba7768c95e526e188658d8fc85b92c153940ad43169de
END
cat <<END > category/package-b/package-b-1.ebuild || exit 1
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI="fooz"
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"
DEPEND=""
END
cd ..

mkdir -p repo11a/{eclass,distfiles,metadata,profiles/profile} || exit 1
mkdir -p repo11a/category/package/files || exit 1
cd repo11a || exit 1
echo "manifest-hashes = SHA256 SHA512 WHIRLPOOL BLAKE2B" > metadata/layout.conf || exit 1
echo "thin-manifests = false" >> metadata/layout.conf || exit 1
echo "test-repo-11a" >> profiles/repo_name || exit 1
echo "category" >> profiles/categories || exit 1
cat <<END > profiles/profile/make.defaults
ARCH=test
END
cat <<END > category/package/package-1.ebuild || exit 1
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI="foo"
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"
DEPEND=""
END
cat <<END > category/package/package-2.ebuild || exit 1
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI="bar"
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"
DEPEND=""
END
cat <<END > category/package/ChangeLog || exit 1
The times, they are-a changin'...
END
cat <<END > category/package/metadata.xml || exit
This isn't valid xml & I don't care!
END
cat <<END > category/package/files/some.patch || exit 1
+ Manifest2
END
echo "something" > distfiles/foo || exit 1
echo "for nothing" > distfiles/bar || exit 1
cat <<END > Manifest_correct || exit 1
AUX some.patch 12 BLAKE2B 17158b4a2eefab33741ab2cd708d150335532f24832e98879ffc5d038015fc3833b494251e897fb5e3b0029e41e4fbc7400b56beba5426fcdb0e18ac98c004c7 SHA256 26955b4e2d4f60561b8002b72c34ce266f534a4f32f13a29aa33875d39d31cc4 SHA512 0b502928483c249c0cd142c771bcd0bc870ea422f1f5a5d69068a6e3d8417ea2d6660cf3fb68c8600cc33d91ca9cfc32762961696186801d4641829445ba1652 WHIRLPOOL fc49ed1a58063cfc598e20473e5e625582d8733b0306aabcecda642d985bc129bf083680403a4eec530708ac5f671610626ba6817c078cf43d4dcb47fd38ad36
DIST bar 12 BLAKE2B cc035f9c66396c7e9a1bc7eda86cdcf9f13a1e910e203227d4771349889a6053a2403351082c06b13901530d650cddfc16223b27d2fa3712696d835b1ec30f0d SHA256 27cd06afc317a809116e7730736663b9f09dd863fcc37b69d32d4f5eb58708b2 SHA512 eb482b4b17a46dbf7023c5caf76aed468ee32559b715df5a1539089c522913f612d7e780edca54546e8300813b41687550176be60899474ee8373183a19e98b0 WHIRLPOOL cbdc7a79ed68423b7d9fd25fc9f1c1cd01dfad53eca3d586083861918357d2081166b7939702eddf88a72ea8494694348b94a4df07775c2a7b1d1830470810ea
DIST foo 10 BLAKE2B 9d30180eabcd29feaafd69ea83ef12fa08039e79c59b68222540b707b32ab2ae7cb624995856ab3f61fd0ea60e8ac7288c3b6a5626fb523da38b50f47272d235 SHA256 4bc453b53cb3d914b45f4b250294236adba2c0e09ff6f03793949e7e39fd4cc1 SHA512 4de57cffd81ee9dbf85831aff72914dc7ca7a7bef0466bfcda81ff3ef75d8a86d2f1c2f1bcb3afc130910384c700dd81d6a8eebdf81abfb5e61a1abcf70743fe WHIRLPOOL 02f9452201dba1f200fce2953487999b45eb85fbe1c4a518399b4640630b53b750d9bc171f37021be8e52ebc5a117394ec0435df2c2b85a1dc2e7a1e8cf75c7c
EBUILD package-1.ebuild 134 BLAKE2B 75881aa11bed402464dbf11c74dfa192b642536ca6f272812e3eb4b5a17a763f15b584c4ac3914691b70eaa6d563290ac3ff4451599d6d97052d14f0220db434 SHA256 4d58e5622889397ff6a257d87652a8220585c4d97efbf0a42bf59b3f75d19e03 SHA512 c85f4539b0ba05b97c9cb75b226e211cf1ce1ac5619d4db344d1cad98235a9ee0839e7c0bd4e6ffbca96fea8f02fb76b4a63357e7c447abc678f0a3a9f609eef WHIRLPOOL 1c97d0f2f8c805de5238ab46a83ee05780ec6bb2cd0aaae5960b77a5ead328c570aeb5eec107a0247699a4b0fadbf9c833737a0b4a75905528c787f75a9607f9
EBUILD package-2.ebuild 134 BLAKE2B c57c8071cdfe54172a6b6648b3c973826f147f064b49d4ba958d1779184002dba752965854c04aa430282427a006f73d38af2a133b394cf0fe81fb6316dad30e SHA256 3fb00f77d96c3e6576c2d424d31023958b507bdf20eb6555e89a135b37a54c07 SHA512 79b54d6aaa773540c77d943891957dbc060bfc714cc210343a7969eae96d64d43896f5221907a9dca5fe4de74e58dd6a36a943448e64c4ccd709c7391b3a7538 WHIRLPOOL 9212c730e541042be975e49f4ccd09816b9d02126c2cac3908296450d067bd0d2e39ec7120f1fbf75a9a32fb65ce8a2e860225da438f6cda89a7921ae56b8972
MISC ChangeLog 34 BLAKE2B 020e3c6bb3e8a7ae31902fe919a55bd55097bc8074320fbe8616a3facd0b45eac63109bbc22088fa7c20b8ae549889447710e4b6df0a30bba0004e349d97c1ea SHA256 a8dfbbc187c93c0731fa9722aff87c437a4b9f59b1786d62651fb104b0c3ed97 SHA512 03d8f86f43de02a64a64f515a5a7ae97b544202ed60544b33814569d4b1502d1c9ce5f2e8e50a107aa2b08a0b127a815f90f11830197a0ecf34b67c019c0625f WHIRLPOOL 411b585cca9c3dcdf609967efc86f1996c2f1fd8a9d6b62549b9099611e312760c572ccbc6384bb0beab4a78b4b354dcf90284f4dab6640972fb38f8d944fcb5
MISC metadata.xml 37 BLAKE2B a0b8ff9a5fe6786ba26764e89f0d01f5a6e09235840de932906f6af3ad6f818f19ae0a5b35e16eec705783793d54d5342e5e337eb45c3ce960381c896b1fd7d5 SHA256 ba3b181b832c002612fba7768c95e526e188658d8fc85b92c153940ad43169de SHA512 5120780bfcd7d0999bf108adfd02ddc1fa3d75666649fb64cf521fd1a94bb9653882a2a8f77d2825fe099e0a56b2858bd01a6439069585f2cc7d87f8ac5227d4 WHIRLPOOL 93bfb6ca3acc70b15dbd96bec3aa903219698bdc4aa141a2acca48d63c2928e237f4d6db524ff2ac98f4f10803726dc84975ad5ba8e41c32f6cbea901bb242aa
END
cd ..

mkdir -p repo11b/{eclass,distfiles,metadata,profiles/profile} || exit 1
mkdir -p repo11b/category/{package,package2}/files || exit 1
cd repo11b || exit 1
echo "manifest-hashes = SHA256 SHA512 WHIRLPOOL BLAKE2B" > metadata/layout.conf || exit 1
echo "thin-manifests = true" >> metadata/layout.conf || exit 1
echo "test-repo-11b" >> profiles/repo_name || exit 1
echo "category" >> profiles/categories || exit 1
cat <<END > profiles/profile/make.defaults
ARCH=test
END
cat <<END > category/package/package-1.ebuild || exit 1
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI="foo"
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"
DEPEND=""
END
cat <<END > category/package/package-2.ebuild || exit 1
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI="bar"
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"
DEPEND=""
END
cat <<END > category/package/ChangeLog || exit 1
The times, they are-a changin'...
END
cat <<END > category/package/metadata.xml || exit
This isn't valid xml & I don't care!
END
cat <<END > category/package/files/some.patch || exit 1
+ Manifest2
END
cat <<END > category/package2/package2-2.ebuild || exit 1
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"
DEPEND=""
END
echo lalala > category/package2/Manifest || exit 1
echo "something" > distfiles/foo || exit 1
echo "for nothing" > distfiles/bar || exit 1
cat <<END > Manifest_correct || exit 1
DIST bar 12 BLAKE2B cc035f9c66396c7e9a1bc7eda86cdcf9f13a1e910e203227d4771349889a6053a2403351082c06b13901530d650cddfc16223b27d2fa3712696d835b1ec30f0d SHA256 27cd06afc317a809116e7730736663b9f09dd863fcc37b69d32d4f5eb58708b2 SHA512 eb482b4b17a46dbf7023c5caf76aed468ee32559b715df5a1539089c522913f612d7e780edca54546e8300813b41687550176be60899474ee8373183a19e98b0 WHIRLPOOL cbdc7a79ed68423b7d9fd25fc9f1c1cd01dfad53eca3d586083861918357d2081166b7939702eddf88a72ea8494694348b94a4df07775c2a7b1d1830470810ea
DIST foo 10 BLAKE2B 9d30180eabcd29feaafd69ea83ef12fa08039e79c59b68222540b707b32ab2ae7cb624995856ab3f61fd0ea60e8ac7288c3b6a5626fb523da38b50f47272d235 SHA256 4bc453b53cb3d914b45f4b250294236adba2c0e09ff6f03793949e7e39fd4cc1 SHA512 4de57cffd81ee9dbf85831aff72914dc7ca7a7bef0466bfcda81ff3ef75d8a86d2f1c2f1bcb3afc130910384c700dd81d6a8eebdf81abfb5e61a1abcf70743fe WHIRLPOOL 02f9452201dba1f200fce2953487999b45eb85fbe1c4a518399b4640630b53b750d9bc171f37021be8e52ebc5a117394ec0435df2c2b85a1dc2e7a1e8cf75c7c
END
cd ..

mkdir -p repo12/{profiles/profile,metadata} || exit 1
cd repo12 || exit 1
echo "test-repo-12" >> profiles/repo_name || exit 1
echo "cat" >> metadata/categories.conf || exit 1
cat <<END > profiles/profile/make.defaults
END
mkdir -p packages/cat/no-files
cat <<'END' > packages/cat/no-files/no-files-1.exheres-0 || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS=""
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"
DEPENDENCIES=""
END
mkdir -p packages/cat/fetched-files
cat <<'END' > packages/cat/fetched-files/fetched-files-1.exheres-0 || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS="file:///var/empty/already-fetched.txt"
SLOT="0"
MYOPTIONS=""
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"
DEPENDENCIES=""
END
mkdir -p packages/cat/fetchable-files
cat <<END > packages/cat/fetchable-files/fetchable-files-1.exheres-0 || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS="file:///$(dirname $(pwd ) )/fetchable/fetchable-1.txt"
SLOT="0"
MYOPTIONS=""
LICENCES="GPL-2"
PLATFORMS="test"
DEPENDENCIES=""
END
mkdir -p packages/cat/arrow-files
cat <<END > packages/cat/arrow-files/arrow-files-1.exheres-0 || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS="file:///$(dirname $(pwd ) )/fetchable/fetchable-1.txt -> arrowed.txt"
SLOT="0"
MYOPTIONS=""
LICENCES="GPL-2"
PLATFORMS="test"
WORK="\${WORKBASE}"
DEPENDENCIES=""
END
mkdir -p packages/cat/unfetchable-files
cat <<'END' > packages/cat/unfetchable-files/unfetchable-files-1.exheres-0 || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS="file:///var/empty/unfetchable-file.txt"
SLOT="0"
MYOPTIONS=""
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"
DEPENDENCIES=""
END
mkdir -p packages/cat/no-files-restricted
cat <<'END' > packages/cat/no-files-restricted/no-files-restricted-1.exheres-0 || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS=""
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"
DEPENDENCIES=""
END
mkdir -p packages/cat/fetched-files-restricted
cat <<'END' > packages/cat/fetched-files-restricted/fetched-files-restricted-1.exheres-0 || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS="manual: file:///var/empty/already-fetched.txt"
SLOT="0"
MYOPTIONS=""
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"
DEPENDENCIES=""
END
mkdir -p packages/cat/fetchable-files-restricted
cat <<END > packages/cat/fetchable-files-restricted/fetchable-files-restricted-1.exheres-0 || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS="manual: file:///$(dirname $(pwd ) )/fetchable/fetchable-2.txt"
SLOT="0"
MYOPTIONS=""
LICENCES="GPL-2"
PLATFORMS="test"
WORK="\${WORKBASE}"
DEPENDENCIES=""
END
cd ..

mkdir -p repo16/{eclass,distfiles,profiles/profile} || exit 1
mkdir -p repo16/category/package-{a,b,c} || exit 1
cd repo16 || exit 1
echo "test-repo-16" >> profiles/repo_name || exit 1
echo "category" >> profiles/categories || exit 1
cat <<END > profiles/profile/make.defaults
ARCH=test
END
cat <<END > category/package-a/package-a-1.ebuild || exit 1
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI="foo"
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"
DEPEND=""
END
cat <<END > category/package-b/package-b-1.ebuild || exit 1
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI="bar"
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"
DEPEND=""
END
cat <<END > category/package-c/package-c-1.ebuild || exit 1
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI="foo bar"
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"
DEPEND=""
END
echo "something" > distfiles/foo || exit 1
cat <<END > Manifest || exit 1
DIST bar 12 RMD160 90bd2a71cf9d8cf744b0afc0e9a00b999bb59f72 SHA1 f27a44461791182539e3d22516148b1e5289fdce SHA256 27cd06afc317a809116e7730736663b9f09dd863fcc37b69d32d4f5eb58708b2
DIST foo 10 RMD160 9e19cc1527a061585aa02dae8b7f4047dcd16275 SHA1 50a4e988380c09d290acdab4bd53d24ee7b497df SHA256 4bc453b53cb3d914b45f4b250294236adba2c0e09ff6f03793949e7e39fd4cc1
END
cp Manifest category/package-a/
cp Manifest category/package-b/
cp Manifest category/package-c/
rm Manifest
cd ..

mkdir -p repo18/{eclass,distfiles,profiles/profile} || exit 1
mkdir -p repo18/category/package || exit 1
cd repo18 || exit 1
echo test-repo-18 >> profiles/repo_name || exit 1
echo category >> profiles/categories || exit 1
cat <<END > profiles/profile/make.defaults || exit 1
ARCH=test
END
cat <<END > profiles/package.mask || exit 1
=category/package-1
=category/package-2
END
cat <<END > category/package/package-1.ebuild || exit 1
SLOT="0"
KEYWORDS="test"
END
cat <<END > category/package/package-2.ebuild || exit 1
SLOT="0"
KEYWORDS="test"
END
cat <<END > category/package/package-3.ebuild || exit 1
SLOT="0"
KEYWORDS="test"
END
cat <<END > category/package/package-4.ebuild || exit 1
SLOT="0"
KEYWORDS="test"
END
cd ..

mkdir -p repo19/profiles || exit 1
mkdir -p repo19/category/package || exit 1
cd repo19 || exit 1
echo test-repo-19 >> profiles/repo_name || exit 1
cat <<END > profiles/package.mask || exit 1
-category/package
-=category/package-2
=category/package-3
END
cat <<END > category/package/package-1.ebuild || exit 1
SLOT="0"
KEYWORDS="test"
END
cat <<END > category/package/package-2.ebuild || exit 1
SLOT="0"
KEYWORDS="test"
END
cat <<END > category/package/package-3.ebuild || exit 1
SLOT="0"
KEYWORDS="test"
END
cat <<END > category/package/package-4.ebuild || exit 1
SLOT="0"
KEYWORDS="test"
END
cd ..

mkdir -p repo20/profiles/profile || exit 1
mkdir -p repo20/category/package || exit 1
cd repo20 || exit 1
echo test-repo-20 >> profiles/repo_name || exit 1
echo "category" >> profiles/categories || exit 1
cat <<END > profiles/profile/make.defaults
ARCH=test
END
spaces="   "
tab=$'\t'
cat <<END > category/package/package-0.ebuild || exit 1
SLOT="0"
END
cat <<END > category/package/package-1.ebuild || exit 1
EAPI=0
SLOT="0"
END
cat <<END > category/package/package-2.ebuild || exit 1
EAPI=
SLOT="0"
END
cat <<END > category/package/package-3.ebuild || exit 1
#EAPI=1
SLOT="0"
END
cat <<END > category/package/package-4.ebuild || exit 1
EAPI=1
SLOT="0"
END
cat <<END > category/package/package-5.ebuild || exit 1

EAPI=1
SLOT="0"
END
cat <<END > category/package/package-6.ebuild || exit 1
${spaces}
EAPI=1
SLOT="0"
END
cat <<END > category/package/package-7.ebuild || exit 1
${tab}
EAPI=1
SLOT="0"
END
cat <<END > category/package/package-8.ebuild || exit 1
# 123
EAPI=1
SLOT="0"
END
cat <<END > category/package/package-9.ebuild || exit 1
   EAPI=1
SLOT="0"
END
cat <<END > category/package/package-10.ebuild || exit 1
${tab}EAPI=1
SLOT="0"
END
cat <<END > category/package/package-11.ebuild || exit 1
EAPI=1${spaces}
SLOT="0"
END
cat <<END > category/package/package-12.ebuild || exit 1
EAPI=1${tab}
SLOT="0"
END
cat <<END > category/package/package-13.ebuild || exit 1
EAPI=1   # 123
SLOT="0"
END
cat <<END > category/package/package-14.ebuild || exit 1
EAPI=1${tab}# 123
SLOT="0"
END
cat <<END > category/package/package-15.ebuild || exit 1
EAPI=1   123
SLOT="0"
END
cat <<END > category/package/package-16.ebuild || exit 1
EAPI=not-a-real-eapi
SLOT="0"
END
cat <<END > category/package/package-17.ebuild || exit 1
EAPI=@
SLOT="0"
END
cat <<END > category/package/package-18.ebuild || exit 1
EAPI=1@
SLOT="0"
END
cat <<END > category/package/package-19.ebuild || exit 1
EAPI="1"
SLOT="0"
END
cat <<END > category/package/package-20.ebuild || exit 1
EAPI="1"123
SLOT="0"
END
cat <<END > category/package/package-21.ebuild || exit 1
EAPI="1"   123
SLOT="0"
END
cat <<END > category/package/package-22.ebuild || exit 1
EAPI="1"   # 123
SLOT="0"
END
cat <<END > category/package/package-23.ebuild || exit 1
EAPI="1
"
SLOT="0"
END
cat <<END > category/package/package-24.ebuild || exit 1
EAPI="1'
"
SLOT="0"
END
cat <<END > category/package/package-25.ebuild || exit 1
EAPI='1'
SLOT="0"
END
cat <<END > category/package/package-26.ebuild || exit 1
EAPI='1
'
SLOT="0"
END
cat <<END > category/package/package-27.ebuild || exit 1
EAPI='1"
'
SLOT="0"
END
cat <<END > category/package/package-28.ebuild || exit 1
EAPI=1
EAPI=2
SLOT="0"
END
cat <<END > category/package/package-29.ebuild || exit 1
EAPI=1
EAPI=2
EAPI=1
SLOT="0"
END
cat <<END > category/package/package-30.ebuild || exit 1
SLOT="0"
EAPI=0
END
cat <<END > category/package/package-31.ebuild || exit 1
SLOT="0"
EAPI=1
END
cat <<END > category/package/package-32.ebuild || exit 1
printf -v EAPI 1
SLOT="0"
END
cat <<END > category/package/package-33.exheres-0 || exit 1
SLOT="0"
END
cat <<END > category/package/package-34.exheres-0 || exit 1
EAPI=exheres-0
SLOT="0"
END
cat <<END > category/package/package-35.exheres-0 || exit 1
EAPI=1
SLOT="0"
END
cd ..

cd ..

