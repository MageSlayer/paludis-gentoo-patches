#!/bin/bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir e_repository_TEST_dir || exit 1
cd e_repository_TEST_dir || exit 1

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


mkdir -p repo6/{eclass,distfiles,profiles/profile} || exit 1
mkdir -p repo6/cat-one/pkg-one || exit 1
mkdir -p repo6/metadata/cache/cat-one
cd repo6 || exit 1
echo "test-repo-6" > profiles/repo_name || exit 1
cat <<END > profiles/categories || exit 1
cat-one
END
cat <<END > profiles/profile/make.defaults
ARCH=test
END
cat <<END > cat-one/pkg-one/pkg-one-1.ebuild || exit 1
END
cat <<END > metadata/cache/cat-one/pkg-one-1
the/depend
the/rdepend
the-slot
the-src-uri
the-restrict
the-homepage
the-license
the-description
the-keywords

the-iuse
unused
the/pdepend
the/provide
0
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


mkdir -p repo9/{eclass,distfiles,profiles/profile,cat-one/pkg-one,cat-two/pkg-one} || exit 1
mkdir -p repo9/{cat-one/pkg-one,cat-two/pkg-two} || exit 1
cd repo9 || exit 1
echo "test-repo-9" > profiles/repo_name || exit 1
cat <<END >profiles/categories || exit 1
cat-one
cat-two
END
cat <<END >profiles/profile/make.defaults || exit 1
ARCH=test
USE="flag1 flag2 flag3 -flag4 -flag5"
END
cat <<END >profiles/profile/use.mask || exit 1
flag2
END
cat <<END >profiles/profile/use.force || exit 1
flag4
END
cat <<END >profiles/profile/package.use.mask || exit 1
cat-two/pkg-two flag3
>=cat-one/pkg-one-2 flag3
END
cat <<END >profiles/profile/package.use.force || exit 1
cat-two/pkg-two flag5
END
cat <<END > cat-one/pkg-one/pkg-one-1.ebuild || exit 1
END
cat <<END > cat-one/pkg-one/pkg-one-2.ebuild || exit 1
END
cat <<END > cat-two/pkg-two/pkg-two-1.ebuild || exit 1
END
cd ..

mkdir -p repo10/{eclass,distfiles,profiles/profile/subprofile,cat/masked,cat/not_masked,cat/was_masked} || exit 1
cd repo10 || exit 1
echo "test-repo-10" > profiles/repo_name || exit 1
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
END
cat <<END > cat/not_masked/not_masked-0.ebuild
END
cat <<END > cat/was_masked/was_masked-0.ebuild
END
cd ..

