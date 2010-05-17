#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir paludis_ruby_TEST_dir || exit 1
cd paludis_ruby_TEST_dir || exit 1

mkdir -p home/.paludis/repositories

cat <<END > home/.paludis/repositories/testrepo.conf
location = `pwd`/testrepo
format = ebuild
names_cache = /var/empty
cache = /var/empty
profiles = \${location}/profiles/testprofile
builddir = `pwd`
END

cat <<END > home/.paludis/repositories/installed.conf
location = `pwd`/installed
format = vdb
names_cache = /var/empty
provides_cache = /var/empty
builddir = `pwd`
END

cat <<END > home/.paludis/keywords.conf
*/* test
~foo/bar-1 ~test
END

cat <<END > home/.paludis/use.conf
*/* enabled
~foo/bar-1 sometimes_enabled
END

cat <<END > home/.paludis/licenses.conf
*/* *
END

cat <<END > home/.paludis/general.conf
world = `pwd`/world
END

mkdir -p testrepo/{eclass,distfiles,profiles/testprofile,foo/bar/files} || exit 1
cd testrepo || exit 1
echo "testrepo" > profiles/repo_name || exit 1
cat <<END > profiles/categories || exit 1
foo
END
cat <<END > profiles/testprofile/make.defaults
ARCH=test
USERLAND=test
KERNEL=test
END
cat <<END > profiles/profiles.desc
test testprofile stable
END

cat <<"END" > foo/bar/bar-1.0.ebuild || exit 1
DESCRIPTION="Test package"
HOMEPAGE="http://paludis.pioto.org/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"
END

cat <<"END" > foo/bar/bar-2.0.ebuild || exit 1
DESCRIPTION="Test package"
HOMEPAGE="http://paludis.pioto.org/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="~test"
END
cd ..

mkdir -p installed || exit 1

echo "foo/bar" > world


