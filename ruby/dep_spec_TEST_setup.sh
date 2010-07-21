#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir dep_spec_TEST_dir || exit 1
cd dep_spec_TEST_dir || exit 1

mkdir -p home/.paludis/repositories

cat <<END > home/.paludis/repositories/testrepo.conf
location = `pwd`/testrepo
format = e
names_cache = /var/empty
cache = /var/empty
profiles = \${location}/profiles/testprofile
builddir = `pwd`
END

cat <<END > home/.paludis/general.conf
world = /dev/null
END

mkdir -p testrepo/{eclass,distfiles,profiles/testprofile,foo/bar,bar/foo,bar/bar} || exit 1
cd testrepo || exit 1
echo "testrepo" > profiles/repo_name || exit 1
cat <<END > profiles/categories || exit 1
foo
bar
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
DEPEND="|| ( foo/bar foo/baz ) foo/monkey"
END

cat <<"END" > bar/foo/foo-1.0.ebuild || exit 1
EAPI="exheres-0"
DESCRIPTION="Test package"
HOMEPAGE="http://paludis.pioto.org/"
DOWNLOADS="mirrors-first: mirrors-only: listed-only: listed-first: local-only: manual: "
SLOT="0"
MYOPTIONS=""
LICENCES="GPL-2"
PLATFORMS="test"
DEPENDENCIES="build: one/two"
END

cat <<"END" > bar/bar/bar-1.0.ebuild || exit 1
EAPI="giant-space-monkey"
END

