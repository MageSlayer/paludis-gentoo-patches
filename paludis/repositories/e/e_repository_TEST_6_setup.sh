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
KERNEL="linux"
LIBC="glibc"
CHOST="i286-badger-linux-gnu"
LINGUAS="enabled_en enabled_en_GB enabled_en_GB@UTF-8"
USE_EXPAND="LINGUAS"
USE_EXPAND_UNPREFIXED="ARCH"
USE_EXPAND_IMPLICIT="ARCH"
USE_EXPAND_VALUES_ARCH="cheese otherarch"
IUSE_IMPLICIT="build"
END
cat<<END > eclass/test.eclass
IUSE="eclass-flag"
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

mkdir -p "cat/get_libdir" || exit 1
cat << 'END' > cat/get_libdir/get_libdir-6.ebuild || exit 1
EAPI="6"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"

S=${WORKDIR}

src_configure() {
    [[ -n $(declare -F get_libdir) ]] || die

    local vars="CONF_LIBDIR_OVERRIDE ABI DEFAULT_ABI ${!LIBDIR_*}"
    local ${vars}
    unset ${vars}

    [[ $(get_libdir) == lib ]] || die
    [[ $(ABI= LIBDIR_=libv get_libdir) == lib ]] || die
    [[ $(ABI=vax get_libdir) == lib ]] || die
    [[ $(ABI=vax LIBDIR_vax= get_libdir) == lib ]] || die
    [[ $(ABI=vax LIBDIR_vax=libv get_libdir) == libv ]] || die
}
END

mkdir -p "cat/no-einstall" || exit 1
cat << 'END' > cat/no-einstall/no-einstall-6.ebuild || exit 1
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
    echo 'install: ; true' >Makefile
}

src_install() {
    einstall
}
END

mkdir -p "cat/in_iuse" || exit 1
cat << 'END' > cat/in_iuse/in_iuse-6.ebuild || exit 1
EAPI="6"

inherit test

DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="+ebuild-flag"
LICENSE="GPL-2"
KEYWORDS="test"

pkg_setup() {
    [[ -n $(declare -F in_iuse) ]] || die not defined

    in_iuse ebuild-flag || die ebuild-flag
    in_iuse eclass-flag || die eclass-flag
    in_iuse build || die build
    in_iuse cheese || die cheese
    in_iuse otherarch || die otherarch
    in_iuse userland_GNU || die userland_GNU
    in_iuse userland_zOS && die userland_zOS
    in_iuse nowhere-flag && die nowhere-flag
}
END

mkdir -p "cat/in_iuse-global" || exit 1
cat << 'END' > cat/in_iuse-global/in_iuse-global-6.ebuild || exit 1
EAPI="6"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"

in_iuse test && DEPEND="test? ( cat/test-dep )"
END

mkdir -p "cat/in_iuse-global-notmetadata" || exit 1
cat << 'END' > cat/in_iuse-global-notmetadata/in_iuse-global-notmetadata-6.ebuild || exit 1
EAPI="6"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"

if [[ -n ${MERGE_TYPE} ]] && in_iuse test ; then
    DEPEND="test? ( cat/test-dep )"
fi
END

mkdir -p "cat/einstalldocs" || exit 1
cat << 'END' > cat/einstalldocs/einstalldocs-6.ebuild || exit 1
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
    echo README >README || die
    echo README0.txt >README0.txt || die
    touch README1.txt || die
    echo testing.txt >testing.txt || die
    echo testing2.txt >testing2.txt || die
}

src_install() {
    d="${D}"/usr/share/doc/${PF}
    docinto testing

    dodoc testing.txt
    [[ -f ${d}/testing/testing.txt ]] || die testing/testing.txt

    [[ -n $(declare -F einstalldocs) ]] || die not defined
    einstalldocs || die einstalldocs
    [[ -d ${d} ]] || die ${d}
    [[ -f ${d}/README ]] || die README
    [[ -f ${d}/README0.txt ]] || die README0.txt
    [[ -f ${d}/README1.txt ]] && die README1.txt
    [[ -d ${d}/html ]] && die html

    dodoc testing2.txt
    [[ -f ${d}/testing/testing2.txt ]] || die testing/testing2.txt
}
END

mkdir -p "cat/einstalldocs-nothing" || exit 1
cat << 'END' > cat/einstalldocs-nothing/einstalldocs-nothing-6.ebuild || exit 1
EAPI="6"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"

S=${WORKDIR}

src_install() {
    d="${D}"/usr/share/doc/${PF}

    einstalldocs || die einstalldocs
    [[ -d ${d} ]] && die ${d}
    [[ -d ${d}/html ]] && die html
}
END

mkdir -p "cat/einstalldocs-DOCS" || exit 1
cat << 'END' > cat/einstalldocs-DOCS/einstalldocs-DOCS-6.ebuild || exit 1
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
    echo README >README || die
    echo testing.txt >testing.txt || die
    echo testing2.txt >testing2.txt || die
    touch empty.txt || die
    mkdir dir || die
    echo dir/testing3.txt >dir/testing3.txt || die
}

DOCS="testing*.txt empty.txt dir"

src_install() {
    d="${D}"/usr/share/doc/${PF}

    einstalldocs || die einstalldocs
    [[ -d ${d} ]] || die ${d}
    [[ -f ${d}/README ]] && die README
    [[ -f ${d}/testing.txt ]] || die testing.txt
    [[ -f ${d}/testing2.txt ]] || die testing2.txt
    [[ -f ${d}/empty.txt ]] && die empty.txt
    [[ -f ${d}/dir/testing3.txt ]] || die dir/testing3.txt
    [[ -d ${d}/html ]] && die html
}
END

mkdir -p "cat/einstalldocs-DOCS-array" || exit 1
cat << 'END' > cat/einstalldocs-DOCS-array/einstalldocs-DOCS-array-6.ebuild || exit 1
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
    echo README >README || die
    echo testing.txt >testing.txt || die
    echo testing2.txt >testing2.txt || die
    touch empty.txt || die
    mkdir dir || die
    echo dir/testing3.txt >dir/testing3.txt || die
    echo "with some spaces.txt" >"with some spaces.txt" || die
}

DOCS=( testing.txt empty.txt dir "with some spaces.txt" )

src_install() {
    d="${D}"/usr/share/doc/${PF}

    einstalldocs || die einstalldocs
    [[ -d ${d} ]] || die ${d}
    [[ -f ${d}/README ]] && die README
    [[ -f ${d}/testing.txt ]] || die testing.txt
    [[ -f ${d}/testing2.txt ]] && die testing2.txt
    [[ -f ${d}/empty.txt ]] && die empty.txt
    [[ -f ${d}/dir/testing3.txt ]] || die dir/testing3.txt
    [[ -f ${d}/"with some spaces.txt" ]] || die "with some spaces.txt"
    [[ -d ${d}/html ]] && die html
}
END

mkdir -p "cat/einstalldocs-empty-DOCS" || exit 1
cat << 'END' > cat/einstalldocs-empty-DOCS/einstalldocs-empty-DOCS-6.ebuild || exit 1
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
    echo README >README || die
    echo testing.txt >testing.txt || die
}

DOCS=""

src_install() {
    d="${D}"/usr/share/doc/${PF}

    einstalldocs || die einstalldocs
    [[ -d ${d} ]] && die ${d}
    [[ -f ${d}/README ]] && die README
    [[ -f ${d}/testing.txt ]] && die testing.txt
    [[ -d ${d}/html ]] && die html
}
END

mkdir -p "cat/einstalldocs-empty-DOCS-array" || exit 1
cat << 'END' > cat/einstalldocs-empty-DOCS-array/einstalldocs-empty-DOCS-array-6.ebuild || exit 1
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
    echo README >README || die
    echo testing.txt >testing.txt || die
}

DOCS=( )

src_install() {
    d="${D}"/usr/share/doc/${PF}

    einstalldocs || die einstalldocs
    [[ -d ${d} ]] && die ${d}
    [[ -f ${d}/README ]] && die README
    [[ -f ${d}/testing.txt ]] && die testing.txt
    [[ -d ${d}/html ]] && die html
}
END

mkdir -p "cat/einstalldocs-HTML_DOCS" || exit 1
cat << 'END' > cat/einstalldocs-HTML_DOCS/einstalldocs-HTML_DOCS-6.ebuild || exit 1
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
    echo README >README || die
    echo testing.html >testing.html || die
    echo testing2.html >testing2.html || die
    touch empty.html || die
    mkdir dir || die
    echo dir/testing3.html >dir/testing3.html || die
}

HTML_DOCS="testing*.html empty.html dir"

src_install() {
    d="${D}"/usr/share/doc/${PF}

    einstalldocs || die einstalldocs
    [[ -d ${d} ]] || die ${d}
    [[ -f ${d}/README ]] || die README
    [[ -d ${d}/html ]] || die html
    [[ -f ${d}/html/testing.html ]] || die html/testing.html
    [[ -f ${d}/html/testing2.html ]] || die html/testing2.html
    [[ -f ${d}/html/empty.html ]] && die html/empty.html
    [[ -f ${d}/html/dir/testing3.html ]] || die html/dir/testing3.html
}
END

mkdir -p "cat/einstalldocs-HTML_DOCS-array" || exit 1
cat << 'END' > cat/einstalldocs-HTML_DOCS-array/einstalldocs-HTML_DOCS-array-6.ebuild || exit 1
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
    echo README >README || die
    echo testing.html >testing.html || die
    echo testing2.html >testing2.html || die
    touch empty.html || die
    mkdir dir || die
    echo dir/testing3.html >dir/testing3.html || die
    echo "with some spaces.html" >"with some spaces.html" || die
}

HTML_DOCS=( testing.html empty.html dir "with some spaces.html" )

src_install() {
    d="${D}"/usr/share/doc/${PF}

    einstalldocs || die einstalldocs
    [[ -d ${d} ]] || die ${d}
    [[ -f ${d}/README ]] || die README
    [[ -d ${d}/html ]] || die html
    [[ -f ${d}/html/testing.html ]] || die testing.html
    [[ -f ${d}/html/testing2.html ]] && die testing2.html
    [[ -f ${d}/html/empty.html ]] && die empty.html
    [[ -f ${d}/html/dir/testing3.html ]] || die dir/testing3.html
    [[ -f ${d}/html/"with some spaces.html" ]] || die "with some spaces.html"
}
END

mkdir -p "cat/einstalldocs-empty-HTML_DOCS" || exit 1
cat << 'END' > cat/einstalldocs-empty-HTML_DOCS/einstalldocs-empty-HTML_DOCS-6.ebuild || exit 1
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
    echo README >README || die
    echo testing.html >testing.html || die
}

HTML_DOCS=""

src_install() {
    d="${D}"/usr/share/doc/${PF}

    einstalldocs || die einstalldocs
    [[ -d ${d} ]] || die ${d}
    [[ -f ${d}/README ]] || die README
    [[ -d ${d}/html ]] && die html
    [[ -f ${d}/html/testing.html ]] && die html/testing.html
}
END

mkdir -p "cat/einstalldocs-empty-HTML_DOCS-array" || exit 1
cat << 'END' > cat/einstalldocs-empty-HTML_DOCS-array/einstalldocs-empty-HTML_DOCS-array-6.ebuild || exit 1
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
    echo README >README || die
    echo testing.html >testing.html || die
}

HTML_DOCS=( )

src_install() {
    d="${D}"/usr/share/doc/${PF}

    einstalldocs || die einstalldocs
    [[ -d ${d} ]] || die ${d}
    [[ -f ${d}/README ]] || die README
    [[ -d ${d}/html ]] && die html
    [[ -f ${d}/html/testing.html ]] && die html/testing.html
}
END

mkdir -p "cat/einstalldocs-DOCS-HTML_DOCS" || exit 1
cat << 'END' > cat/einstalldocs-DOCS-HTML_DOCS/einstalldocs-DOCS-HTML_DOCS-6.ebuild || exit 1
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
    echo README >README || die
    echo testing.txt >testing.txt || die
    echo testing.html >testing.html || die
}

DOCS="testing.txt"
HTML_DOCS="testing.html"

src_install() {
    d="${D}"/usr/share/doc/${PF}

    einstalldocs || die einstalldocs
    [[ -d ${d} ]] || die ${d}
    [[ -f ${d}/README ]] && die README
    [[ -f ${d}/testing.txt ]] || die testing.txt
    [[ -d ${d}/html ]] || die html
    [[ -f ${d}/html/testing.html ]] || die html/testing.html
}
END

mkdir -p "cat/einstalldocs-failure" || exit 1
cat << 'END' > cat/einstalldocs-failure/einstalldocs-failure-6.ebuild || exit 1
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
    echo README >README || die
    chmod -r README || die
}

src_install() {
    einstalldocs
}
END

mkdir -p "cat/einstalldocs-nonfatal" || exit 1
cat << 'END' > cat/einstalldocs-nonfatal/einstalldocs-nonfatal-6.ebuild || exit 1
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
    echo README0.txt >README0.txt || die
    echo README1.txt >README1.txt || die
    chmod -r README1.txt || die
    echo README2.txt >README2.txt || die
    echo testing.html >testing.html || die
}

HTML_DOCS="testing.html"

src_install() {
    d="${D}"/usr/share/doc/${PF}

    nonfatal einstalldocs && die einstalldocs
    [[ -d ${d} ]] || die ${d}
    [[ -f ${d}/README0.txt ]] || die README0.txt
    [[ -f ${d}/README1.txt ]] && die README1.txt
    [[ -f ${d}/README2.txt ]] && die README2.txt
    [[ -d ${d}/html ]] && die html
    [[ -f ${d}/html/testing.html ]] && die html/testing.html
}
END

mkdir -p "cat/einstalldocs-DOCS-failure" || exit 1
cat << 'END' > cat/einstalldocs-DOCS-failure/einstalldocs-DOCS-failure-6.ebuild || exit 1
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
    echo testing.txt >testing.txt || die
    chmod -r testing.txt || die
}

DOCS="testing.txt"

src_install() {
    einstalldocs
}
END

mkdir -p "cat/einstalldocs-DOCS-nonfatal" || exit 1
cat << 'END' > cat/einstalldocs-DOCS-nonfatal/einstalldocs-DOCS-nonfatal-6.ebuild || exit 1
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
    echo testing.txt >testing.txt || die
    echo testing2.txt >testing2.txt || die
    chmod -r testing2.txt || die
    echo testing3.txt >testing3.txt || die
    echo testing.html >testing.html || die
}

DOCS="testing*.txt"
HTML_DOCS="testing.html"

src_install() {
    d="${D}"/usr/share/doc/${PF}

    nonfatal einstalldocs && die einstalldocs
    [[ -d ${d} ]] || die ${d}
    [[ -f ${d}/testing.txt ]] || die testing.txt
    [[ -f ${d}/testing2.txt ]] && die testing2.txt
    [[ -f ${d}/testing3.txt ]] || die testing3.txt
    [[ -d ${d}/html ]] && die html
    [[ -f ${d}/html/testing.html ]] && die html/testing.html
}
END

mkdir -p "cat/einstalldocs-DOCS-array-failure" || exit 1
cat << 'END' > cat/einstalldocs-DOCS-array-failure/einstalldocs-DOCS-array-failure-6.ebuild || exit 1
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
    echo testing.txt >testing.txt || die
    chmod -r testing.txt || die
}

DOCS=( testing.txt )

src_install() {
    einstalldocs
}
END

mkdir -p "cat/einstalldocs-DOCS-array-nonfatal" || exit 1
cat << 'END' > cat/einstalldocs-DOCS-array-nonfatal/einstalldocs-DOCS-array-nonfatal-6.ebuild || exit 1
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
    echo testing.txt >testing.txt || die
    echo testing2.txt >testing2.txt || die
    chmod -r testing2.txt || die
    echo testing3.txt >testing3.txt || die
    echo testing.html >testing.html || die
}

DOCS=( testing{,2,3}.txt )
HTML_DOCS="testing.html"

src_install() {
    d="${D}"/usr/share/doc/${PF}

    nonfatal einstalldocs && die einstalldocs
    [[ -d ${d} ]] || die ${d}
    [[ -f ${d}/testing.txt ]] || die testing.txt
    [[ -f ${d}/testing2.txt ]] && die testing2.txt
    [[ -f ${d}/testing3.txt ]] || die testing3.txt
    [[ -d ${d}/html ]] && die html
    [[ -f ${d}/html/testing.html ]] && die html/testing.html
}
END

mkdir -p "cat/einstalldocs-HTML_DOCS-failure" || exit 1
cat << 'END' > cat/einstalldocs-HTML_DOCS-failure/einstalldocs-HTML_DOCS-failure-6.ebuild || exit 1
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
    echo testing.html >testing.html || die
    chmod -r testing.html || die
}

HTML_DOCS="testing.html"

src_install() {
    einstalldocs
}
END

mkdir -p "cat/einstalldocs-HTML_DOCS-nonfatal" || exit 1
cat << 'END' > cat/einstalldocs-HTML_DOCS-nonfatal/einstalldocs-HTML_DOCS-nonfatal-6.ebuild || exit 1
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
    echo README >README || die
    echo testing.html >testing.html || die
    echo testing2.html >testing2.html || die
    chmod -r testing2.html || die
    echo testing3.html >testing3.html || die
}

HTML_DOCS="testing*.html"

src_install() {
    d="${D}"/usr/share/doc/${PF}

    nonfatal einstalldocs && die einstalldocs
    [[ -d ${d} ]] || die ${d}
    [[ -f ${d}/README ]] || die README
    [[ -d ${d}/html ]] || die html
    [[ -f ${d}/html/testing.html ]] || die html/testing.html
    [[ -f ${d}/html/testing2.html ]] && die html/testing2.html
    [[ -f ${d}/html/testing3.html ]] || die html/testing3.html
}
END

mkdir -p "cat/einstalldocs-HTML_DOCS-array-failure" || exit 1
cat << 'END' > cat/einstalldocs-HTML_DOCS-array-failure/einstalldocs-HTML_DOCS-array-failure-6.ebuild || exit 1
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
    echo testing.html >testing.html || die
    chmod -r testing.html || die
}

HTML_DOCS=( testing.html )

src_install() {
    einstalldocs
}
END

mkdir -p "cat/einstalldocs-HTML_DOCS-array-nonfatal" || exit 1
cat << 'END' > cat/einstalldocs-HTML_DOCS-array-nonfatal/einstalldocs-HTML_DOCS-array-nonfatal-6.ebuild || exit 1
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
    echo README >README || die
    echo testing.html >testing.html || die
    echo testing2.html >testing2.html || die
    chmod -r testing2.html || die
    echo testing3.html >testing3.html || die
}

HTML_DOCS=( testing{,2,3}.html )

src_install() {
    d="${D}"/usr/share/doc/${PF}

    nonfatal einstalldocs && die einstalldocs
    [[ -d ${d} ]] || die ${d}
    [[ -f ${d}/README ]] || die README
    [[ -d ${d}/html ]] || die html
    [[ -f ${d}/html/testing.html ]] || die testing.html
    [[ -f ${d}/html/testing2.html ]] && die testing2.html
    [[ -f ${d}/html/testing3.html ]] || die testing3.html
}
END

mkdir -p "cat/default_src_install" || exit 1
cat << 'END' > cat/default_src_install/default_src_install-6.ebuild || exit 1
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
    [[ -d ${d}/html ]] || die html
    [[ -f ${d}/html/testing.html ]] || die html/testing.html
}
END

mkdir -p "cat/eapply/files/subdir" || exit 1
cat << 'END' > cat/eapply/eapply-6.ebuild || exit 1
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
    echo first >file || die
    echo donkey >file2 || die
}

src_prepare() {
    [[ -n $(declare -F eapply) ]] || die not defined

    eapply "${FILESDIR}"/first "${FILESDIR}"/subdir "${FILESDIR}"/"last with spaces" || die eapply
    [[ $(< file) == seventh ]] || die file
    [[ $(< file2) == donkey ]] || die file2
}
END
cat << 'END' > cat/eapply/files/first || exit 1
--- directory/file
+++ directory/file
@@ -1 +1 @@
-first
+second
END
cat << 'END' > cat/eapply/files/subdir/A.patch || exit 1
--- directory/file
+++ directory/file
@@ -1 +1 @@
-second
+third
END
cat << 'END' > cat/eapply/files/subdir/B.diff || exit 1
--- directory/file
+++ directory/file
@@ -1 +1 @@
-third
+fourth
END
cat << 'END' > cat/eapply/files/subdir/"C with spaces".patch || exit 1
--- directory/file
+++ directory/file
@@ -1 +1 @@
-fourth
+fifth
END
cat << 'END' > cat/eapply/files/subdir/D.patch~ || exit 1
--- directory/file2
+++ directory/file2
@@ -1 +1 @@
-donkey
+monkey
END
cat << 'END' > cat/eapply/files/subdir/E.txt || exit 1
--- directory/file2
+++ directory/file2
@@ -1 +1 @@
-donkey
+turkey
END
cat << 'END' > cat/eapply/files/subdir/a.patch || exit 1
--- directory/file
+++ directory/file
@@ -1 +1 @@
-fifth
+sixth
END
cat << 'END' > cat/eapply/files/"last with spaces" || exit 1
--- directory/file
+++ directory/file
@@ -1 +1 @@
-sixth
+seventh
END

mkdir -p "cat/eapply-options/files/subdir" || exit 1
cat << 'END' > cat/eapply-options/eapply-options-6.ebuild || exit 1
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
    echo first >file || die
}

src_prepare() {
    eapply -p0 "${FILESDIR}"/first.patch "${FILESDIR}"/second.patch "${FILESDIR}"/subdir/third.patch || die eapply
    [[ $(< file) == fourth ]] || die file
}
END
cat << 'END' > cat/eapply-options/files/first.patch || exit 1
--- file
+++ file
@@ -1 +1 @@
-first
+second
END
cat << 'END' > cat/eapply-options/files/second.patch || exit 1
--- file
+++ file
@@ -1 +1 @@
-second
+third
END
cat << 'END' > cat/eapply-options/files/subdir/third.patch || exit 1
--- file
+++ file
@@ -1 +1 @@
-third
+fourth
END

mkdir -p "cat/eapply-dashdash/files" || exit 1
cat << 'END' > cat/eapply-dashdash/eapply-dashdash-6.ebuild || exit 1
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
    echo first >file || die
    cp "${FILESDIR}"/-p2 . || die
}

src_prepare() {
    eapply -p0 -- -p2 || die eapply
    [[ $(< file) == second ]] || die file
}
END
cat << 'END' > cat/eapply-dashdash/files/-p2 || exit 1
--- file
+++ file
@@ -1 +1 @@
-first
+second
END

mkdir -p "cat/eapply-missing/files" || exit 1
cat << 'END' > cat/eapply-missing/eapply-missing-6.ebuild || exit 1
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
    echo first >file || die
}

src_prepare() {
    eapply "${FILESDIR}"/first.patch
}
END
cat << 'END' > cat/eapply-missing/files/second.patch || exit 1
--- directory/file
+++ directory/file
@@ -1 +1 @@
-second
+third
END

mkdir -p "cat/eapply-failure/files" || exit 1
cat << 'END' > cat/eapply-failure/eapply-failure-6.ebuild || exit 1
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
    echo first >file || die
}

src_prepare() {
    eapply "${FILESDIR}"/fail.patch
}
END
cat << 'END' > cat/eapply-failure/files/fail.patch || exit 1
--- directory/file
+++ directory/file
@@ -1 +1 @@
-fourth
+fifth
END

mkdir -p "cat/eapply-nonfatal/files" || exit 1
cat << 'END' > cat/eapply-nonfatal/eapply-nonfatal-6.ebuild || exit 1
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
    echo first >file || die
    echo donkey >file2 || die
}

src_prepare() {
    nonfatal eapply "${FILESDIR}"/first.patch "${FILESDIR}"/fail.patch "${FILESDIR}"/last.patch && die eapply
    [[ $(< file) == second ]] || die file
    [[ $(< file2) == donkey ]] || die file2
}
END
cat << 'END' > cat/eapply-nonfatal/files/first.patch || exit 1
--- directory/file
+++ directory/file
@@ -1 +1 @@
-first
+second
END
cat << 'END' > cat/eapply-nonfatal/files/fail.patch || exit 1
--- directory/file
+++ directory/file
@@ -1 +1 @@
-fourth
+fifth
END
cat << 'END' > cat/eapply-nonfatal/files/last.patch || exit 1
--- directory/file2
+++ directory/file2
@@ -1 +1 @@
-donkey
+monkey
END

mkdir -p "cat/eapply-dir-failure/files/subdir" || exit 1
cat << 'END' > cat/eapply-dir-failure/eapply-dir-failure-6.ebuild || exit 1
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
    echo first >file || die
}

src_prepare() {
    eapply "${FILESDIR}"/subdir
}
END
cat << 'END' > cat/eapply-dir-failure/files/subdir/fail.patch || exit 1
--- file
+++ file
@@ -1 +1 @@
-first
+second
END

mkdir -p "cat/eapply-dir-nonfatal/files/subdir" || exit 1
cat << 'END' > cat/eapply-dir-nonfatal/eapply-dir-nonfatal-6.ebuild || exit 1
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
    echo first >file || die
    echo donkey >file2 || die
}

src_prepare() {
    nonfatal eapply "${FILESDIR}"/subdir && die eapply
    [[ $(< file) == second ]] || die file
    [[ $(< file2) == donkey ]] || die file2
}
END
cat << 'END' > cat/eapply-dir-nonfatal/files/subdir/A.patch || exit 1
--- directory/file
+++ directory/file
@@ -1 +1 @@
-first
+second
END
cat << 'END' > cat/eapply-dir-nonfatal/files/subdir/B.patch || exit 1
--- file
+++ file
@@ -1 +1 @@
-second
+third
END
cat << 'END' > cat/eapply-dir-nonfatal/files/subdir/C.patch || exit 1
--- directory/file2
+++ directory/file2
@@ -1 +1 @@
-donkey
+monkey
END

mkdir -p "cat/eapply-badmix/files/subdir" || exit 1
cat << 'END' > cat/eapply-badmix/eapply-badmix-6.ebuild || exit 1
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
    echo first >file || die
}

src_prepare() {
    nonfatal eapply "${FILESDIR}"/first.patch -p0 "${FILESDIR}"/second.patch
}
END
cat << 'END' > cat/eapply-badmix/files/first.patch || exit 1
--- directory/file
+++ directory/file
@@ -1 +1 @@
-first
+second
END
cat << 'END' > cat/eapply-badmix/files/second.patch || exit 1
--- file
+++ file
@@ -1 +1 @@
-second
+third
END

mkdir -p "cat/eapply-nopatches" || exit 1
cat << 'END' > cat/eapply-nopatches/eapply-nopatches-6.ebuild || exit 1
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
    echo first >file || die
}

src_prepare() {
    nonfatal eapply -p0
}
END

mkdir -p "cat/eapply-dir-nopatches/files/subdir" || exit 1
cat << 'END' > cat/eapply-dir-nopatches/eapply-dir-nopatches-6.ebuild || exit 1
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
    echo first >file || die
}

src_prepare() {
    nonfatal eapply "${FILESDIR}"/subdir
}
END
cat << 'END' > cat/eapply-dir-nopatches/files/subdir/first.patch~ || exit 1
--- directory/file
+++ directory/file
@@ -1 +1 @@
-first
+second
END

mkdir -p "cat/eapply_user" || exit 1
cat << 'END' > cat/eapply_user/eapply_user-6.ebuild || exit 1
EAPI="6"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"

S=${WORKDIR}

src_prepare() {
    [[ -n $(declare -F eapply_user) ]] || die not defined
    eapply_user || die eapply_user
}
END

mkdir -p "../root/var/paludis/user_patches/cat/eapply_user2-6" || exit 1
cat << 'END' > ../root/var/paludis/user_patches/cat/eapply_user2-6/eapply_user2-6.patch || exit 1
--- directory/file
+++ directory/file
@@ -1 +1 @@
-first
+second
END

mkdir -p "cat/eapply_user2" || exit 1
cat << 'END' > cat/eapply_user2/eapply_user2-6.ebuild || exit 1
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
    echo first > file || die
}

src_prepare() {
    [[ -n $(declare -F eapply_user) ]] || die not defined
    eapply_user || die eapply_user
    local f=$(cat file)
    [ "$f" != "second" ] && die patch failed
}
END

mkdir -p "../root/var/paludis/user_patches/cat/eapply_user3-6-r1" || exit 1
cat << 'END' > ../root/var/paludis/user_patches/cat/eapply_user3-6-r1/eapply_user3-6-r1.patch || exit 1
--- directory/file
+++ directory/file
@@ -1 +1 @@
-first
+second
END

mkdir -p "cat/eapply_user3" || exit 1
cat << 'END' > cat/eapply_user3/eapply_user3-6-r1.ebuild || exit 1
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
    echo first > file || die
}

src_prepare() {
    [[ -n $(declare -F eapply_user) ]] || die not defined
    eapply_user || die eapply_user
    local f=$(cat file)
    [ "$f" != "second" ] && die patch failed
}
END

mkdir -p "../root/var/paludis/user_patches/cat/eapply_user4-6:1" || exit 1
cat << 'END' > ../root/var/paludis/user_patches/cat/eapply_user4-6:1/eapply_user4-6-r1.patch || exit 1
--- directory/file
+++ directory/file
@@ -1 +1 @@
-first
+second
END

mkdir -p "cat/eapply_user4" || exit 1
cat << 'END' > cat/eapply_user4/eapply_user4-6-r1.ebuild || exit 1
EAPI="6"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="1"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"

S=${WORKDIR}

src_unpack() {
    echo first > file || die
}

src_prepare() {
    [[ -n $(declare -F eapply_user) ]] || die not defined
    eapply_user || die eapply_user
    local f=$(cat file)
    [ "$f" != "second" ] && die patch failed
}
END

mkdir -p "../root/var/paludis/user_patches/cat/eapply_user5" || exit 1
cat << 'END' > ../root/var/paludis/user_patches/cat/eapply_user5/eapply_user5.patch || exit 1
--- directory/file
+++ directory/file
@@ -1 +1 @@
-first
+second
END

mkdir -p "cat/eapply_user5" || exit 1
cat << 'END' > cat/eapply_user5/eapply_user5-6.ebuild || exit 1
EAPI="6"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="1"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"

S=${WORKDIR}

src_unpack() {
    echo first > file || die
}

src_prepare() {
    [[ -n $(declare -F eapply_user) ]] || die not defined
    eapply_user || die eapply_user
    local f=$(cat file)
    [ "$f" != "second" ] && die patch failed
}
END

# test for empty patch directory
mkdir -p "../root/var/paludis/user_patches/cat/eapply_user6-6" || exit 1
mkdir -p "cat/eapply_user6" || exit 1
cat << 'END' > cat/eapply_user6/eapply_user6-6.ebuild || exit 1
EAPI="6"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"

S=${WORKDIR}

src_prepare() {
    [[ -n $(declare -F eapply_user) ]] || die not defined
    eapply_user || die eapply_user
}
END

mkdir -p "cat/default_src_prepare-nothing/files" || exit 1
cat << 'END' > cat/default_src_prepare-nothing/default_src_prepare-nothing-6.ebuild || exit 1
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
    echo first > file || die
}

src_configure() {
    [[ $(< file) == first ]] || die file
}
END
cat << 'END' > cat/default_src_prepare-nothing/files/first.patch || exit 1
--- directory/file
+++ directory/file
@@ -1 +1 @@
-first
+second
END

mkdir -p "cat/default_src_prepare-PATCHES/files" || exit 1
cat << 'END' > cat/default_src_prepare-PATCHES/default_src_prepare-PATCHES-6.ebuild || exit 1
EAPI="6"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"

S=${WORKDIR}

PATCHES="-p0 ${FILESDIR}/first.patch ${FILESDIR}/second-*.patch"

src_unpack() {
    echo first > file || die
}

src_configure() {
    [[ $(< file) == fourth ]] || die file
}
END
cat << 'END' > cat/default_src_prepare-PATCHES/files/first.patch || exit 1
--- file
+++ file
@@ -1 +1 @@
-first
+second
END
cat << 'END' > cat/default_src_prepare-PATCHES/files/second-1.patch || exit 1
--- file
+++ file
@@ -1 +1 @@
-second
+third
END
cat << 'END' > cat/default_src_prepare-PATCHES/files/second-2.patch || exit 1
--- file
+++ file
@@ -1 +1 @@
-third
+fourth
END

mkdir -p "cat/default_src_prepare-empty-PATCHES/files" || exit 1
cat << 'END' > cat/default_src_prepare-empty-PATCHES/default_src_prepare-empty-PATCHES-6.ebuild || exit 1
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
    echo first > file || die
}

PATCHES=""

src_configure() {
    [[ $(< file) == first ]] || die file
}
END
cat << 'END' > cat/default_src_prepare-empty-PATCHES/files/first.patch || exit 1
--- directory/file
+++ directory/file
@@ -1 +1 @@
-first
+second
END

mkdir -p "cat/default_src_prepare-PATCHES-array/files" || exit 1
cat << 'END' > cat/default_src_prepare-PATCHES-array/default_src_prepare-PATCHES-array-6.ebuild || exit 1
EAPI="6"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"

S=${WORKDIR}

PATCHES=( -p0 "${FILESDIR}/first with spaces.patch" "${FILESDIR}"/second-{1,2}.patch )

src_unpack() {
    echo first > file || die
}

src_configure() {
    [[ $(< file) == fourth ]] || die file
}
END
cat << 'END' > cat/default_src_prepare-PATCHES-array/files/"first with spaces".patch || exit 1
--- file
+++ file
@@ -1 +1 @@
-first
+second
END
cat << 'END' > cat/default_src_prepare-PATCHES-array/files/second-1.patch || exit 1
--- file
+++ file
@@ -1 +1 @@
-second
+third
END
cat << 'END' > cat/default_src_prepare-PATCHES-array/files/second-2.patch || exit 1
--- file
+++ file
@@ -1 +1 @@
-third
+fourth
END

mkdir -p "cat/default_src_prepare-empty-PATCHES-array/files" || exit 1
cat << 'END' > cat/default_src_prepare-empty-PATCHES-array/default_src_prepare-empty-PATCHES-array-6.ebuild || exit 1
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
    echo first > file || die
}

PATCHES=( )

src_configure() {
    [[ $(< file) == first ]] || die file
}
END
cat << 'END' > cat/default_src_prepare-empty-PATCHES-array/files/first.patch || exit 1
--- directory/file
+++ directory/file
@@ -1 +1 @@
-first
+second
END

mkdir -p "cat/bash-compat" || exit 1
cat << 'END' > cat/bash-compat/bash-compat-6.ebuild || exit 1
EAPI="6"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"

pkg_setup() {
    [[ ${BASH_COMPAT} == 4.4 ]] || [[ ${BASH_COMPAT} == 44 ]] || die BASH_COMPAT=${BASH_COMPAT}
}
END

cd ..
cd ..
