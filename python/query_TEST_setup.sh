#!/bin/bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir query_TEST_dir || exit 1
cd query_TEST_dir || exit 1

mkdir -p testrepo/{eclass,distfiles,profiles/testprofile,cat/package} || exit 1
cd testrepo || exit 1
echo "testrepo" > profiles/repo_name || exit 1
cat <<END > profiles/categories || exit 1
cat
END
cat <<END > profiles/profiles.desc
test testprofile stable
END
cat <<END > profiles/testprofile/make.defaults
ARCH=test
USERLAND=test
KERNEL=test
END

cat <<"END" > cat/package/package-1.0.ebuild || exit 1
DESCRIPTION="Query for me"
HOMEPAGE="http://paludis.pioto.org/"
SRC_URI="http://example.com/${P}.tar.bz2"
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test
RESTRICT=""
DEPEND=""
RDEPEND=""
END

