#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir e_repository_TEST_6_dir || exit 1
cd e_repository_TEST_6_dir || exit 1

mkdir -p root/etc

mkdir -p vdb
touch vdb/THISISTHEVDB

mkdir -p build
ln -s build symlinked_build

mkdir -p distdir
gzip -c <<<test >distdir/test.gz

mkdir -p repo/{profiles/profile,metadata,eclass} || exit 1
cd repo || exit 1
echo "test-repo" >> profiles/repo_name || exit 1
echo "cat" >> profiles/categories || exit 1
cat <<END > profiles/profile/make.defaults
ARCH="cheese"
USERLAND="GNU"
KERNEL="linux"
LIBC="glibc"
CHOST="i286-badger-linux-gnu"
LINGUAS="enabled_en enabled_en_GB enabled_en_GB@UTF-8"
USE_EXPAND="LINGUAS USERLAND"
USE_EXPAND_UNPREFIXED="ARCH"
USE_EXPAND_IMPLICIT="USERLAND ARCH"
USE_EXPAND_VALUES_USERLAND="GNU"
USE_EXPAND_VALUES_ARCH="cheese otherarch"
IUSE_IMPLICIT="build"
END

mkdir -p "cat/global-failglob" || exit 1
cat << 'END' > cat/global-failglob/global-failglob-6.ebuild || exit 1
EAPI="6"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"

fail=( does/not/exist/* )
END

mkdir -p "cat/nonglobal-no-failglob" || exit 1
cat << 'END' > cat/nonglobal-no-failglob/nonglobal-no-failglob-6.ebuild || exit 1
EAPI="6"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"

pkg_setup() {
    fail=( does/not/exist/* )
}
END

mkdir -p "cat/unpack-bare" || exit 1
cat << 'END' > cat/unpack-bare/unpack-bare-6.ebuild || exit 1
EAPI="6"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI="test.gz"
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"

S=${WORKDIR}

src_unpack() {
    unpack test.gz
    [[ $(< test) == test ]] || die
}
END

mkdir -p "cat/unpack-dotslash" || exit 1
cat << 'END' > cat/unpack-dotslash/unpack-dotslash-6.ebuild || exit 1
EAPI="6"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"

S=${WORKDIR}

src_unpack() {
    gzip -c <<<test2 >test2.gz
    unpack ./test2.gz
    [[ $(< test2) == test2 ]] || die
}
END

mkdir -p "cat/unpack-absolute" || exit 1
cat << 'END' > cat/unpack-absolute/unpack-absolute-6.ebuild || exit 1
EAPI="6"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"

S=${WORKDIR}

src_unpack() {
    gzip -c <<<test3 >test3.gz
    unpack $(pwd)/test3.gz
    [[ $(< test3) == test3 ]] || die
}
END

mkdir -p "cat/unpack-relative" || exit 1
cat << 'END' > cat/unpack-relative/unpack-relative-6.ebuild || exit 1
EAPI="6"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"

S=${WORKDIR}

src_unpack() {
    mkdir subdir
    gzip -c <<<test4 >subdir/test4.gz
    unpack subdir/test4.gz
    [[ $(< test4) == test4 ]] || die
}
END

mkdir -p "cat/unpack-case-insensitive" || exit 1
cat << 'END' > cat/unpack-case-insensitive/unpack-case-insensitive-6.ebuild || exit 1
EAPI="6"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"

S=${WORKDIR}

src_unpack() {
    echo test5 >test5
    tar czf test5.TAR.z test5 || die
    rm test5
    [[ -e test5 ]] && die
    unpack ./test5.TAR.z
    [[ $(< test5) == test5 ]] || die
}
END

cd ..
cd ..
