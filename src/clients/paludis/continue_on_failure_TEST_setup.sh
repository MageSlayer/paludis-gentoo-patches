#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir continue_on_failure_TEST_dir || exit 1
cd continue_on_failure_TEST_dir || exit 1
mkdir -p build

mkdir -p config/.paludis-continue-on-failure-test
cat <<END > config/.paludis-continue-on-failure-test/specpath.conf
root = `pwd`/root
config-suffix =
END

mkdir -p root/${SYSCONFDIR}/paludis/repositories
cat <<END > root/${SYSCONFDIR}/paludis/use.conf
*/* foo
END

cat <<END > root/${SYSCONFDIR}/paludis/licenses.conf
*/* *
END

cat <<END > root/${SYSCONFDIR}/paludis/keywords.conf
*/* test
END

cat <<END > root/${SYSCONFDIR}/paludis/general.conf
world = `pwd`/root/world
END

cat <<END > root/${SYSCONFDIR}/paludis/bashrc
export CHOST="my-chost"
END

cat <<END > root/${SYSCONFDIR}/paludis/repositories/repo1.conf
location = `pwd`/repo1
cache = /var/empty
format = e
names_cache = /var/empty
profiles = \${location}/profiles/testprofile
builddir = `pwd`/build
END

cat <<END > root/${SYSCONFDIR}/paludis/repositories/installed.conf
location = `pwd`/root/var/db/pkg
format = vdb
names_cache = /var/empty
provides_cache = /var/empty
builddir = `pwd`/build
END

mkdir -p root/tmp
mkdir -p root/var/db/pkg
touch root/${SYSCONFDIR}/ld.so.conf

mkdir -p repo1/{eclass,distfiles,profiles/testprofile,cat/{a,b,c,d,e,u,v,w,x,y,z}/files} || exit 1

cd repo1 || exit 1
echo "test-repo-1" > profiles/repo_name || exit 1
cat <<END > profiles/categories || exit 1
cat
END
cat <<END > profiles/testprofile/make.defaults
ARCH=test
USERLAND=test
KERNEL=test
TESTPROFILE_WAS_SOURCED=yes
PROFILE_ORDERING=1
USE_EXPAND="USERLAND KERNEL"
END

cat <<"END" > cat/a/a-1.ebuild || exit 1
DESCRIPTION="Test a"
HOMEPAGE="http://paludis.pioto.org/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"
RDEPEND="cat/d"

src_install() {
    touch ${D}/a
}
END

cat <<"END" > cat/b/b-1.ebuild || exit 1
DESCRIPTION="Test b"
HOMEPAGE="http://paludis.pioto.org/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"
RDEPEND="cat/a"

src_install() {
    touch ${D}/b
}
END

cat <<"END" > cat/c/c-1.ebuild || exit 1
DESCRIPTION="Test c"
HOMEPAGE="http://paludis.pioto.org/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"
RDEPEND="cat/e"

src_install() {
    touch ${D}/c
}
END

cat <<"END" > cat/d/d-1.ebuild || exit 1
DESCRIPTION="Test d"
HOMEPAGE="http://paludis.pioto.org/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"
RDEPEND=""

pkg_setup() {
    die "supposed to fail"
}
END

cat <<"END" > cat/e/e-1.ebuild || exit 1
DESCRIPTION="Test e"
HOMEPAGE="http://paludis.pioto.org/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"
RDEPEND=""

src_install() {
    touch ${D}/e
}
END

cat <<"END" > cat/z/z-1.ebuild || exit 1
DESCRIPTION="Test z"
HOMEPAGE="http://paludis.pioto.org/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"
RDEPEND="cat/w"

src_install() {
    touch ${D}/z
}
END

cat <<"END" > cat/y/y-1.ebuild || exit 1
DESCRIPTION="Test y"
HOMEPAGE="http://paludis.pioto.org/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"
RDEPEND="cat/u"

src_install() {
    touch ${D}/y
}
END

cat <<"END" > cat/x/x-1.ebuild || exit 1
DESCRIPTION="Test x"
HOMEPAGE="http://paludis.pioto.org/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"
RDEPEND="cat/v"

src_install() {
    touch ${D}/x
}
END

cat <<"END" > cat/w/w-1.ebuild || exit 1
DESCRIPTION="Test w"
HOMEPAGE="http://paludis.pioto.org/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"
RDEPEND=""
END

cat <<"END" > cat/w/w-2.ebuild || exit 1
DESCRIPTION="Test w"
HOMEPAGE="http://paludis.pioto.org/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"
RDEPEND=""

pkg_setup() {
    die "supposed to fail"
}
END

cat <<"END" > cat/v/v-1.ebuild || exit 1
DESCRIPTION="Test v"
HOMEPAGE="http://paludis.pioto.org/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"
RDEPEND=""

src_install() {
    touch ${D}/v
}
END

cat <<"END" > cat/u/u-1.ebuild || exit 1
DESCRIPTION="Test u"
HOMEPAGE="http://paludis.pioto.org/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"
RDEPEND="cat/w"

src_install() {
    touch ${D}/u
}
END

cd ..

