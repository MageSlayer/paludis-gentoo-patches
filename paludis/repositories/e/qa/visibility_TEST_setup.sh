#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir visibility_TEST_dir || exit 1
cd visibility_TEST_dir || exit 1

mkdir -p repo1/{eclass,distfiles,profiles/test{,64},cat-one/{visible,masked,needs-masked,use-masking}} || exit 1
cd repo1 || exit 1
echo "repo1" > profiles/repo_name || exit 1
cat <<END > profiles/arch.list || exit 1
test
test64
END
cat <<END > profiles/categories || exit 1
cat-one
END
cat <<END > profiles/test/make.defaults
ARCH=test
ACCEPT_KEYWORDS=test
END
cat <<END > profiles/test64/make.defaults
ARCH=test64
ACCEPT_KEYWORDS=test64
END
echo masked > profiles/test64/use.mask
echo forced > profiles/test64/use.force
cat <<END > profiles/profiles.desc
test test/ stable
test64 test64/ stable
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
cat <<END > cat-one/use-masking/use-masking-1.ebuild
DESCRIPTION="use masking"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENCE="GPL-2"
KEYWORDS="test test64"
DEPEND="foo? ( cat-one/visible )"
RDEPEND=""
END
cat <<END > cat-one/use-masking/use-masking-2.ebuild
DESCRIPTION="use masking"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENCE="GPL-2"
KEYWORDS="test test64"
DEPEND="!foo? ( cat-one/visible )"
RDEPEND=""
END
cat <<END > cat-one/use-masking/use-masking-3.ebuild
DESCRIPTION="use masking"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENCE="GPL-2"
KEYWORDS="test test64"
DEPEND="masked? ( cat-one/visible )"
RDEPEND=""
END
cat <<END > cat-one/use-masking/use-masking-4.ebuild
DESCRIPTION="use masking"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENCE="GPL-2"
KEYWORDS="test test64"
DEPEND="!forced? ( cat-one/visible )"
RDEPEND=""
END
cat <<END > cat-one/use-masking/use-masking-5.ebuild
DESCRIPTION="use masking"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENCE="GPL-2"
KEYWORDS="test test64"
DEPEND="test? ( cat-one/visible )"
RDEPEND=""
END
cat <<END > cat-one/use-masking/use-masking-6.ebuild
DESCRIPTION="use masking"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENCE="GPL-2"
KEYWORDS="test test64"
DEPEND="!test64? ( cat-one/visible )"
RDEPEND=""
END
cd ..

cd ..


