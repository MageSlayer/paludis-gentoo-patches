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

mkdir -p "cat/strict-use" || exit 1
cat << 'END' > cat/strict-use/strict-use-5.ebuild || exit 1
EAPI="5"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork enabled"
LICENSE="GPL-2"
KEYWORDS="test"

S="${WORKDIR}"

pkg_setup() {
    use enabled || die "enabled not enabled"
    use spork && die "sporks are bad"
}
END

mkdir -p "cat/strict-use-fail" || exit 1
cat << 'END' > cat/strict-use-fail/strict-use-fail-5.ebuild || exit 1
EAPI="5"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork enabled"
LICENSE="GPL-2"
KEYWORDS="test"

S="${WORKDIR}"

pkg_setup() {
    use pony
}
END

mkdir -p "cat/strict-use-injection" || exit 1
cat << 'END' > cat/strict-use-injection/strict-use-injection-5.ebuild || exit 1
EAPI="5"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork enabled"
LICENSE="GPL-2"
KEYWORDS="test"

S="${WORKDIR}"

pkg_setup() {
    use build && die "build set"
    use userland_GNU || die "userland_GNU not set"
    use cheese || die "cheese not set"
    use otherarch && die "otherarch set"
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
    usex !enabled                     >>"${T}"/usex.out || die
    usex !disabled                    >>"${T}"/usex.out || die
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
no
yes
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

mkdir -p "cat/doheader" || exit 1
cat << 'END' > cat/doheader/doheader-5.ebuild || exit 1
EAPI="5"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"

S="${WORKDIR}"

src_install() {
    echo foo >foo.h
    echo foo2 >foo2.h
    doheader foo.h foo2.h
    [[ $(<"${D}"/usr/include/foo.h)  == foo ]] || die
    [[ $(<"${D}"/usr/include/foo2.h) == foo2 ]] || die
    [[ -x ${D}/usr/include/foo.h  ]] && die
    [[ -x ${D}/usr/include/foo2.h ]] && die

    echo bar >bar.h
    newheader bar.h baz.h
    [[ -e ${D}/usr/include/bar.h ]] && die
    [[ $(<"${D}"/usr/include/baz.h) == bar ]] || die
    [[ -x ${D}/usr/include/baz.h ]] && die

    insopts -m0755

    echo xyzzy >xyzzy.h
    doheader xyzzy.h
    [[ $(<"${D}"/usr/include/xyzzy.h) == xyzzy ]] || die
    [[ -x ${D}/usr/include/xyzzy.h ]] || die

    echo plugh >plugh.h
    newheader plugh.h plover.h
    [[ -e ${D}/usr/include/plugh.h ]] && die
    [[ $(<"${D}"/usr/include/plover.h) == plugh ]] || die
    [[ -x ${D}/usr/include/plover.h ]] || die

    nonfatal doheader missing.h && die
}
END

mkdir -p "cat/doheader-dies" || exit 1
cat << 'END' > cat/doheader-dies/doheader-dies-5.ebuild || exit 1
EAPI="5"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"

S="${WORKDIR}"

src_install() {
    doheader missing.h
}
END

mkdir -p "cat/new-stdin" || exit 1
cat << 'END' > cat/new-stdin/new-stdin-5.ebuild || exit 1
EAPI="5"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"

S="${WORKDIR}"

src_install() {
    echo testbin    | newbin    - testbin
    echo testconfd  | newconfd  - testconfd
    echo testdoc    | newdoc    - testdoc
    echo testenvd   | newenvd   - testenvd
    exeinto /usr/libexec
    echo testexe    | newexe    - testexe
    echo testheader | newheader - testheader
    echo testinitd  | newinitd  - testinitd
    insinto /usr/share/test
    echo testins    | newins    - testins
    echo testlib.a  | newlib.a  - testlib.a
    echo testlib.so | newlib.so - testlib.so
    echo testman.1  | newman    - testman.1
    echo testsbin   | newsbin   - testsbin

    cat "${D}"/{usr/bin/testbin,etc/conf.d/testconfd,usr/share/doc/${PF}/testdoc,etc/env.d/testenvd,usr/libexec/testexe,usr/include/testheader,etc/init.d/testinitd,usr/share/test/testins,usr/lib*/testlib.a,usr/lib*/testlib.so,usr/share/man/man1/testman.1,usr/sbin/testsbin} > "${T}"/new.out
    cat >"${T}"/new.expected <<EOF
testbin
testconfd
testdoc
testenvd
testexe
testheader
testinitd
testins
testlib.a
testlib.so
testman.1
testsbin
EOF

    diff "${T}"/new.{expected,out} || die
}
END

mkdir -p "cat/ebuild-phase-func" || exit 1
cat << 'END' > cat/ebuild-phase-func/ebuild-phase-func-5.ebuild || exit 1
EAPI="5"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"

pkg_pretend() {
    [[ ${EBUILD_PHASE} == pretend ]] || die
    [[ ${EBUILD_PHASE_FUNC} == pkg_pretend ]] || die
}
END

mkdir -p "cat/apply-user-patches" || exit 1
cat << 'END' > cat/apply-user-patches/apply-user-patches-5.ebuild || exit 1
EAPI="5"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"

S="${WORKDIR}"

src_prepare() {
    apply_user_patches && die
}
END

mkdir -p "cat/apply-user-patches-default" || exit 1
cat << 'END' > cat/apply-user-patches-default/apply-user-patches-default-5.ebuild || exit 1
EAPI="5"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"

S="${WORKDIR}"

src_prepare() {
    default
}
END

mkdir -p "cat/apply-user-patches-uncalled" || exit 1
cat << 'END' > cat/apply-user-patches-uncalled/apply-user-patches-uncalled-5.ebuild || exit 1
EAPI="5"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"

S="${WORKDIR}"

src_prepare() {
    :
}
END

cd ..
cd ..
