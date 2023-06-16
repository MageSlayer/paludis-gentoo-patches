#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir package_id_TEST_dir || exit 1
cd package_id_TEST_dir || exit 1

mkdir -p home/.paludis/repositories

cat <<END > home/.paludis/repositories/testrepo.conf
location = `pwd`/testrepo
format = e
names_cache = /var/empty
cache = /var/empty
profiles = \${location}/profiles/testprofile
builddir = `pwd`
END

cat <<END > home/.paludis/repositories/installed.conf
location = `pwd`/installed
format = vdb
names_cache = /var/empty
cache = /var/empty
builddir = `pwd`
name = installed
END

cat <<END > home/.paludis/keywords.conf
*/* test
END
cat <<END > home/.paludis/licenses.conf
*/* *
END

cat <<END > home/.paludis/general.conf
world = /dev/null
END

mkdir -p testrepo/{eclass,distfiles,profiles/testprofile,foo/bar,cat/masked} || exit 1
cd testrepo || exit 1
echo "testrepo" > profiles/repo_name || exit 1
cat <<END > profiles/categories || exit 1
foo
cat
END
cat <<END > profiles/profiles.desc
test testprofile stable
END
cat <<END > profiles/testprofile/make.defaults
ARCH=test
KERNEL=test
END
cat <<END > profiles/testprofile/virtuals
virtual/bar foo/bar
END

cat <<"END" > foo/bar/bar-1.0.ebuild || exit 1
EAPI="8"
DESCRIPTION="Test package"
HOMEPAGE="http://paludis.exherbo.org/"
SRC_URI="http://example.com/${P}.tar.bz2"
SLOT="0"
IUSE="testflag"
LICENSE="GPL-2"
KEYWORDS="test"
RESTRICT="monkey"
DEPEND="foo/bar"
RDEPEND=""
IDEPEND=""
BDEPEND="foo/bar"
PROVIDE="virtual/monkey"
END


cat <<"END" > cat/masked/masked-1.0.ebuild || exit 1
DESCRIPTION="Masked package"
HOMEPAGE="http://paludis.exherbo.org/"
SRC_URI="http://example.com/${P}.tar.bz2"
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="masked"
RESTRICT=""
DEPEND=""
RDEPEND=""
END

cd ..

mkdir -p installed/cat-one/pkg-one-1 || exit 1


echo "cat-one" > installed/cat-one/pkg-one-1/CATEGORY
touch installed/cat-one/pkg-one-1/CONTENTS
echo "0" > installed/cat-one/pkg-one-1/EAPI
echo "a description" > installed/cat-one/pkg-one-1/DESCRIPTION
echo "test_inherited" > installed/cat-one/pkg-one-1/INHERITED
echo "test_iuse test" > installed/cat-one/pkg-one-1/IUSE
echo "test" > installed/cat-one/pkg-one-1/KEYWORDS
echo "origin_test" > installed/cat-one/pkg-one-1/REPOSITORY
echo "test_slot" > installed/cat-one/pkg-one-1/SLOT
echo "test test_use" > installed/cat-one/pkg-one-1/USE
