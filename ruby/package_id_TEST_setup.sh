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

cat <<END > home/.paludis/repositories/exheresrepo.conf
location = `pwd`/exheresrepo
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
*/* test ~test
END
cat <<END > home/.paludis/licenses.conf
*/* *
END

cat <<END > home/.paludis/general.conf
world = /dev/null
END

mkdir -p testrepo/{eclass,distfiles,profiles/testprofile,foo/bar/files,bad/pkg/files} || exit 1
cd testrepo || exit 1
echo "testrepo" > profiles/repo_name || exit 1
cat <<END > profiles/categories || exit 1
foo
bad
END
cat <<END > profiles/profiles.desc
test testprofile stable
END
cat <<END > profiles/testprofile/make.defaults
ARCH=test
USERLAND=test
KERNEL=test
END
cat <<END > profiles/package.mask
# this is
# a test
foo/bar
END

cat <<"END" > foo/bar/bar-1.0.ebuild || exit 1
EAPI="8"
DESCRIPTION="Test package"
HOMEPAGE="http://paludis.exherbo.org/"
SRC_URI="http://example.com/${P}.tar.bz2"
SLOT="0"
IUSE="testflag"
LICENSE="GPL-2"
KEYWORDS="~test"
RESTRICT="monkey"
DEPEND="foo/bar"
BDEPEND="foo/bar"
RDEPEND="foo/bar"
IDEPEND="foo/bar"
PROVIDE="virtual/monkey"
END

cat <<"END" > bad/pkg/pkg-1.0.ebuild || exit 1
EAPI="7"
DESCRIPTION="Test package"
HOMEPAGE="http://paludis.exherbo.org/"
SRC_URI="http://example.com/${P}.tar.bz2"
SLOT="0"
IUSE="testflag!!!"
LICENSE="GPL-2"
KEYWORDS="test!!!"
RESTRICT="monkey"
DEPEND="||(foo/bar bar/foo)"
RDEPEND=""
BDEPEND="||(foo/bar bar/foo)"
END

cd ..

mkdir -p exheresrepo/{exlibs,metadata,profiles/testprofile,packages/scm/scm/files} || exit 1
cd exheresrepo || exit 1
echo "exheresrepo" > profiles/repo_name || exit 1
cat <<END > metadata/categories.conf || exit 1
scm
END
cat <<END > metadata/layout.conf
layout = exheres
eapi_when_unknown = exheres-0
eapi_when_unspecified = exheres-0
profile_eapi_when_unspecified = exheres-0
END
cat <<END > metadata/profiles_desc.conf
test testprofile stable
END
cat <<END > metadata/repository_mask.conf
scm/scm[=scm] [[ token = scm ]]
END

cat <<"END" > packages/scm/scm/scm-scm.exheres-0 || exit 1
SUMMARY="Test package"
HOMEPAGE="http://paludis.exherbo.org/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="testflag"
LICENCES="GPL-2"
PLATFORMS="test"
DEPENDENCIES="build: foo/bar"
END

cd ..

mkdir -p installed/cat-one/pkg-{one,two}-1 || exit 1

echo "cat-one" > installed/cat-one/pkg-one-1/CATEGORY
touch installed/cat-one/pkg-one-1/CONTENTS
echo "0" > installed/cat-one/pkg-one-1/EAPI
echo "a description" > installed/cat-one/pkg-one-1/DESCRIPTION
echo "test_iuse test" > installed/cat-one/pkg-one-1/IUSE
echo "test" > installed/cat-one/pkg-one-1/KEYWORDS
echo "origin_test" > installed/cat-one/pkg-one-1/REPOSITORY
echo "test_slot" > installed/cat-one/pkg-one-1/SLOT
echo "test test_use" > installed/cat-one/pkg-one-1/USE
