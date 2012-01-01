#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir choices_TEST_dir || exit 1
cd choices_TEST_dir || exit 1

mkdir -p home/.paludis/repositories

cat <<END > home/.paludis/repositories/testrepo.conf
location = `pwd`/testrepo
format = e
names_cache = /var/empty
cache = /var/empty
profiles = \${location}/profiles/testprofile
builddir = `pwd`
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

mkdir -p testrepo/{eclass,distfiles,profiles/{testprofile,desc},foo/bar/files} || exit 1
cd testrepo || exit 1
echo "testrepo" > profiles/repo_name || exit 1
cat <<END > profiles/categories || exit 1
foo
END
cat <<END > profiles/profiles.desc
test testprofile stable
END
cat <<END > profiles/testprofile/make.defaults
ARCH=test
USERLAND=test
KERNEL=test
USE_EXPAND=LINGUAS
END

cat <<"END" > foo/bar/bar-1.0.ebuild || exit 1
DESCRIPTION="Test package"
HOMEPAGE="http://paludis.exherbo.org/"
SRC_URI="http://example.com/${P}.tar.bz2"
SLOT="0"
IUSE="testflag linguas_en"
LICENSE="GPL-2"
KEYWORDS="test"
RESTRICT="monkey"
DEPEND="foo/bar"
RDEPEND=""
END

cat <<END > profiles/use.desc
testflag - the test flag
END

cat <<END > profiles/desc/linguas.desc
en - English
END

cd ..

