#!/bin/bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir visibility_TEST_dir || exit 1
cd visibility_TEST_dir || exit 1

mkdir -p repo1/{eclass,distfiles,profiles/test,cat-one/{visible,masked,needs-masked}} || exit 1
cd repo1 || exit 1
echo "repo1" > profiles/repo_name || exit 1
cat <<END > profiles/categories || exit 1
cat-one
END
cat <<END > profiles/test/make.defaults
ARCH=test
ACCEPT_KEYWORDS=test
END
cat <<END > profiles/profiles.desc
test test/ stable
END
cat <<END > cat-one/visible/visible-1.ebuild
DESCRIPTION="visible"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"
END
cat <<END > cat-one/visible/visible-2.ebuild
DESCRIPTION="visible"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="~test"
END
cat <<END > cat-one/masked/masked-1.ebuild
DESCRIPTION="masked"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="monkey"
END
cat <<END > cat-one/needs-masked/needs-masked-1.ebuild
DESCRIPTION="needs masked"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"
DEPEND="cat-one/masked"
RDEPEND=""
END
cd ..

cd ..


