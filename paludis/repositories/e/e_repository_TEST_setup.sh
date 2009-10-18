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


mkdir -p repo9/{eclass,distfiles,profiles/{profile,child},cat-one/pkg-one,cat-two/pkg-one} || exit 1
mkdir -p repo9/{cat-one/pkg-one,cat-two/pkg-two} || exit 1
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

mkdir -p repo11/{eclass,distfiles,profiles/profile} || exit 1
mkdir -p repo11/category/package{,-b}/files || exit 1
cd repo11 || exit 1
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

mkdir -p repo13/{profiles/profile,metadata,eclass} || exit 1
cd repo13 || exit 1
echo "test-repo-13" >> profiles/repo_name || exit 1
echo "cat" >> profiles/categories || exit 1
cat <<END > profiles/profile/virtuals
virtual/virtual-pretend-installed cat/pretend-installed
virtual/virtual-doesnotexist cat/doesnotexist
END
cat <<END > profiles/profile/make.defaults
ARCH="cheese"
USERLAND="GNU"
KERNEL="linux"
LIBC="glibc"
CHOST="i286-badger-linux-gnu"
LINGUAS="enabled_en enabled_en_GB enabled_en_GB@UTF-8"
USE_EXPAND="LINGUAS USERLAND"
USE_EXPAND_UNPREFIXED="ARCH"
USE_EXPAND_IMPLICIT="USERLAND ARCH"
USE_EXPAND_VALUES_USERLAND="GNU"
USE_EXPAND_VALUES_ARCH="cheese otherarch"
IUSE_IMPLICIT="build"
END
mkdir -p "cat/in-ebuild-die"
cat <<END > cat/in-ebuild-die/in-ebuild-die-1.ebuild || exit 1
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"

pkg_setup() {
    die "boom"
}
END
mkdir -p "cat/in-subshell-die"
cat <<'END' > cat/in-subshell-die/in-subshell-die-1.ebuild || exit 1
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"

pkg_setup() {
    ( hasq test $KEYWORDS && die "boom" )
}
END
mkdir -p "cat/success"
cat <<END > cat/success/success-1.ebuild || exit 1
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

pkg_setup() {
    useq spork && die "boom"
}
END
mkdir -p "cat/unpack-die"
cat <<END > cat/unpack-die/unpack-die-1.ebuild || exit 1
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

src_unpack() {
    echo "123" > f.bz2
    unpack ./f.bz2
}
END
mkdir -p "cat/econf-die"
cat <<END > cat/econf-die/econf-die-1.ebuild || exit 1
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

src_compile() {
    econf
}
END
mkdir -p "cat/emake-fail"
cat <<END > cat/emake-fail/emake-fail-1.ebuild || exit 1
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

src_compile() {
    emake monkey
}
END
mkdir -p "cat/emake-die"
cat <<END > cat/emake-die/emake-die-1.ebuild || exit 1
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

src_compile() {
    emake monkey || die
}
END
mkdir -p "cat/einstall-die"
cat <<END > cat/einstall-die/einstall-die-1.ebuild || exit 1
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

src_install() {
    einstall
}
END
mkdir -p "cat/keepdir-die"
cat <<"END" > cat/keepdir-die/keepdir-die-1.ebuild || exit 1
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

src_install() {
    dodir /usr/share
    touch "${D}"/usr/share/monkey
    keepdir /usr/share/monkey
}
END
mkdir -p "cat/dobin-fail"
cat <<END > cat/dobin-fail/dobin-fail-1.ebuild || exit 1
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

src_install() {
    dobin monkey
}
END
mkdir -p "cat/dobin-die"
cat <<END > cat/dobin-die/dobin-die-1.ebuild || exit 1
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

src_install() {
    dobin monkey || die
}
END
mkdir -p "cat/fperms-fail"
cat <<END > cat/fperms-fail/fperms-fail-1.ebuild || exit 1
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

src_install() {
    fperms 755 monkey
}
END
mkdir -p "cat/fperms-die"
cat <<END > cat/fperms-die/fperms-die-1.ebuild || exit 1
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

src_install() {
    fperms 755 monkey || die
}
END
mkdir -p "cat/pretend-installed"
cat <<END > cat/pretend-installed/pretend-installed-2.ebuild || exit 1
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

src_compile() {
    die "not supposed to install this"
}
END
mkdir -p "cat/econf-source"
cat <<END > cat/econf-source/econf-source-0.ebuild || exit 1
EAPI="\${PV}"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

src_unpack() {
    ECONF_SOURCE=subdir
    mkdir \${S}
    cd \${S}
    mkdir subdir
    echo 'touch monkey' > subdir/configure
    chmod +x subdir/configure
}

src_install() {
    insinto /usr/bin
    doins monkey || die "no monkey"
}
END
cp cat/econf-source/econf-source-{0,1}.ebuild || exit 1
cp cat/econf-source/econf-source-{0,2}.ebuild || exit 1
mkdir -p "cat/doman"
cat <<END > cat/doman/doman-0.ebuild || exit 1
EAPI="\${PV}"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

src_compile() {
    echo \${PF} >foo.1
    mkdir dir
    echo \${PF} >dir/foo.2
    echo \${PF} >foo.2
    echo \${PF} >foo.3x
    echo \${PF} >foo.4.gz
    echo \${PF} >foo.5f.bz2
    echo \${PF} >foo.6.Z
    echo \${PF} >foo.en.7
    echo \${PF} >foo.en_GB.8
    echo \${PF} >foo.e.9
    echo \${PF} >foo.enn.n
    echo \${PF} >foo.EN.1
    echo \${PF} >foo.en-GB.2
    echo \${PF} >foo.en_gb.3
    echo \${PF} >foo.en_G.4
    echo \${PF} >foo.en_GBB.5
    echo \${PF} >foo.nonkey
    touch foo.1x
    echo \${PF} >bar.m
    echo \${PF} >bar.monkey
    echo \${PF} >baz.6
    echo \${PF} >baz.en_US.7
}

src_install() {
    doman foo.* dir/foo.* || die
    doman bar.m && die
    doman bar.monkey && die
    doman bar.1 && die
    doman -i18n=en_GB baz.* || die
    keepdir /meh || die
    cd "\${D}"/meh || die
    doman .keep* || die
    rm "\${D}"/usr/share/man/{man1/foo.1,man2/foo.2,man3/foo.3x,man4/foo.4.gz,man5/foo.5f.bz2} || die
    rm "\${D}"/usr/share/man/{man6/foo.6.Z,man7/foo.en.7,man8/foo.en_GB.8,man9/foo.e.9,mann/foo.enn.n} || die
    rm "\${D}"/usr/share/man/{man1/foo.EN.1,man2/foo.en-GB.2,man3/foo.en_gb.3,man4/foo.en_G.4} || die
    rm "\${D}"/usr/share/man/{man5/foo.en_GBB.5,mann/foo.nonkey,en_GB/man6/baz.6,en_GB/man7/baz.en_US.7} || die
    rmdir "\${D}"/usr/share/man/{man1,man2,man3,man4,man5,man6,man7,man8,man9,mann,en_GB/man6,en_GB/man7,en_GB,} || die
}
END
cp cat/doman/doman-{0,1}.ebuild || exit 1
cat <<END > cat/doman/doman-2.ebuild || exit 1
EAPI="\${PV}"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

src_compile() {
    echo \${PF} >foo.1
    mkdir dir
    echo \${PF} >dir/foo.2
    echo \${PF} >foo.3x
    echo \${PF} >foo.4.gz
    echo \${PF} >foo.5f.bz2
    echo \${PF} >foo.6.Z
    echo \${PF} >foo.en.7
    echo \${PF} >foo.en_GB.8
    echo \${PF} >foo.e.9
    echo \${PF} >foo.enn.n
    echo \${PF} >foo.EN.1
    echo \${PF} >foo.en-GB.2
    echo \${PF} >foo.en_gb.3
    echo \${PF} >foo.en_G.4
    echo \${PF} >foo.en_GBB.5
    echo \${PF} >foo.nonkey
    touch foo.1x
    echo \${PF} >bar.m
    echo \${PF} >bar.monkey
    echo \${PF} >baz.6
    echo \${PF} >baz.en_US.7
}

src_install() {
    doman foo.* dir/foo.* || die
    doman bar.m && die
    doman bar.monkey && die
    doman bar.1 && die
    doman -i18n=en_GB baz.* || die
    keepdir /meh || die
    cd "\${D}"/meh || die
    doman .keep* || die
    rm "\${D}"/usr/share/man/{man1/foo.1,man2/foo.2,man3/foo.3x,man4/foo.4.gz,man5/foo.5f.bz2} || die
    rm "\${D}"/usr/share/man/{man6/foo.6.Z,en/man7/foo.7,en_GB/man8/foo.8,man9/foo.e.9,mann/foo.enn.n} || die
    rm "\${D}"/usr/share/man/{man1/foo.EN.1,man2/foo.en-GB.2,man3/foo.en_gb.3,man4/foo.en_G.4} || die
    rm "\${D}"/usr/share/man/{man5/foo.en_GBB.5,mann/foo.nonkey,en_GB/man6/baz.6,en_US/man7/baz.7} || die
    rmdir "\${D}"/usr/share/man/{man1,man2,man3,man4,man5,man6,man9,mann,en/man7,en_GB/man6,en_GB/man8,en_US/man7,en,en_GB,en_US,} || die
}
END
mkdir -p "cat/dosym-success"
cat <<'END' > cat/dosym-success/dosym-success-1.ebuild || exit 1
EAPI="${PV}"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

src_install() {
    dosym foo /usr/bin/bar
    [[ "$(readlink ${D}/usr/bin/bar )" == "foo" ]] || die
}
END
mkdir -p "cat/best-version"
cat <<'END' > cat/best-version/best-version-0.ebuild || exit 1
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

pkg_setup() {
    if ! best_version cat/pretend-installed >/dev/null ; then
        die "failed cat/pretend-installed"
    fi

    BV1=$(best_version cat/pretend-installed )
    [[ "$BV1" == "cat/pretend-installed-1" ]] || die "BV1 is $BV1"

    if best_version cat/doesnotexist >/dev/null ; then
        die "not failed cat/doesnotexist"
    fi

    BV2=$(best_version cat/doesnotexist )
    [[ "$BV2" == "" ]] || die "BV2 is $BV2"

    if [[ -n "$PALUDIS_ENABLE_VIRTUALS_REPOSITORY" ]] ; then
        if ! best_version virtual/virtual-pretend-installed >/dev/null ; then
            die "failed virtual/virtual-pretend-installed"
        fi

        BV3=$(best_version virtual/virtual-pretend-installed )
        [[ "$BV3" == "cat/pretend-installed-1" ]] || die "BV3 is $BV3"

        if best_version virtual/virtual-doesnotexist >/dev/null ; then
            die "not failed virtual/virtual-doesnotexist"
        fi

        BV2=$(best_version virtual/virtual-doesnotexist )
        [[ "$BV4" == "" ]] || die "BV4 is $BV4"
    fi
}
END
mkdir -p "cat/has-version"
cat <<'END' > cat/has-version/has-version-0.ebuild || exit 1
EAPI="${PV}"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

pkg_setup() {
    if ! has_version cat/pretend-installed ; then
        die "failed cat/pretend-installed"
    fi

    if has_version cat/doesnotexist >/dev/null ; then
        die "not failed cat/doesnotexist"
    fi
}
END
mkdir -p "cat/match"
cat <<'END' > cat/match/match-0.ebuild || exit 1
EAPI="${PV}"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

pkg_setup() {
    if ! portageq match "${ROOT}" cat/pretend-installed >/dev/null ; then
        die "failed cat/pretend-installed"
    fi

    cat <<'DONE' > ${T}/expected
cat/pretend-installed-0
cat/pretend-installed-1
DONE
    portageq match "${ROOT}" cat/pretend-installed > ${T}/got
    cmp ${T}/expected ${T}/got || die "oops"

    if portageq match "${ROOT}" cat/doesnotexist >/dev/null ; then
        die "not failed cat/doesnotexist"
    fi

    BV2=$(portageq match "${ROOT}" cat/doesnotexist )
    [[ "$BV2" == "" ]] || die "BV2 is $BV2"
}
END
mkdir -p "cat/vars"
cat <<'END' > cat/vars/vars-0.ebuild || exit 1
EAPI="${PV}"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

pkg_setup() {
    [[ -d "${T}" ]] || die "T not a dir"
}

src_compile() {
    [[ -d "${T}" ]] || die "T not a dir"
}

pkg_preinst() {
    [[ -d "${T}" ]] || die "T not a dir"
}
END
mkdir -p "cat/src_prepare"
cat <<END > cat/src_prepare/src_prepare-0.ebuild || exit 1
EAPI="\${PV}"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

src_prepare() {
    die src_prepare
}
END
cp cat/src_prepare/src_prepare-{0,1}.ebuild || exit 1
cp cat/src_prepare/src_prepare-{0,2}.ebuild || exit 1
mkdir -p "cat/src_configure"
cat <<END > cat/src_configure/src_configure-0.ebuild || exit 1
EAPI="\${PV}"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

src_configure() {
    die src_configure
}
END
cp cat/src_configure/src_configure-{0,1}.ebuild || exit 1
cp cat/src_configure/src_configure-{0,2}.ebuild || exit 1
mkdir -p "cat/default-src_configure" || exit 1
cat << END > cat/default-src_configure/default-src_configure-2.ebuild || exit 1
EAPI="\${PV}"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

src_unpack() {
    cat <<EOF >configure
#! /bin/sh
touch foo
EOF
    chmod +x configure
    echo 'all: ; rm foo' >Makefile
}

src_compile() {
    [[ -e foo ]] || die
}
END
mkdir -p "cat/default-src_compile" || exit 1
cat << END > cat/default-src_compile/default-src_compile-2.ebuild || exit 1
EAPI="\${PV}"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

src_unpack() {
    cat <<EOF >configure
#! /bin/sh
rm Makefile
EOF
    chmod +x configure
    echo 'all: ; touch foo' >Makefile
}

src_configure() {
    :
}

src_install() {
    [[ -e foo ]] || die
}
END
mkdir -p "cat/default_src_compile" || exit 1
cat << END > cat/default_src_compile/default_src_compile-2.ebuild || exit 1
EAPI="\${PV}"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

src_unpack() {
    cat <<EOF >configure
#! /bin/sh
rm Makefile
EOF
    chmod +x configure
    echo 'all: ; touch foo' >Makefile
}

src_configure() {
    :
}

src_compile() {
    default_src_compile
    [[ -e foo ]] || die
}
END
mkdir -p "cat/src_compile-via-default-func" || exit 1
cat << END > cat/src_compile-via-default-func/src_compile-via-default-func-2.ebuild || exit 1
EAPI="\${PV}"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

src_unpack() {
    cat <<EOF >configure
#! /bin/sh
rm Makefile
EOF
    chmod +x configure
    echo 'all: ; touch foo' >Makefile
}

src_configure() {
    :
}

src_compile() {
    default
    [[ -e foo ]] || die
}
END
mkdir -p "cat/econf-source-kdebuild"
cat <<END > cat/econf-source-kdebuild/econf-source-kdebuild-1.kdebuild-1 || exit 1
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

src_unpack() {
    ECONF_SOURCE=subdir
    mkdir \${S}
    cd \${S}
    mkdir subdir
    echo 'touch monkey' > subdir/configure
    chmod +x subdir/configure
}

src_install() {
    insinto /usr/bin
    doins monkey || die "no monkey"
}
END
mkdir -p "cat/info-success-kdebuild"
cat <<END > cat/info-success-kdebuild/info-success-kdebuild-1.kdebuild-1 || exit 1
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

pkg_info() {
    einfo "This is my pkg_info. There are many like it, but this one is mine."
}
END
mkdir -p "cat/info-fail-kdebuild"
cat <<END > cat/info-fail-kdebuild/info-fail-kdebuild-1.kdebuild-1 || exit 1
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

pkg_info() {
    die "This is my pkg_info. There are many like it, but this one is mine."
}
END
mkdir -p "cat/banned-functions-kdebuild"
cat <<END > cat/banned-functions-kdebuild/banned-functions-kdebuild-1.kdebuild-1 || exit 1
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

pkg_setup() {
    prepall
}
END
mkdir -p "cat/banned-vars-kdebuild"
cat <<END > cat/banned-vars-kdebuild/banned-vars-kdebuild-1.kdebuild-1 || exit 1
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"
PROVIDE="virtual/monkey"
END
mkdir -p "cat/dosym-success-kdebuild"
cat <<'END' > cat/dosym-success-kdebuild/dosym-success-kdebuild-1.kdebuild-1 || exit 1
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

src_install() {
    dodir /usr/bin
    dosym foo /usr/bin/bar
    [[ "$(readlink ${D}/usr/bin/bar )" == "foo" ]] || die
}
END
mkdir -p "cat/dosym-fail-kdebuild"
cat <<END > cat/dosym-fail-kdebuild/dosym-fail-kdebuild-1.kdebuild-1 || exit 1
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

src_install() {
    dosym foo /usr/bin/bar
}
END
mkdir -p "cat/doman-kdebuild"
sed -e /EAPI=/d cat/doman/doman-0.ebuild >cat/doman-kdebuild/doman-kdebuild-1.kdebuild-1 || exit 1
mkdir -p "cat/src_prepare-kdebuild"
sed -e /EAPI=/d cat/src_prepare/src_prepare-0.ebuild >cat/src_prepare-kdebuild/src_prepare-kdebuild-1.kdebuild-1 || exit 1
mkdir -p "cat/src_configure-kdebuild"
sed -e /EAPI=/d cat/src_configure/src_configure-0.ebuild >cat/src_configure-kdebuild/src_configure-kdebuild-1.kdebuild-1 || exit 1
mkdir -p "cat/expand-vars"
cat <<"END" > cat/expand-vars/expand-vars-0.ebuild || exit 1
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="enabled-weasel broccoli"
LICENSE="GPL-2"
KEYWORDS="test"

pkg_setup() {
    [[ $USE == "enabled-weasel linguas_enabled_en linguas_enabled_en_GB linguas_enabled_en_GB@UTF-8 userland_GNU cheese " ]] \
        || die "USE=$USE is wrong"
    [[ $USERLAND == "GNU" ]] || die "USERLAND=$USERLAND is wrong"
    [[ $LINGUAS == "enabled_en enabled_en_GB enabled_en_GB@UTF-8" ]] || die "LINGUAS=$LINGUAS is wrong"
}
END
mkdir -p "cat/pkg_pretend"
cat <<"END" > cat/pkg_pretend/pkg_pretend-3.ebuild || exit 1
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="enabled-weasel broccoli"
LICENSE="GPL-2"
KEYWORDS="test"
EAPI="3"

pkg_pretend() {
    einfo "This is my pkg_pretend. There are many like it, but this one is mine."
}
END
mkdir -p "cat/pkg_pretend-failure"
cat <<"END" > cat/pkg_pretend-failure/pkg_pretend-failure-3.ebuild || exit 1
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="enabled-weasel broccoli"
LICENSE="GPL-2"
KEYWORDS="test"
EAPI="3"

pkg_pretend() {
    die "This is my pkg_pretend. There are many like it, but this one is mine."
}
END
mkdir -p "cat/default_src_install" || exit 1
cat << 'END' > cat/default_src_install/default_src_install-3.ebuild || exit 1
EAPI="${PV}"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"
EAPI="3"

S="${WORKDIR}"

src_unpack() {
    mkdir -p ${WORKDIR}
    cat <<'EOF' >${WORKDIR}/Makefile
all :
	echo spork > README
	echo monkey > README.txt
	touch README.foo
	echo gerbil > GERBIL

install :
	echo spork > $(DESTDIR)/EATME
EOF
}

pkg_preinst() {
    [[ -e ${D}/usr/share/doc/${PF}/README ]] || die README
    [[ -e ${D}/usr/share/doc/${PF}/README.txt ]] || die README.txt
    [[ -e ${D}/usr/share/doc/${PF}/README.foo ]] && die README.foo
    [[ -e ${D}/usr/share/doc/${PF}/GERBIL ]] && die GERBIL
    [[ -e ${D}/EATME ]] || die EATME
}
END
mkdir -p "cat/docompress" || exit 1
cat << 'END' > cat/docompress/docompress-3.ebuild || exit 1
EAPI="${PV}"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"
EAPI="3"

S="${WORKDIR}"

src_install() {
    docompress foo || die
    docompress bar || die
}
END
mkdir -p "cat/dodoc-r" || exit 1
cat << 'END' > cat/dodoc-r/dodoc-r-3.ebuild || exit 1
EAPI="${PV}"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"
EAPI="3"

S="${WORKDIR}"

src_unpack() {
    mkdir -p ${WORKDIR}
    cd "${WORKDIR}"

    mkdir one two three
    echo foo > one/first
    echo foo > two/second
    echo foo > four
    mkdir dot
    mkdir dot/five
    echo foo > dot/five/fifth
}

src_install() {
    dodoc -r one two three four
    cd dot
    dodoc -r .
}

pkg_preinst() {
    [[ -e ${D}/usr/share/doc/${PF}/one/first ]] || die one/first
    [[ -e ${D}/usr/share/doc/${PF}/two/second ]] || die two/second
    [[ -e ${D}/usr/share/doc/${PF}/four ]] || die four
    [[ -e ${D}/usr/share/doc/${PF}/five/fifth ]] || die five/fifth
}
END
mkdir -p "cat/doins-symlink" || exit 1
cat << 'END' > cat/doins-symlink/doins-symlink-3.ebuild || exit 1
EAPI="${PV}"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"
EAPI="3"

S="${WORKDIR}"

src_unpack() {
    mkdir -p ${WORKDIR}
    cd "${WORKDIR}"

    mkdir a
    cd a
    echo qwerty > qwerty
    ln -s qwerty uiop
    ln -s qwerty adfs

    cd ..

    mkdir b
    cd b
    echo foo > foo
    ln -s foo bar

}

src_install() {
    insinto /foo
    doins a/qwerty
    doins a/uiop
    newins a/adfs asdf
    cd b
    doins -r .
}

pkg_preinst() {
    [[ -f ${D}/foo/qwerty ]] || die qwerty
    [[ -L ${D}/foo/uiop ]] || die uiop
    [[ $(readlink ${D}/foo/uiop ) == qwerty ]] || die sym
    [[ -L ${D}/foo/asdf ]] || die asdf
    [[ $(readlink ${D}/foo/asdf ) == qwerty ]] || die sym
    [[ -f ${D}/foo/foo ]] || die foo
    [[ -L ${D}/foo/bar ]] || die bar
    [[ $(readlink ${D}/foo/bar ) == foo ]] || die sym
}
END
mkdir -p "cat/banned-functions"
cat <<END > cat/banned-functions/banned-functions-3.ebuild || exit 1
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"
EAPI="3"

S="\${WORKDIR}"

src_install() {
    touch foo
    dohard foo bar
}
END
mkdir -p "cat/econf-disable-dependency-tracking" || exit 1
cat << 'END' > cat/econf-disable-dependency-tracking/econf-disable-dependency-tracking-0.ebuild || exit 1
EAPI="${PV}"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"
EAPI="0"

S="${WORKDIR}"

src_unpack() {
    mkdir -p ${WORKDIR}
    cd "${WORKDIR}"

    cat <<'EOF' > configure
#!/bin/sh

if echo "$@" | grep -q 'disable-dependency-tracking' ; then
    exit 1
fi

exit 0
EOF

    chmod +x configure
}
END
cat << 'END' > cat/econf-disable-dependency-tracking/econf-disable-dependency-tracking-3.ebuild || exit 1
EAPI="${PV}"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"
EAPI="3"

S="${WORKDIR}"

src_unpack() {
    mkdir -p ${WORKDIR}
    cd "${WORKDIR}"

    cat <<'EOF' > configure
#!/bin/sh

if ! echo "$@" | grep -q 'disable-dependency-tracking' ; then
    exit 1
fi

exit 0
EOF

    chmod +x configure
}
END
mkdir -p "cat/strict-use" || exit 1
cat << 'END' > cat/strict-use/strict-use-3.ebuild || exit 1
EAPI="${PV}"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork enabled"
LICENSE="GPL-2"
KEYWORDS="test"
EAPI="3"

S="${WORKDIR}"

pkg_setup() {
    use enabled || die "enabled not enabled"
    use spork && die "sporks are bad"
}
END
mkdir -p "cat/strict-use-fail" || exit 1
cat << 'END' > cat/strict-use-fail/strict-use-fail-3.ebuild || exit 1
EAPI="${PV}"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork enabled"
LICENSE="GPL-2"
KEYWORDS="test"
EAPI="3"

S="${WORKDIR}"

pkg_setup() {
    use pony
}
END
mkdir -p "cat/strict-use-injection" || exit 1
cat << 'END' > cat/strict-use-injection/strict-use-injection-3.ebuild || exit 1
EAPI="${PV}"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork enabled"
LICENSE="GPL-2"
KEYWORDS="test"
EAPI="3"

S="${WORKDIR}"

pkg_setup() {
    use build && die "build set"
    use userland_GNU || die "userland_GNU not set"
    use cheese || die "cheese not set"
    use otherarch && die "otherarch set"
}
END
mkdir -p "cat/global-scope-use" || exit 1
cat << 'END' > cat/global-scope-use/global-scope-use-3.ebuild || exit 1
use spork

EAPI="${PV}"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork enabled"
LICENSE="GPL-2"
KEYWORDS="test"
EAPI="3"

S="${WORKDIR}"
END
cd ..

mkdir -p repo14/{profiles/profile,metadata,eclass} || exit 1
cd repo14 || exit 1
echo "test-repo-14" >> profiles/repo_name || exit 1
cat <<END > profiles/profile/virtuals
virtual/virtual-pretend-installed cat/pretend-installed
virtual/virtual-doesnotexist cat/doesnotexist
END
echo "cat" >> metadata/categories.conf || exit 1
cat <<END > profiles/profile/make.defaults
CHOST="i286-badger-linux-gnu"
SUBOPTIONS="LINGUAS"
LINGUAS="en en_GB en_GB@UTF-8"
USERLAND="GNU"
OPTIONS="weasel spinach"
USE_EXPAND="USERLAND"
END
mkdir -p "packages/cat/in-ebuild-die"
cat <<'END' > packages/cat/in-ebuild-die/in-ebuild-die-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS=""
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

pkg_setup() {
    die "boom"
}
END
mkdir -p "packages/cat/in-subshell-die"
cat <<'END' > packages/cat/in-subshell-die/in-subshell-die-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS=""
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

pkg_setup() {
    ( hasq test $PLATFORMS && die "boom" )
}
END
mkdir -p "packages/cat/success"
cat <<'END' > packages/cat/success/success-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

pkg_setup() {
    optionq spork && die "boom"
}
END
mkdir -p "packages/cat/expatch-success"
cat <<'END' > packages/cat/expatch-success/expatch-success-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_unpack() {
    echo foo > bar
    echo -e 'foo\nbar\nbaz' > baz
}

src_prepare() {
    expatch "${FETCHEDDIR}"/${PNV}.patch
    diff bar baz || die "expatch failed"
}
END
mkdir -p "packages/cat/expatch-success-dir/files/expatch-success-dir"
cat <<END > packages/cat/expatch-success-dir/files/expatch-success-dir/foo.patch || exit 1
--- a/bar
+++ b/bar
@@ -1 +1,3 @@
 foo
+bar
+baz
END
cat <<'END' > packages/cat/expatch-success-dir/expatch-success-dir-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_unpack() {
    echo foo > bar
    echo -e 'foo\nbar\nbaz' > baz
}

src_prepare() {
    expatch "${FILES}"/${PN}/
    diff bar baz || die "expatch failed"
}
END
mkdir -p "packages/cat/expatch-die"
cat <<'END' > packages/cat/expatch-die/expatch-die-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_prepare() {
    expatch monkey.patch
}
END
mkdir -p "packages/cat/expatch-unrecognised/files/expatch-unrecognised"
cat <<END > packages/cat/expatch-unrecognised/files/expatch-unrecognised/foo || exit 1
--- a/bar
+++ b/bar
@@ -1 +1,3 @@
 foo
+bar
+baz
END
cat <<'END' > packages/cat/expatch-unrecognised/expatch-unrecognised-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_unpack() {
    echo foo > bar
    echo foo > baz
}

src_prepare() {
    expatch "${FILES}"/${PN}/
    diff bar baz || die "expatch applied unrecognised patch"
}
END
mkdir -p "packages/cat/nonfatal-expatch-fail"
cat <<'END' > packages/cat/nonfatal-expatch-fail/nonfatal-expatch-fail-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_prepare() {
    nonfatal expatch monkey.patch
}
END
mkdir -p "packages/cat/nonfatal-expatch-die"
cat <<'END' > packages/cat/nonfatal-expatch-die/nonfatal-expatch-die-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_prepare() {
    nonfatal expatch monkey.patch || die
}
END
mkdir -p "packages/cat/unpack-die"
cat <<'END' > packages/cat/unpack-die/unpack-die-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_unpack() {
    echo "123" > f.bz2
    unpack ./f.bz2
}
END
mkdir -p "packages/cat/nonfatal-unpack-fail"
cat <<'END' > packages/cat/nonfatal-unpack-fail/nonfatal-unpack-fail-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_unpack() {
    echo "123" > f.bz2
    nonfatal unpack ./f.bz2
}
END
mkdir -p "packages/cat/nonfatal-unpack-die"
cat <<'END' > packages/cat/nonfatal-unpack-die/nonfatal-unpack-die-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_unpack() {
    echo "123" > f.bz2
    nonfatal unpack ./f.bz2 || die
}
END
mkdir -p "packages/cat/econf-fail"
cat <<'END' > packages/cat/econf-fail/econf-fail-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_configure() {
    econf
}
END
mkdir -p "packages/cat/nonfatal-econf"
cat <<'END' > packages/cat/nonfatal-econf/nonfatal-econf-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_configure() {
    nonfatal econf
}
END
mkdir -p "packages/cat/nonfatal-econf-die"
cat <<'END' > packages/cat/nonfatal-econf-die/nonfatal-econf-die-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_configure() {
    nonfatal econf || die
}
END
mkdir -p "packages/cat/emake-fail"
cat <<'END' > packages/cat/emake-fail/emake-fail-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_compile() {
    emake monkey
}
END
mkdir -p "packages/cat/nonfatal-emake"
cat <<'END' > packages/cat/nonfatal-emake/nonfatal-emake-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_compile() {
    nonfatal emake monkey
}
END
mkdir -p "packages/cat/nonfatal-emake-die"
cat <<'END' > packages/cat/nonfatal-emake-die/nonfatal-emake-die-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_compile() {
    nonfatal emake monkey || die
}
END
mkdir -p "packages/cat/keepdir-success"
cat <<'END' > packages/cat/keepdir-success/keepdir-success-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_install() {
    keepdir /usr/share/monkey
}
END
mkdir -p "packages/cat/keepdir-fail"
cat <<'END' > packages/cat/keepdir-fail/keepdir-fail-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_install() {
    dodir /usr/share
    touch "${IMAGE}"/usr/share/monkey
    keepdir /usr/share/monkey
}
END
mkdir -p "packages/cat/nonfatal-keepdir"
cat <<'END' > packages/cat/nonfatal-keepdir/nonfatal-keepdir-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_install() {
    dodir /usr/share
    touch "${IMAGE}"/usr/share/monkey
    nonfatal keepdir /usr/share/monkey
}
END
mkdir -p "packages/cat/nonfatal-keepdir-die"
cat <<'END' > packages/cat/nonfatal-keepdir-die/nonfatal-keepdir-die-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_install() {
    dodir /usr/share
    touch "${IMAGE}"/usr/share/monkey
    nonfatal keepdir /usr/share/monkey || die
}
END
mkdir -p "packages/cat/einstall-fail"
cat <<'END' > packages/cat/einstall-fail/einstall-fail-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_install() {
    einstall
}
END
mkdir -p "packages/cat/nonfatal-einstall"
cat <<'END' > packages/cat/nonfatal-einstall/nonfatal-einstall-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_install() {
    nonfatal einstall
}
END
mkdir -p "packages/cat/nonfatal-einstall-die"
cat <<'END' > packages/cat/nonfatal-einstall-die/nonfatal-einstall-die-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_install() {
    nonfatal einstall || die
}
END
mkdir -p "packages/cat/dobin-success"
cat <<'END' > packages/cat/dobin-success/dobin-success-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_compile() {
    touch foo
}

src_install() {
    dobin foo
}
END
mkdir -p "packages/cat/dobin-fail"
cat <<'END' > packages/cat/dobin-fail/dobin-fail-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_install() {
    dobin monkey
}
END
mkdir -p "packages/cat/nonfatal-dobin-success"
cat <<'END' > packages/cat/nonfatal-dobin-success/nonfatal-dobin-success-1.ebuild || exit 1
DESCRIPTION="The Lnog Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_compile() {
    touch foo
}

src_install() {
    nonfatal dobin foo || die
}
END
mkdir -p "packages/cat/nonfatal-dobin-fail"
cat <<'END' > packages/cat/nonfatal-dobin-fail/nonfatal-dobin-fail-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_install() {
    nonfatal dobin monkey
}
END
mkdir -p "packages/cat/nonfatal-dobin-die"
cat <<'END' > packages/cat/nonfatal-dobin-die/nonfatal-dobin-die-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_install() {
    nonfatal dobin monkey || die
}
END
mkdir -p "packages/cat/herebin-success"
cat <<'END' > packages/cat/herebin-success/herebin-success-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_install() {
    herebin foo <<EOF
blah
EOF
}
END
mkdir -p "packages/cat/herebin-fail"
cat <<'END' > packages/cat/herebin-fail/herebin-fail-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_install() {
    herebin <<EOF
blah
EOF
}
END
mkdir -p "packages/cat/hereconfd-success"
cat <<'END' > packages/cat/hereconfd-success/hereconfd-success-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_install() {
    hereconfd foo <<EOF
blah
EOF
}
END
mkdir -p "packages/cat/hereconfd-fail"
cat <<'END' > packages/cat/hereconfd-fail/hereconfd-fail-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_install() {
    hereconfd <<EOF
blah
EOF
}
END
mkdir -p "packages/cat/hereenvd-success"
cat <<'END' > packages/cat/hereenvd-success/hereenvd-success-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_install() {
    hereenvd foo <<EOF
blah
EOF
}
END
mkdir -p "packages/cat/hereenvd-fail"
cat <<'END' > packages/cat/hereenvd-fail/hereenvd-fail-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_install() {
    hereenvd <<EOF
blah
EOF
}
END
mkdir -p "packages/cat/hereinitd-success"
cat <<'END' > packages/cat/hereinitd-success/hereinitd-success-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_install() {
    hereinitd foo <<EOF
blah
EOF
}
END
mkdir -p "packages/cat/hereinitd-fail"
cat <<'END' > packages/cat/hereinitd-fail/hereinitd-fail-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_install() {
    hereinitd <<EOF
blah
EOF
}
END
mkdir -p "packages/cat/hereins-success"
cat <<'END' > packages/cat/hereins-success/hereins-success-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_install() {
    hereins foo <<EOF
blah
EOF
}
END
mkdir -p "packages/cat/hereins-fail"
cat <<'END' > packages/cat/hereins-fail/hereins-fail-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_install() {
    hereins <<EOF
blah
EOF
}
END
mkdir -p "packages/cat/heresbin-success"
cat <<'END' > packages/cat/heresbin-success/heresbin-success-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_install() {
    heresbin foo <<EOF
blah
EOF
}
END
mkdir -p "packages/cat/heresbin-fail"
cat <<'END' > packages/cat/heresbin-fail/heresbin-fail-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_install() {
    heresbin <<EOF
blah
EOF
}
END
mkdir -p "packages/cat/fperms-success"
cat <<'END' > packages/cat/fperms-success/fperms-success-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_compile() {
    touch foo
}

src_install() {
    fperms 755 foo
}
END
mkdir -p "packages/cat/fperms-fail"
cat <<'END' > packages/cat/fperms-fail/fperms-fail-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_install() {
    fperms 755 monkey
}
END
mkdir -p "packages/cat/nonfatal-fperms-success"
cat <<'END' > packages/cat/nonfatal-fperms-success/nonfatal-fperms-success-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_compile() {
    touch foo
}

src_install() {
    nonfatal fperms 755 foo || die
}
END
mkdir -p "packages/cat/nonfatal-fperms-fail"
cat <<'END' > packages/cat/nonfatal-fperms-fail/nonfatal-fperms-fail-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_install() {
    nonfatal fperms 755 monkey
}
END
mkdir -p "packages/cat/nonfatal-fperms-die"
cat <<'END' > packages/cat/nonfatal-fperms-die/nonfatal-fperms-die-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_install() {
    nonfatal fperms 755 monkey || die
}
END
mkdir -p "packages/cat/best-version"
cat <<'END' > packages/cat/best-version/best-version-0.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

pkg_setup() {
    if ! best_version cat/pretend-installed >/dev/null ; then
        die "failed cat/pretend-installed"
    fi

    BV1=$(best_version cat/pretend-installed )
    [[ "$BV1" == "cat/pretend-installed-1:0::installed" ]] || die "BV1 is $BV1"

    if best_version cat/doesnotexist >/dev/null ; then
        die "not failed cat/doesnotexist"
    fi

    BV2=$(best_version cat/doesnotexist )
    [[ "$BV2" == "" ]] || die "BV2 is $BV2"

    if [[ -n "$PALUDIS_ENABLE_VIRTUALS_REPOSITORY" ]] ; then
        if ! best_version virtual/virtual-pretend-installed >/dev/null ; then
            die "failed virtual/virtual-pretend-installed"
        fi

        BV3=$(best_version virtual/virtual-pretend-installed )
        [[ "$BV3" == "virtual/virtual-pretend-installed-1::installed-virtuals (virtual for cat/pretend-installed-1:0::installed)" ]] \
            || die "BV3 is $BV3"

        if best_version virtual/virtual-doesnotexist >/dev/null ; then
            die "not failed virtual/virtual-doesnotexist"
        fi

        BV2=$(best_version virtual/virtual-doesnotexist )
        [[ "$BV4" == "" ]] || die "BV4 is $BV4"
    fi
}
END
mkdir -p "packages/cat/has-version"
cat <<'END' > packages/cat/has-version/has-version-0.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

pkg_setup() {
    if ! has_version cat/pretend-installed ; then
        die "failed cat/pretend-installed"
    fi

    if has_version cat/doesnotexist >/dev/null ; then
        die "not failed cat/doesnotexist"
    fi
}
END
mkdir -p "packages/cat/match"
cat <<'END' > packages/cat/match/match-0.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

pkg_setup() {
    portageq match "${ROOT}" cat/foo | while read a ; do
        einfo moo
    done
}
END
mkdir -p "packages/cat/econf-phase"
cat <<'END' > packages/cat/econf-phase/econf-phase-0.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"

HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_unpack() {
    mkdir ${WORK}
    echo "#!/bin/bash" > ${WORK}/configure
    chmod +x ${WORK}/configure
}

src_compile() {
    econf
}
END
mkdir -p "packages/cat/econf-vars"
cat <<'END' > packages/cat/econf-vars/econf-vars-0.ebuild || exit 1
DESCRIPTION="The Long Description"
DESCRIPTION="The Short Description"

HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="enabled-hamster gerbil dormouse"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

DEFAULT_SRC_CONFIGURE_PARAMS=( --nice-juicy-steak )
DEFAULT_SRC_CONFIGURE_OPTION_ENABLES=( enabled-hamster gerbil )
DEFAULT_SRC_CONFIGURE_OPTION_WITHS=( dormouse )

src_unpack() {
    mkdir ${WORK}
    cat <<'END2' > ${WORK}/configure
#!/usr/bin/env bash
echo "${@}" | grep -q -- '--enable-enabled-hamster' || exit 1
echo "${@}" | grep -q -- '--disable-gerbil' || exit 2
echo "${@}" | grep -q -- '--nice-juicy-steak' || exit 3
echo "${@}" | grep -q -- '--without-dormouse' || exit 4
true
END2
    chmod +x ${WORK}/configure
}
END
mkdir -p "packages/cat/expand-vars"
cat <<"END" > packages/cat/expand-vars/expand-vars-0.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="enabled-weasel broccoli linguas: enabled-en_GB"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

pkg_setup() {
    [[ ${OPTIONS%%+( )} == "enabled-weasel linguas:enabled-en_GB" ]] || die "OPTIONS=$OPTIONS is wrong"
    [[ ${LINGUAS%%+( )} == "enabled-en_GB" ]] || die "LINGUAS=$LINGUAS is wrong"
}
END
mkdir -p "packages/cat/doman-success"
cat <<'END' > packages/cat/doman-success/doman-success-0.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_compile() {
    echo ${PNVR} >foo.1
    mkdir dir
    echo ${PNVR} >dir/foo.2
    echo ${PNVR} >foo.3x
    echo ${PNVR} >foo.4.gz
    echo ${PNVR} >foo.5f.bz2
    echo ${PNVR} >foo.6.Z
    echo ${PNVR} >foo.en.7
    echo ${PNVR} >foo.en_GB.8
    echo ${PNVR} >foo.e.9
    echo ${PNVR} >foo.enn.n
    echo ${PNVR} >foo.EN.1
    echo ${PNVR} >foo.en-GB.2
    echo ${PNVR} >foo.en_gb.3
    echo ${PNVR} >foo.en_G.4
    echo ${PNVR} >foo.en_GBB.5
    echo ${PNVR} >foo.nonkey
    touch foo.1x
    echo ${PNVR} >baz.6
    echo ${PNVR} >baz.en_US.7
}

src_install() {
    doman foo.* dir/foo.* || die
    doman -i18n=en_GB baz.* || die
    keepdir /meh || die
    cd "${IMAGE}"/meh || die
    doman .keep* || die
    rm "${IMAGE}"/usr/share/man/{man1/foo.1,man2/foo.2,man3/foo.3x,man4/foo.4.gz,man5/foo.5f.bz2} || die
    rm "${IMAGE}"/usr/share/man/{man6/foo.6.Z,en/man7/foo.7,en_GB/man8/foo.8,man9/foo.e.9,mann/foo.enn.n} || die
    rm "${IMAGE}"/usr/share/man/{man1/foo.EN.1,man2/foo.en-GB.2,man3/foo.en_gb.3,man4/foo.en_G.4} || die
    rm "${IMAGE}"/usr/share/man/{man5/foo.en_GBB.5,mann/foo.nonkey,en_GB/man6/baz.6,en_US/man7/baz.7} || die
    rmdir "${IMAGE}"/usr/share/man/{man1,man2,man3,man4,man5,man6,man9,mann,en/man7,en_GB/man6,en_GB/man8,en_US/man7,en,en_GB,en_US,} || die
}
END
mkdir -p "packages/cat/doman-failure"
cat <<'END' > packages/cat/doman-failure/doman-failure-0.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_compile() {
    echo ${PNVR} >bar.m
}

src_install() {
    doman bar.m
}
END
mkdir -p "packages/cat/change-globals"
cat <<'END' > packages/cat/change-globals/change-globals-0.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_compile() {
    DEFAULT_SRC_COMPILE_PARAMS="foo"
    default
}
END
mkdir -p "packages/cat/install"
cat <<'END' > packages/cat/install/install-0.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_unpack() {
    touch -- -s
}

src_install() {
    install -v -- -s dest
    [[ -x dest ]] || die "install didn't work"
}
END
mkdir -p "packages/cat/install-s"
cat <<'END' > packages/cat/install-s/install-s-0.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_unpack() {
    touch src
}

src_install() {
    install -s src dest
    [[ -x dest ]] && die "install didn't fail"
}
END
mkdir -p "packages/cat/doman-nonfatal"
cat <<'END' > packages/cat/doman-nonfatal/doman-nonfatal-0.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_install() {
    nonfatal doman bar.1 && die
}
END
cd ..

mkdir -p repo15/{eclass,distfiles,profiles/profile/subprofile} || exit 1
cd repo15 || exit 1
echo "test-repo-15" >> profiles/repo_name || exit 1
cat <<END >profiles/profiles.desc || exit 1
test profile stable
test profile/subprofile stable
END
cat <<END > profiles/profile/make.defaults || exit 1
ARCH=test
END
cat <<END > profiles/profile/virtuals || exit 1
virtual/one	cat-one/pkg-one
virtual/two	cat-two/pkg-two
END
cat <<END >profiles/profile/subprofile/parent || exit 1
..
END
cat <<END > profiles/profile/subprofile/virtuals || exit 1
virtual/one	cat-two/pkg-two
virtual/two	cat-one/pkg-one
virtual/three	cat-three/pkg-three
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

mkdir -p repo17/{eclass,distfiles,profiles/profile} || exit 1
mkdir -p repo17/category/package || exit 1
cd repo17 || exit 1
echo "test-repo-17" >> profiles/repo_name || exit 1
echo "category" >> profiles/categories || exit 1
cat <<END > profiles/profile/make.defaults
ARCH=test
END
cat <<END > category/package/package-1.ebuild || exit 1
EAPI="exheres-0"
SLOT="0"
PLATFORMS="test"
DEPENDENCIES="cat/pkg1 build: cat/pkg2 build+run: cat/pkg3 suggestion: cat/pkg4 post: cat/pkg5"
END
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

mkdir -p repo20/{eclass,distfiles,profiles/profile} || exit 1
mkdir -p repo20/cat/pkg || exit 1
cd repo20 || exit 1
echo "test-repo-20" >> profiles/repo_name || exit 1
echo "cat" >> profiles/categories || exit 1
cat <<END > profiles/profile/make.defaults
ARCH=test
USERLAND="GNU"
KERNEL="linux"
LIBC="glibc"
CHOST="i286-badger-linux-gnu"
END
cat <<END > cat/pkg/pkg-1.ebuild || exit 1
SLOT="0"
PLATFORMS="test"
src_install() {
	ln -s "\${IMAGE}/foo" "\${IMAGE}/bar" || die
}
END
cd ..

cd ..

