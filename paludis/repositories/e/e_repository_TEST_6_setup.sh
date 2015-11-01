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

mkdir -p "cat/econf-no-docdir-htmldir" || exit 1
cat << 'END' > cat/econf-no-docdir-htmldir/econf-no-docdir-htmldir-6.ebuild || exit 1
EAPI="6"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

S="${WORKDIR}"

src_unpack() {
    mkdir -p ${WORKDIR}
    cd "${WORKDIR}"

    cat <<'EOF' > configure
#!/bin/sh

if echo "$@" | grep -q 'help' ; then
    exit 0
fi

if echo "$@" | grep -q 'docdir' ; then
    exit 1
fi

if echo "$@" | grep -q 'htmldir' ; then
    exit 1
fi

exit 0
EOF

    chmod +x configure
}
END

mkdir -p "cat/econf-docdir-only" || exit 1
cat << 'END' > cat/econf-docdir-only/econf-docdir-only-6-r6.ebuild || exit 1
EAPI="6"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

S="${WORKDIR}"

src_unpack() {
    mkdir -p ${WORKDIR}
    cd "${WORKDIR}"

    cat <<'EOF' > configure
#!/bin/sh

if echo "$@" | grep -q 'help' ; then
    echo --docdir
    exit 0
fi

if ! echo "$@" | grep -q 'docdir=/usr/share/doc/econf-docdir-only-6-r6' ; then
    exit 1
fi

if echo "$@" | grep -q 'htmldir' ; then
    exit 1
fi

exit 0
EOF

    chmod +x configure
}
END

mkdir -p "cat/econf-htmldir-only" || exit 1
cat << 'END' > cat/econf-htmldir-only/econf-htmldir-only-6-r6.ebuild || exit 1
EAPI="6"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

S="${WORKDIR}"

src_unpack() {
    mkdir -p ${WORKDIR}
    cd "${WORKDIR}"

    cat <<'EOF' > configure
#!/bin/sh

if echo "$@" | grep -q 'help' ; then
    echo --htmldir
    exit 0
fi

if echo "$@" | grep -q 'docdir' ; then
    exit 1
fi

if ! echo "$@" | grep -q 'htmldir=/usr/share/doc/econf-htmldir-only-6-r6/html' ; then
    exit 1
fi

exit 0
EOF

    chmod +x configure
}
END

mkdir -p "cat/econf-docdir-htmldir" || exit 1
cat << 'END' > cat/econf-docdir-htmldir/econf-docdir-htmldir-6-r6.ebuild || exit 1
EAPI="6"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

S="${WORKDIR}"

src_unpack() {
    mkdir -p ${WORKDIR}
    cd "${WORKDIR}"

    cat <<'EOF' > configure
#!/bin/sh

if echo "$@" | grep -q 'help' ; then
    echo --docdir
    echo --htmldir
    exit 0
fi

if ! echo "$@" | grep -q 'docdir=/usr/share/doc/econf-docdir-htmldir-6-r6' ; then
    exit 1
fi

if ! echo "$@" | grep -q 'htmldir=/usr/share/doc/econf-docdir-htmldir-6-r6/html' ; then
    exit 1
fi

exit 0
EOF

    chmod +x configure
}
END

mkdir -p "cat/plain-die" || exit 1
cat << 'END' > cat/plain-die/plain-die-6.ebuild || exit 1
EAPI="6"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"

pkg_setup() {
    test
}

test() {
    die test
}
END

mkdir -p "cat/plain-assert" || exit 1
cat << 'END' > cat/plain-assert/plain-assert-6.ebuild || exit 1
EAPI="6"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"

pkg_setup() {
    test
}

test() {
    true | false | true
    assert test
}
END

mkdir -p "cat/nonfatal-die" || exit 1
cat << 'END' > cat/nonfatal-die/nonfatal-die-6.ebuild || exit 1
EAPI="6"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"

pkg_setup() {
    nonfatal test
}

test() {
    die test
}
END

mkdir -p "cat/nonfatal-assert" || exit 1
cat << 'END' > cat/nonfatal-assert/nonfatal-assert-6.ebuild || exit 1
EAPI="6"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"

pkg_setup() {
    nonfatal test
}

test() {
    true | false | true
    assert test
}
END

mkdir -p "cat/die-n" || exit 1
cat << 'END' > cat/die-n/die-n-6.ebuild || exit 1
EAPI="6"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"

pkg_setup() {
    test
}

test() {
    die -n test
}
END

mkdir -p "cat/assert-n" || exit 1
cat << 'END' > cat/assert-n/assert-n-6.ebuild || exit 1
EAPI="6"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"

pkg_setup() {
    test
}

test() {
    true | false | true
    assert -n test
}
END

mkdir -p "cat/nonfatal-die-n" || exit 1
cat << 'END' > cat/nonfatal-die-n/nonfatal-die-n-6.ebuild || exit 1
EAPI="6"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"

pkg_setup() {
    nonfatal test && die
}

test() {
    die -n test
}
END

mkdir -p "cat/nonfatal-assert-n" || exit 1
cat << 'END' > cat/nonfatal-assert-n/nonfatal-assert-n-6.ebuild || exit 1
EAPI="6"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"

pkg_setup() {
    nonfatal test && die
}

test() {
    true | false | true
    assert -n test
}
END

cd ..
cd ..
