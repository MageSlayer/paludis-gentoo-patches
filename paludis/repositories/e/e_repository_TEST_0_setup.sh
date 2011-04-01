#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir e_repository_TEST_0_dir || exit 1
cd e_repository_TEST_0_dir || exit 1

mkdir -p root/etc

mkdir -p vdb
touch vdb/THISISTHEVDB

mkdir -p build
ln -s build symlinked_build

mkdir -p distdir
echo "already fetched" > distdir/already-fetched.txt || exit 1
cat <<END > distdir/expatch-success-1.patch || exit 1
--- a/bar
+++ b/bar
@@ -1 +1,3 @@
 foo
+bar
+baz
END

mkdir -p repo/{profiles/profile,metadata,eclass} || exit 1
cd repo || exit 1
echo "test-repo" >> profiles/repo_name || exit 1
echo "cat" >> profiles/categories || exit 1
cat <<END > profiles/profile/virtuals
virtual/virtual-pretend-installed cat/pretend-installed
virtual/virtual-doesnotexist cat/doesnotexist
END
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
mkdir -p "cat/in-ebuild-die"
cat <<END > cat/in-ebuild-die/in-ebuild-die-1.ebuild || exit 1
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"

pkg_setup() {
    die "boom"
}
END
mkdir -p "cat/in-subshell-die"
cat <<'END' > cat/in-subshell-die/in-subshell-die-1.ebuild || exit 1
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"

pkg_setup() {
    ( hasq test $KEYWORDS && die "boom" )
}
END
mkdir -p "cat/success"
cat <<END > cat/success/success-1.ebuild || exit 1
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

pkg_setup() {
    useq spork && die "boom"
}
END
mkdir -p "cat/unpack-die"
cat <<END > cat/unpack-die/unpack-die-1.ebuild || exit 1
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

src_unpack() {
    echo "123" > f.bz2
    unpack ./f.bz2
}
END
mkdir -p "cat/econf-die"
cat <<END > cat/econf-die/econf-die-1.ebuild || exit 1
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

src_compile() {
    econf
}
END
mkdir -p "cat/emake-fail"
cat <<END > cat/emake-fail/emake-fail-1.ebuild || exit 1
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

src_compile() {
    emake monkey
}
END
mkdir -p "cat/emake-die"
cat <<END > cat/emake-die/emake-die-1.ebuild || exit 1
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

src_compile() {
    emake monkey || die
}
END
mkdir -p "cat/einstall-die"
cat <<END > cat/einstall-die/einstall-die-1.ebuild || exit 1
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

src_install() {
    einstall
}
END
mkdir -p "cat/keepdir-die"
cat <<"END" > cat/keepdir-die/keepdir-die-1.ebuild || exit 1
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

src_install() {
    dodir /usr/share
    touch "${D}"/usr/share/monkey
    keepdir /usr/share/monkey
}
END
mkdir -p "cat/dobin-fail"
cat <<END > cat/dobin-fail/dobin-fail-1.ebuild || exit 1
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

src_install() {
    dobin monkey
}
END
mkdir -p "cat/dobin-die"
cat <<END > cat/dobin-die/dobin-die-1.ebuild || exit 1
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

src_install() {
    dobin monkey || die
}
END
mkdir -p "cat/fperms-fail"
cat <<END > cat/fperms-fail/fperms-fail-1.ebuild || exit 1
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

src_install() {
    fperms 755 monkey
}
END
mkdir -p "cat/fperms-die"
cat <<END > cat/fperms-die/fperms-die-1.ebuild || exit 1
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

src_install() {
    fperms 755 monkey || die
}
END
mkdir -p "cat/pretend-installed"
cat <<END > cat/pretend-installed/pretend-installed-2.ebuild || exit 1
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

src_compile() {
    die "not supposed to install this"
}
END
mkdir -p "cat/econf-source"
cat <<END > cat/econf-source/econf-source-0.ebuild || exit 1
EAPI="\${PV}"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

src_unpack() {
    ECONF_SOURCE=subdir
    mkdir \${S}
    cd \${S}
    mkdir subdir
    echo 'touch monkey' > subdir/configure
    chmod +x subdir/configure
}

src_install() {
    insinto /usr/bin
    doins monkey || die "no monkey"
}
END
cp cat/econf-source/econf-source-{0,1}.ebuild || exit 1
cp cat/econf-source/econf-source-{0,2}.ebuild || exit 1
mkdir -p "cat/doman"
cat <<END > cat/doman/doman-0.ebuild || exit 1
EAPI="\${PV}"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

src_compile() {
    echo \${PF} >foo.1
    mkdir dir
    echo \${PF} >dir/foo.2
    echo \${PF} >foo.2
    echo \${PF} >foo.3x
    echo \${PF} >foo.4.gz
    echo \${PF} >foo.5f.bz2
    echo \${PF} >foo.6.Z
    echo \${PF} >foo.en.7
    echo \${PF} >foo.en_GB.8
    echo \${PF} >foo.e.9
    echo \${PF} >foo.enn.n
    echo \${PF} >foo.EN.1
    echo \${PF} >foo.en-GB.2
    echo \${PF} >foo.en_gb.3
    echo \${PF} >foo.en_G.4
    echo \${PF} >foo.en_GBB.5
    echo \${PF} >foo.nonkey
    touch foo.1x
    echo \${PF} >bar.m
    echo \${PF} >bar.monkey
    echo \${PF} >baz.6
    echo \${PF} >baz.en_US.7
}

src_install() {
    doman foo.* dir/foo.* || die
    doman bar.m && die
    doman bar.monkey && die
    doman bar.1 && die
    doman -i18n=en_GB baz.* || die
    keepdir /meh || die
    cd "\${D}"/meh || die
    doman .keep* || die
    rm "\${D}"/usr/share/man/{man1/foo.1,man2/foo.2,man3/foo.3x,man4/foo.4.gz,man5/foo.5f.bz2} || die
    rm "\${D}"/usr/share/man/{man6/foo.6.Z,man7/foo.en.7,man8/foo.en_GB.8,man9/foo.e.9,mann/foo.enn.n} || die
    rm "\${D}"/usr/share/man/{man1/foo.EN.1,man2/foo.en-GB.2,man3/foo.en_gb.3,man4/foo.en_G.4} || die
    rm "\${D}"/usr/share/man/{man5/foo.en_GBB.5,mann/foo.nonkey,en_GB/man6/baz.6,en_GB/man7/baz.en_US.7} || die
    rmdir "\${D}"/usr/share/man/{man1,man2,man3,man4,man5,man6,man7,man8,man9,mann,en_GB/man6,en_GB/man7,en_GB,} || die
}
END
cp cat/doman/doman-{0,1}.ebuild || exit 1
cat <<END > cat/doman/doman-2.ebuild || exit 1
EAPI="\${PV}"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

src_compile() {
    echo \${PF} >foo.1
    mkdir dir
    echo \${PF} >dir/foo.2
    echo \${PF} >foo.3x
    echo \${PF} >foo.4.gz
    echo \${PF} >foo.5f.bz2
    echo \${PF} >foo.6.Z
    echo \${PF} >foo.en.7
    echo \${PF} >foo.en_GB.8
    echo \${PF} >foo.e.9
    echo \${PF} >foo.enn.n
    echo \${PF} >foo.EN.1
    echo \${PF} >foo.en-GB.2
    echo \${PF} >foo.en_gb.3
    echo \${PF} >foo.en_G.4
    echo \${PF} >foo.en_GBB.5
    echo \${PF} >foo.nonkey
    touch foo.1x
    echo \${PF} >bar.m
    echo \${PF} >bar.monkey
    echo \${PF} >baz.6
    echo \${PF} >baz.en_US.7
}

src_install() {
    doman foo.* dir/foo.* || die
    doman bar.m && die
    doman bar.monkey && die
    doman bar.1 && die
    doman -i18n=en_GB baz.* || die
    keepdir /meh || die
    cd "\${D}"/meh || die
    doman .keep* || die
    rm "\${D}"/usr/share/man/{man1/foo.1,man2/foo.2,man3/foo.3x,man4/foo.4.gz,man5/foo.5f.bz2} || die
    rm "\${D}"/usr/share/man/{man6/foo.6.Z,en/man7/foo.7,en_GB/man8/foo.8,man9/foo.e.9,mann/foo.enn.n} || die
    rm "\${D}"/usr/share/man/{man1/foo.EN.1,man2/foo.en-GB.2,man3/foo.en_gb.3,man4/foo.en_G.4} || die
    rm "\${D}"/usr/share/man/{man5/foo.en_GBB.5,mann/foo.nonkey,en_GB/man6/baz.6,en_US/man7/baz.7} || die
    rmdir "\${D}"/usr/share/man/{man1,man2,man3,man4,man5,man6,man9,mann,en/man7,en_GB/man6,en_GB/man8,en_US/man7,en,en_GB,en_US,} || die
}
END
mkdir -p "cat/dosym-success"
cat <<'END' > cat/dosym-success/dosym-success-1.ebuild || exit 1
EAPI="${PV}"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

src_install() {
    dosym foo /usr/bin/bar
    [[ "$(readlink ${D}/usr/bin/bar )" == "foo" ]] || die
}
END
mkdir -p "cat/best-version"
cat <<'END' > cat/best-version/best-version-0.ebuild || exit 1
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

pkg_setup() {
    if ! best_version cat/pretend-installed >/dev/null ; then
        die "failed cat/pretend-installed"
    fi

    BV1=$(best_version cat/pretend-installed )
    [[ "$BV1" == "cat/pretend-installed-1" ]] || die "BV1 is $BV1"

    if best_version cat/doesnotexist >/dev/null ; then
        die "not failed cat/doesnotexist"
    fi

    BV2=$(best_version cat/doesnotexist )
    [[ "$BV2" == "" ]] || die "BV2 is $BV2"

    if [[ -n "$PALUDIS_ENABLE_VIRTUALS_REPOSITORY" ]] ; then
        if ! best_version virtual/virtual-pretend-installed >/dev/null ; then
            die "failed virtual/virtual-pretend-installed"
        fi

        BV3=$(best_version virtual/virtual-pretend-installed )
        [[ "$BV3" == "cat/pretend-installed-1" ]] || die "BV3 is $BV3"

        if best_version virtual/virtual-doesnotexist >/dev/null ; then
            die "not failed virtual/virtual-doesnotexist"
        fi

        BV2=$(best_version virtual/virtual-doesnotexist )
        [[ "$BV4" == "" ]] || die "BV4 is $BV4"
    fi
}
END
mkdir -p "cat/has-version"
cat <<'END' > cat/has-version/has-version-0.ebuild || exit 1
EAPI="${PV}"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

pkg_setup() {
    if ! has_version cat/pretend-installed ; then
        die "failed cat/pretend-installed"
    fi

    if has_version cat/doesnotexist >/dev/null ; then
        die "not failed cat/doesnotexist"
    fi
}
END
mkdir -p "cat/match"
cat <<'END' > cat/match/match-0.ebuild || exit 1
EAPI="${PV}"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

pkg_setup() {
    if ! portageq match "${ROOT}" cat/pretend-installed >/dev/null ; then
        die "failed cat/pretend-installed"
    fi

    cat <<'DONE' > ${T}/expected
cat/pretend-installed-0
cat/pretend-installed-1
DONE
    portageq match "${ROOT}" cat/pretend-installed > ${T}/got
    cmp ${T}/expected ${T}/got || die "oops"

    if portageq match "${ROOT}" cat/doesnotexist >/dev/null ; then
        die "not failed cat/doesnotexist"
    fi

    BV2=$(portageq match "${ROOT}" cat/doesnotexist )
    [[ "$BV2" == "" ]] || die "BV2 is $BV2"
}
END
mkdir -p "cat/vars"
cat <<'END' > cat/vars/vars-0.ebuild || exit 1
EAPI="${PV}"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

pkg_setup() {
    [[ -d "${T}" ]] || die "T not a dir"
}

src_compile() {
    [[ -d "${T}" ]] || die "T not a dir"
}

pkg_preinst() {
    [[ -d "${T}" ]] || die "T not a dir"
}
END
mkdir -p "cat/src_prepare"
cat <<END > cat/src_prepare/src_prepare-0.ebuild || exit 1
EAPI="\${PV}"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

src_prepare() {
    die src_prepare
}
END
cp cat/src_prepare/src_prepare-{0,1}.ebuild || exit 1
cp cat/src_prepare/src_prepare-{0,2}.ebuild || exit 1
mkdir -p "cat/src_configure"
cat <<END > cat/src_configure/src_configure-0.ebuild || exit 1
EAPI="\${PV}"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

src_configure() {
    die src_configure
}
END
cp cat/src_configure/src_configure-{0,1}.ebuild || exit 1
cp cat/src_configure/src_configure-{0,2}.ebuild || exit 1
mkdir -p "cat/default-src_configure" || exit 1
cat << END > cat/default-src_configure/default-src_configure-2.ebuild || exit 1
EAPI="\${PV}"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

src_unpack() {
    cat <<EOF >configure
#! /bin/sh
touch foo
EOF
    chmod +x configure
    echo 'all: ; rm foo' >Makefile
}

src_compile() {
    [[ -e foo ]] || die
}
END
mkdir -p "cat/default-src_compile" || exit 1
cat << END > cat/default-src_compile/default-src_compile-2.ebuild || exit 1
EAPI="\${PV}"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

src_unpack() {
    cat <<EOF >configure
#! /bin/sh
rm Makefile
EOF
    chmod +x configure
    echo 'all: ; touch foo' >Makefile
}

src_configure() {
    :
}

src_install() {
    [[ -e foo ]] || die
}
END
mkdir -p "cat/default_src_compile" || exit 1
cat << END > cat/default_src_compile/default_src_compile-2.ebuild || exit 1
EAPI="\${PV}"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

src_unpack() {
    cat <<EOF >configure
#! /bin/sh
rm Makefile
EOF
    chmod +x configure
    echo 'all: ; touch foo' >Makefile
}

src_configure() {
    :
}

src_compile() {
    default_src_compile
    [[ -e foo ]] || die
}
END
mkdir -p "cat/src_compile-via-default-func" || exit 1
cat << END > cat/src_compile-via-default-func/src_compile-via-default-func-2.ebuild || exit 1
EAPI="\${PV}"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

src_unpack() {
    cat <<EOF >configure
#! /bin/sh
rm Makefile
EOF
    chmod +x configure
    echo 'all: ; touch foo' >Makefile
}

src_configure() {
    :
}

src_compile() {
    default
    [[ -e foo ]] || die
}
END
mkdir -p "cat/expand-vars"
cat <<"END" > cat/expand-vars/expand-vars-0.ebuild || exit 1
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="enabled-weasel broccoli"
LICENSE="GPL-2"
KEYWORDS="test"

pkg_setup() {
    [[ $USE == "enabled-weasel linguas_enabled_en linguas_enabled_en_GB linguas_enabled_en_GB@UTF-8 userland_GNU cheese " ]] \
        || die "USE=$USE is wrong"
    [[ $USERLAND == "GNU" ]] || die "USERLAND=$USERLAND is wrong"
    [[ $LINGUAS == "enabled_en enabled_en_GB enabled_en_GB@UTF-8" ]] || die "LINGUAS=$LINGUAS is wrong"
}
END
mkdir -p "cat/pkg_pretend"
cat <<"END" > cat/pkg_pretend/pkg_pretend-3.ebuild || exit 1
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="enabled-weasel broccoli"
LICENSE="GPL-2"
KEYWORDS="test"
EAPI="3"

pkg_pretend() {
    einfo "This is my pkg_pretend. There are many like it, but this one is mine."
}
END
mkdir -p "cat/pkg_pretend-failure"
cat <<"END" > cat/pkg_pretend-failure/pkg_pretend-failure-3.ebuild || exit 1
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="enabled-weasel broccoli"
LICENSE="GPL-2"
KEYWORDS="test"
EAPI="3"

pkg_pretend() {
    die "This is my pkg_pretend. There are many like it, but this one is mine."
}
END
mkdir -p "cat/default_src_install" || exit 1
cat << 'END' > cat/default_src_install/default_src_install-3.ebuild || exit 1
EAPI="${PV}"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"
EAPI="3"

S="${WORKDIR}"

src_unpack() {
    mkdir -p ${WORKDIR}
    cat <<'EOF' >${WORKDIR}/Makefile
all :
	echo spork > README
	echo monkey > README.txt
	touch README.foo
	echo gerbil > GERBIL

install :
	echo spork > $(DESTDIR)/EATME
EOF
}

pkg_preinst() {
    [[ -e ${D}/usr/share/doc/${PF}/README ]] || die README
    [[ -e ${D}/usr/share/doc/${PF}/README.txt ]] || die README.txt
    [[ -e ${D}/usr/share/doc/${PF}/README.foo ]] && die README.foo
    [[ -e ${D}/usr/share/doc/${PF}/GERBIL ]] && die GERBIL
    [[ -e ${D}/EATME ]] || die EATME
}
END
mkdir -p "cat/docompress" || exit 1
cat << 'END' > cat/docompress/docompress-3.ebuild || exit 1
EAPI="${PV}"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"
EAPI="3"

S="${WORKDIR}"

src_install() {
    docompress foo || die
    docompress bar || die
}
END
mkdir -p "cat/dodoc-r" || exit 1
cat << 'END' > cat/dodoc-r/dodoc-r-3.ebuild || exit 1
EAPI="${PV}"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"
EAPI="3"

S="${WORKDIR}"

src_unpack() {
    mkdir -p ${WORKDIR}
    cd "${WORKDIR}"

    mkdir one two three
    echo foo > one/first
    echo foo > two/second
    echo foo > four
    mkdir dot
    mkdir dot/five
    echo foo > dot/five/fifth
}

src_install() {
    dodoc -r one two three four
    cd dot
    dodoc -r .
}

pkg_preinst() {
    [[ -e ${D}/usr/share/doc/${PF}/one/first ]] || die one/first
    [[ -e ${D}/usr/share/doc/${PF}/two/second ]] || die two/second
    [[ -e ${D}/usr/share/doc/${PF}/four ]] || die four
    [[ -e ${D}/usr/share/doc/${PF}/five/fifth ]] || die five/fifth
}
END
mkdir -p "cat/doins-symlink" || exit 1
cat << 'END' > cat/doins-symlink/doins-symlink-3.ebuild || exit 1
EAPI="${PV}"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"
EAPI="3"

S="${WORKDIR}"

src_unpack() {
    mkdir -p ${WORKDIR}
    cd "${WORKDIR}"

    mkdir a
    cd a
    echo qwerty > qwerty
    ln -s qwerty uiop
    ln -s qwerty adfs

    cd ..

    mkdir b
    cd b
    echo foo > foo
    ln -s foo bar

}

src_install() {
    insinto /foo
    doins a/qwerty
    doins a/uiop
    newins a/adfs asdf
    cd b
    doins -r .
}

pkg_preinst() {
    [[ -f ${D}/foo/qwerty ]] || die qwerty
    [[ -L ${D}/foo/uiop ]] || die uiop
    [[ $(readlink ${D}/foo/uiop ) == qwerty ]] || die sym
    [[ -L ${D}/foo/asdf ]] || die asdf
    [[ $(readlink ${D}/foo/asdf ) == qwerty ]] || die sym
    [[ -f ${D}/foo/foo ]] || die foo
    [[ -L ${D}/foo/bar ]] || die bar
    [[ $(readlink ${D}/foo/bar ) == foo ]] || die sym
}
END
mkdir -p "cat/banned-functions"
cat <<END > cat/banned-functions/banned-functions-3.ebuild || exit 1
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"
EAPI="3"

S="\${WORKDIR}"

src_install() {
    touch foo
    dohard foo bar
}
END
mkdir -p "cat/econf-disable-dependency-tracking" || exit 1
cat << 'END' > cat/econf-disable-dependency-tracking/econf-disable-dependency-tracking-0.ebuild || exit 1
EAPI="${PV}"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"
EAPI="0"

S="${WORKDIR}"

src_unpack() {
    mkdir -p ${WORKDIR}
    cd "${WORKDIR}"

    cat <<'EOF' > configure
#!/bin/sh

if echo "$@" | grep -q 'disable-dependency-tracking' ; then
    exit 1
fi

exit 0
EOF

    chmod +x configure
}
END
cat << 'END' > cat/econf-disable-dependency-tracking/econf-disable-dependency-tracking-3.ebuild || exit 1
EAPI="${PV}"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"
EAPI="3"

S="${WORKDIR}"

src_unpack() {
    mkdir -p ${WORKDIR}
    cd "${WORKDIR}"

    cat <<'EOF' > configure
#!/bin/sh

if ! echo "$@" | grep -q 'disable-dependency-tracking' ; then
    exit 1
fi

exit 0
EOF

    chmod +x configure
}
END
mkdir -p "cat/strict-use" || exit 1
cat << 'END' > cat/strict-use/strict-use-3.ebuild || exit 1
EAPI="${PV}"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork enabled"
LICENSE="GPL-2"
KEYWORDS="test"
EAPI="3"

S="${WORKDIR}"

pkg_setup() {
    use enabled || die "enabled not enabled"
    use spork && die "sporks are bad"
}
END
mkdir -p "cat/strict-use-fail" || exit 1
cat << 'END' > cat/strict-use-fail/strict-use-fail-3.ebuild || exit 1
EAPI="${PV}"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork enabled"
LICENSE="GPL-2"
KEYWORDS="test"
EAPI="3"

S="${WORKDIR}"

pkg_setup() {
    use pony
}
END
mkdir -p "cat/strict-use-injection" || exit 1
cat << 'END' > cat/strict-use-injection/strict-use-injection-3.ebuild || exit 1
EAPI="${PV}"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork enabled"
LICENSE="GPL-2"
KEYWORDS="test"
EAPI="3"

S="${WORKDIR}"

pkg_setup() {
    use build && die "build set"
    use userland_GNU || die "userland_GNU not set"
    use cheese || die "cheese not set"
    use otherarch && die "otherarch set"
}
END
mkdir -p "cat/global-scope-use" || exit 1
cat << 'END' > cat/global-scope-use/global-scope-use-3.ebuild || exit 1
use spork

EAPI="${PV}"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork enabled"
LICENSE="GPL-2"
KEYWORDS="test"
EAPI="3"

S="${WORKDIR}"
END
mkdir -p "cat/use-with-enable-empty-third"
cat <<'END' > cat/use-with-enable-empty-third/use-with-enable-empty-third-0.ebuild || exit 1
EAPI="${PV}"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

src_compile() {
    [[ $(use_with   cheese cheese "") == --with-cheese   ]] || die
    [[ $(use_enable cheese cheese "") == --enable-cheese ]] || die
}
END
cd ..

cd ..

