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
    echo --disable-silent-rules
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

mkdir -p "cat/econf-no-docdir-htmldir" || exit 1
cat << 'END' > cat/econf-no-docdir-htmldir/econf-no-docdir-htmldir-5.ebuild || exit 1
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
    echo --docdir
    echo --htmldir
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

mkdir -p "cat/global-no-failglob" || exit 1
cat << 'END' > cat/global-no-failglob/global-no-failglob-5.ebuild || exit 1
EAPI="5"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"

fail=( does/not/exist/* )
END

mkdir -p "cat/unpack-bare" || exit 1
cat << 'END' > cat/unpack-bare/unpack-bare-5.ebuild || exit 1
EAPI="5"
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
cat << 'END' > cat/unpack-dotslash/unpack-dotslash-5.ebuild || exit 1
EAPI="5"
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
cat << 'END' > cat/unpack-absolute/unpack-absolute-5.ebuild || exit 1
EAPI="5"
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
    [[ -e test3.gz ]] || return
    unpack $(pwd)/test3.gz
}
END

mkdir -p "cat/unpack-relative" || exit 1
cat << 'END' > cat/unpack-relative/unpack-relative-5.ebuild || exit 1
EAPI="5"
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
    [[ -e subdir/test4.gz ]] || return
    unpack subdir/test4.gz
}
END

mkdir -p "cat/unpack-case-sensitive" || exit 1
cat << 'END' > cat/unpack-case-sensitive/unpack-case-sensitive-5.ebuild || exit 1
EAPI="5"
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
    unpack ./test5.TAR.z
    [[ -e test5 ]] && die
}
END

mkdir -p "cat/nonfatal-die" || exit 1
cat << 'END' > cat/nonfatal-die/nonfatal-die-5.ebuild || exit 1
EAPI="5"
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
    die -n test
}
END

mkdir -p "cat/nonfatal-assert" || exit 1
cat << 'END' > cat/nonfatal-assert/nonfatal-assert-5.ebuild || exit 1
EAPI="5"
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
    assert -n test
}
END

mkdir -p "cat/no-get_libdir" || exit 1
cat << 'END' > cat/no-get_libdir/no-get_libdir-5.ebuild || exit 1
EAPI="5"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"

S=${WORKDIR}

src_configure() {
    [[ -z $(declare -F get_libdir) ]] || die
}
END

mkdir -p "cat/has-einstall" || exit 1
cat << 'END' > cat/has-einstall/has-einstall-5.ebuild || exit 1
EAPI="5"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"

S=${WORKDIR}

src_unpack() {
    echo 'install: ; true' >Makefile
}

src_install() {
    einstall
}
END

mkdir -p "cat/no-in_iuse" || exit 1
cat << 'END' > cat/no-in_iuse/no-in_iuse-5.ebuild || exit 1
EAPI="5"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"

pkg_setup() {
    [[ -z $(declare -F in_iuse) ]] || die
}
END

mkdir -p "cat/no-einstalldocs" || exit 1
cat << 'END' > cat/no-einstalldocs/no-einstalldocs-5.ebuild || exit 1
EAPI="5"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"

S=${WORKDIR}

src_install() {
    [[ -z $(declare -F einstalldocs) ]] || die
}
END

mkdir -p "cat/default_src_install" || exit 1
cat << 'END' > cat/default_src_install/default_src_install-5.ebuild || exit 1
EAPI="5"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"

S=${WORKDIR}

src_unpack() {
    echo testing.txt >testing.txt || die
    echo testing.html >testing.html || die
    cat <<'EOF' >Makefile || die
all: ; echo monkey
install: ; echo monkey >$(DESTDIR)/monkey
EOF
}

DOCS="testing.txt"
HTML_DOCS="testing.html"

pkg_preinst() {
    [[ -f ${D}/monkey ]] || die monkey
    d=${D}/usr/share/doc/${PF}
    [[ -d ${d} ]] || die ${d}
    [[ -f ${d}/testing.txt ]] || die testing.txt
    [[ -d ${d}/html ]] && die html
    [[ -f ${d}/html/testing.html ]] && die html/testing.html
}
END

mkdir -p "cat/no-eapply" || exit 1
cat << 'END' > cat/no-eapply/no-eapply-5.ebuild || exit 1
EAPI="5"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"

S=${WORKDIR}

src_prepare() {
    [[ -z $(declare -F eapply) ]] || die
}
END

mkdir -p "cat/no-eapply_user" || exit 1
cat << 'END' > cat/no-eapply_user/no-eapply_user-5.ebuild || exit 1
EAPI="5"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"

S=${WORKDIR}

src_prepare() {
    [[ -z $(declare -F eapply_user) ]] || die
}
END

mkdir -p "cat/default_src_prepare/files" || exit 1
cat << 'END' > cat/default_src_prepare/default_src_prepare-5.ebuild || exit 1
EAPI="5"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"

S=${WORKDIR}

src_unpack() {
    echo first > file || die
}

PATCHES="${FILESDIR}/first.patch"

src_configure() {
    [[ $(< file) == first ]] || die file
}
END
cat << 'END' > cat/default_src_prepare/files/first.patch || exit 1
--- directory/file
+++ directory/file
@@ -1 +1 @@
-first
+second
END

mkdir -p "cat/bash-compat" || exit 1
cat << 'END' > cat/bash-compat/bash-compat-5.ebuild || exit 1
EAPI="5"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"

pkg_setup() {
    [[ ${BASH_COMPAT} == 3.2 ]] || [[ ${BASH_COMPAT} == 32 ]] || die BASH_COMPAT=${BASH_COMPAT}
}
END

mkdir -p "cat/subslots" || exit 1
cat << 'END' > cat/subslots/subslots-5.ebuild || exit 1
EAPI="5"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="foo/bar"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"

S="${WORKDIR}"
END

mkdir -p "cat/subslot-dep" || exit 1
cat << 'END' > cat/subslot-dep/subslot-dep-5.ebuild || exit 1
EAPI="5"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"
DEPEND="cat/subslots:= cat/subslots:foo="

S="${WORKDIR}"
END

cd ..
cd ..
