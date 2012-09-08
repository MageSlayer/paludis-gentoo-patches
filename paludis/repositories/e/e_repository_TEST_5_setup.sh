#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir e_repository_TEST_5_dir || exit 1
cd e_repository_TEST_5_dir || exit 1

mkdir -p root/etc

mkdir -p vdb
touch vdb/THISISTHEVDB

mkdir -p build
ln -s build symlinked_build

mkdir -p distdir

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

mkdir -p "cat/required-use-at-most-one-none" || exit 1
cat << 'END' > cat/required-use-at-most-one-none/required-use-at-most-one-none-5.ebuild || exit 1
EAPI="5"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
LICENSE="GPL-2"
KEYWORDS="test"
IUSE="disabled1 disabled2 disabled3 enabled1 enabled2 enabled3"
REQUIRED_USE="?? ( disabled1 disabled2 disabled3 )"
S="${WORKDIR}"
END

mkdir -p "cat/required-use-at-most-one-one" || exit 1
cat << 'END' > cat/required-use-at-most-one-one/required-use-at-most-one-one-5.ebuild || exit 1
EAPI="5"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
LICENSE="GPL-2"
KEYWORDS="test"
IUSE="disabled1 disabled2 disabled3 enabled1 enabled2 enabled3"
REQUIRED_USE="?? ( disabled1 enabled2 disabled3 )"
S="${WORKDIR}"
END

mkdir -p "cat/required-use-at-most-one-two" || exit 1
cat << 'END' > cat/required-use-at-most-one-two/required-use-at-most-one-two-5.ebuild || exit 1
EAPI="5"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
LICENSE="GPL-2"
KEYWORDS="test"
IUSE="disabled1 disabled2 disabled3 enabled1 enabled2 enabled3"
REQUIRED_USE="?? ( disabled1 enabled2 enabled3 )"
S="${WORKDIR}"
END

mkdir -p "cat/econf-disable-silent-rules" || exit 1
cat << 'END' > cat/econf-disable-silent-rules/econf-disable-silent-rules-5.ebuild || exit 1
EAPI="5"
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
    echo disable-silent-rules
    exit 0
fi

if ! echo "$@" | grep -q 'disable-silent-rules' ; then
    exit 1
fi

exit 0
EOF

    chmod +x configure
}
END

mkdir -p "cat/usex" || exit 1
cat << 'END' > cat/usex/usex-5.ebuild || exit 1
EAPI="5"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="enabled disabled"
LICENSE="GPL-2"
KEYWORDS="test"

S="${WORKDIR}"

pkg_pretend() {
    usex enabled                      >>"${T}"/usex.out || die
    usex disabled                     >>"${T}"/usex.out || die
    usex enabled  foo                 >>"${T}"/usex.out || die
    usex disabled foo                 >>"${T}"/usex.out || die
    usex enabled  foo bar             >>"${T}"/usex.out || die
    usex disabled foo bar             >>"${T}"/usex.out || die
    usex enabled  foo bar xyzzy       >>"${T}"/usex.out || die
    usex disabled foo bar xyzzy       >>"${T}"/usex.out || die
    usex enabled  foo bar xyzzy plugh >>"${T}"/usex.out || die
    usex disabled foo bar xyzzy plugh >>"${T}"/usex.out || die
    usex enabled  ""  bar xyzzy plugh >>"${T}"/usex.out || die
    usex disabled ""  bar xyzzy plugh >>"${T}"/usex.out || die
    usex enabled  foo ""  xyzzy plugh >>"${T}"/usex.out || die
    usex disabled foo ""  xyzzy plugh >>"${T}"/usex.out || die
    usex enabled  foo bar ""    plugh >>"${T}"/usex.out || die
    usex disabled foo bar ""    plugh >>"${T}"/usex.out || die
    usex enabled  foo bar xyzzy ""    >>"${T}"/usex.out || die
    usex disabled foo bar xyzzy ""    >>"${T}"/usex.out || die

    cat >"${T}"/usex.expected <<EOF
yes
no
foo
no
foo
bar
fooxyzzy
bar
fooxyzzy
barplugh
xyzzy
barplugh
fooxyzzy
plugh
foo
barplugh
fooxyzzy
bar
EOF

    diff "${T}"/usex.{expected,out} || die
}
END

cd ..
cd ..
