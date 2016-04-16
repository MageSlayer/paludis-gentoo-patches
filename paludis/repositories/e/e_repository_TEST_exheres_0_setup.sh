#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir e_repository_TEST_exheres_0_dir || exit 1
cd e_repository_TEST_exheres_0_dir || exit 1

mkdir -p root/etc

mkdir -p instrepo

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
echo "cat" >> metadata/categories.conf || exit 1
cat <<END > profiles/profile/make.defaults
CHOST="i286-badger-linux-gnu"
SUBOPTIONS="LINGUAS"
LINGUAS="en en_GB en_GB@UTF-8"
USERLAND="GNU"
OPTIONS="weasel spinach"
USE_EXPAND="USERLAND"
END
mkdir -p "packages/cat/in-ebuild-die"
cat <<'END' > packages/cat/in-ebuild-die/in-ebuild-die-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS=""
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

pkg_setup() {
    die "boom"
}
END
mkdir -p "packages/cat/in-subshell-die"
cat <<'END' > packages/cat/in-subshell-die/in-subshell-die-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS=""
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

pkg_setup() {
    ( hasq test $PLATFORMS && die "boom" )
}
END
mkdir -p "packages/cat/success"
cat <<'END' > packages/cat/success/success-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

pkg_setup() {
    optionq spork && die "boom"
}
END
mkdir -p "packages/cat/expatch-success"
cat <<'END' > packages/cat/expatch-success/expatch-success-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_unpack() {
    echo foo > bar
    echo -e 'foo\nbar\nbaz' > baz
}

src_prepare() {
    expatch "${FETCHEDDIR}"/${PNV}.patch
    diff bar baz || die "expatch failed"
}
END
mkdir -p "packages/cat/expatch-success-dir/files/expatch-success-dir"
cat <<END > packages/cat/expatch-success-dir/files/expatch-success-dir/foo.patch || exit 1
--- a/bar
+++ b/bar
@@ -1 +1,3 @@
 foo
+bar
+baz
END
cat <<'END' > packages/cat/expatch-success-dir/expatch-success-dir-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_unpack() {
    echo foo > bar
    echo -e 'foo\nbar\nbaz' > baz
}

src_prepare() {
    expatch "${FILES}"/${PN}/
    diff bar baz || die "expatch failed"
}
END
mkdir -p "packages/cat/expatch-die"
cat <<'END' > packages/cat/expatch-die/expatch-die-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_prepare() {
    expatch monkey.patch
}
END
mkdir -p "packages/cat/expatch-unrecognised/files/expatch-unrecognised"
cat <<END > packages/cat/expatch-unrecognised/files/expatch-unrecognised/foo || exit 1
--- a/bar
+++ b/bar
@@ -1 +1,3 @@
 foo
+bar
+baz
END
cat <<'END' > packages/cat/expatch-unrecognised/expatch-unrecognised-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_unpack() {
    echo foo > bar
    echo foo > baz
}

src_prepare() {
    expatch "${FILES}"/${PN}/
    diff bar baz || die "expatch applied unrecognised patch"
}
END
mkdir -p "packages/cat/nonfatal-expatch-fail"
cat <<'END' > packages/cat/nonfatal-expatch-fail/nonfatal-expatch-fail-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_prepare() {
    nonfatal expatch monkey.patch
}
END
mkdir -p "packages/cat/nonfatal-expatch-die"
cat <<'END' > packages/cat/nonfatal-expatch-die/nonfatal-expatch-die-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_prepare() {
    nonfatal expatch monkey.patch || die
}
END
mkdir -p "packages/cat/unpack-die"
cat <<'END' > packages/cat/unpack-die/unpack-die-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_unpack() {
    echo "123" > f.bz2
    unpack ./f.bz2
}
END
mkdir -p "packages/cat/nonfatal-unpack-fail"
cat <<'END' > packages/cat/nonfatal-unpack-fail/nonfatal-unpack-fail-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_unpack() {
    echo "123" > f.bz2
    nonfatal unpack ./f.bz2
}
END
mkdir -p "packages/cat/nonfatal-unpack-die"
cat <<'END' > packages/cat/nonfatal-unpack-die/nonfatal-unpack-die-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_unpack() {
    echo "123" > f.bz2
    nonfatal unpack ./f.bz2 || die
}
END
mkdir -p "packages/cat/econf-fail"
cat <<'END' > packages/cat/econf-fail/econf-fail-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_configure() {
    econf
}
END
mkdir -p "packages/cat/nonfatal-econf"
cat <<'END' > packages/cat/nonfatal-econf/nonfatal-econf-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_configure() {
    nonfatal econf
}
END
mkdir -p "packages/cat/nonfatal-econf-die"
cat <<'END' > packages/cat/nonfatal-econf-die/nonfatal-econf-die-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_configure() {
    nonfatal econf || die
}
END
mkdir -p "packages/cat/emake-fail"
cat <<'END' > packages/cat/emake-fail/emake-fail-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_compile() {
    emake monkey
}
END
mkdir -p "packages/cat/nonfatal-emake"
cat <<'END' > packages/cat/nonfatal-emake/nonfatal-emake-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_compile() {
    nonfatal emake monkey
}
END
mkdir -p "packages/cat/nonfatal-emake-die"
cat <<'END' > packages/cat/nonfatal-emake-die/nonfatal-emake-die-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_compile() {
    nonfatal emake monkey || die
}
END
mkdir -p "packages/cat/keepdir-success"
cat <<'END' > packages/cat/keepdir-success/keepdir-success-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_install() {
    keepdir /usr/share/monkey
}
END
mkdir -p "packages/cat/keepdir-fail"
cat <<'END' > packages/cat/keepdir-fail/keepdir-fail-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_install() {
    nonfatal dodir /usr/share || return
    touch "${IMAGE}"/usr/share/monkey || return
    keepdir /usr/share/monkey
}
END
cat <<'END' > packages/cat/keepdir-fail/keepdir-fail-2.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_install() {
    nonfatal dosym no/where /usr/share/.keep_${CATEGORY}_${PN}-${SLOT%/*} || return
    keepdir /usr/share
}
END
mkdir -p "packages/cat/nonfatal-keepdir"
cat <<'END' > packages/cat/nonfatal-keepdir/nonfatal-keepdir-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_install() {
    dodir /usr/share
    touch "${IMAGE}"/usr/share/monkey
    nonfatal keepdir /usr/share/monkey
}
END
mkdir -p "packages/cat/nonfatal-keepdir-die"
cat <<'END' > packages/cat/nonfatal-keepdir-die/nonfatal-keepdir-die-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_install() {
    dodir /usr/share
    touch "${IMAGE}"/usr/share/monkey
    nonfatal keepdir /usr/share/monkey || die
}
END
mkdir -p "packages/cat/dobin-success"
cat <<'END' > packages/cat/dobin-success/dobin-success-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_compile() {
    touch foo
}

src_install() {
    dobin foo
}
END
mkdir -p "packages/cat/dobin-fail"
cat <<'END' > packages/cat/dobin-fail/dobin-fail-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_install() {
    dobin monkey
}
END
mkdir -p "packages/cat/nonfatal-dobin-success"
cat <<'END' > packages/cat/nonfatal-dobin-success/nonfatal-dobin-success-1.ebuild || exit 1
DESCRIPTION="The Lnog Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_compile() {
    touch foo
}

src_install() {
    nonfatal dobin foo || die
}
END
mkdir -p "packages/cat/nonfatal-dobin-fail"
cat <<'END' > packages/cat/nonfatal-dobin-fail/nonfatal-dobin-fail-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_install() {
    nonfatal dobin monkey
}
END
mkdir -p "packages/cat/nonfatal-dobin-die"
cat <<'END' > packages/cat/nonfatal-dobin-die/nonfatal-dobin-die-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_install() {
    nonfatal dobin monkey || die
}
END
mkdir -p "packages/cat/herebin-success"
cat <<'END' > packages/cat/herebin-success/herebin-success-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_install() {
    herebin foo <<EOF
blah
EOF
}
END
mkdir -p "packages/cat/herebin-fail"
cat <<'END' > packages/cat/herebin-fail/herebin-fail-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_install() {
    herebin <<EOF
blah
EOF
}
END
mkdir -p "packages/cat/hereconfd-success"
cat <<'END' > packages/cat/hereconfd-success/hereconfd-success-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_install() {
    hereconfd foo <<EOF
blah
EOF
}
END
mkdir -p "packages/cat/hereconfd-fail"
cat <<'END' > packages/cat/hereconfd-fail/hereconfd-fail-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_install() {
    hereconfd <<EOF
blah
EOF
}
END
mkdir -p "packages/cat/hereenvd-success"
cat <<'END' > packages/cat/hereenvd-success/hereenvd-success-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_install() {
    hereenvd foo <<EOF
blah
EOF
}
END
mkdir -p "packages/cat/hereenvd-fail"
cat <<'END' > packages/cat/hereenvd-fail/hereenvd-fail-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_install() {
    hereenvd <<EOF
blah
EOF
}
END
mkdir -p "packages/cat/hereinitd-success"
cat <<'END' > packages/cat/hereinitd-success/hereinitd-success-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_install() {
    hereinitd foo <<EOF
blah
EOF
}
END
mkdir -p "packages/cat/hereinitd-fail"
cat <<'END' > packages/cat/hereinitd-fail/hereinitd-fail-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_install() {
    hereinitd <<EOF
blah
EOF
}
END
mkdir -p "packages/cat/hereins-success"
cat <<'END' > packages/cat/hereins-success/hereins-success-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_install() {
    hereins foo <<EOF
blah
EOF
}
END
mkdir -p "packages/cat/hereins-fail"
cat <<'END' > packages/cat/hereins-fail/hereins-fail-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_install() {
    hereins <<EOF
blah
EOF
}
END
mkdir -p "packages/cat/heresbin-success"
cat <<'END' > packages/cat/heresbin-success/heresbin-success-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_install() {
    heresbin foo <<EOF
blah
EOF
}
END
mkdir -p "packages/cat/heresbin-fail"
cat <<'END' > packages/cat/heresbin-fail/heresbin-fail-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_install() {
    heresbin <<EOF
blah
EOF
}
END
mkdir -p "packages/cat/best-version"
cat <<'END' > packages/cat/best-version/best-version-0.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

pkg_setup() {
    if ! best_version cat/pretend-installed >/dev/null ; then
        die "failed cat/pretend-installed"
    fi

    BV1=$(best_version cat/pretend-installed )
    [[ "$BV1" == "cat/pretend-installed-1:0::installed" ]] || die "BV1 is $BV1"

    if best_version cat/doesnotexist >/dev/null ; then
        die "not failed cat/doesnotexist"
    fi

    BV2=$(best_version cat/doesnotexist )
    [[ "$BV2" == "" ]] || die "BV2 is $BV2"
}
END
mkdir -p "packages/cat/has-version"
cat <<'END' > packages/cat/has-version/has-version-0.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

pkg_setup() {
    if ! has_version cat/pretend-installed ; then
        die "failed cat/pretend-installed"
    fi

    if has_version cat/doesnotexist >/dev/null ; then
        die "not failed cat/doesnotexist"
    fi
}
END
mkdir -p "packages/cat/match"
cat <<'END' > packages/cat/match/match-0.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

pkg_setup() {
    portageq match "${ROOT}" cat/foo | while read a ; do
        einfo moo
    done
}
END
mkdir -p "packages/cat/econf-phase"
cat <<'END' > packages/cat/econf-phase/econf-phase-0.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"

HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_unpack() {
    mkdir ${WORK}
    echo "#!/usr/bin/env bash" > ${WORK}/configure
    chmod +x ${WORK}/configure
}

src_compile() {
    econf
}
END
mkdir -p "packages/cat/econf-vars"
cat <<'END' > packages/cat/econf-vars/econf-vars-0.ebuild || exit 1
DESCRIPTION="The Long Description"
DESCRIPTION="The Short Description"

HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="enabled-hamster gerbil dormouse"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

DEFAULT_SRC_CONFIGURE_PARAMS=( --nice-juicy-steak )
DEFAULT_SRC_CONFIGURE_OPTION_ENABLES=( enabled-hamster gerbil )
DEFAULT_SRC_CONFIGURE_OPTION_WITHS=( dormouse )

src_unpack() {
    mkdir ${WORK}
    cat <<'END2' > ${WORK}/configure
#!/usr/bin/env bash
echo "${@}" | grep -q -- '--enable-enabled-hamster' || exit 1
echo "${@}" | grep -q -- '--disable-gerbil' || exit 2
echo "${@}" | grep -q -- '--nice-juicy-steak' || exit 3
echo "${@}" | grep -q -- '--without-dormouse' || exit 4
true
END2
    chmod +x ${WORK}/configure
}
END
mkdir -p "packages/cat/expand-vars"
cat <<"END" > packages/cat/expand-vars/expand-vars-0.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="enabled-weasel broccoli linguas: enabled-en_GB"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

pkg_setup() {
    [[ ${OPTIONS%%+( )} == "enabled-weasel linguas:enabled-en_GB" ]] || die "OPTIONS=$OPTIONS is wrong"
    [[ ${LINGUAS%%+( )} == "enabled-en_GB" ]] || die "LINGUAS=$LINGUAS is wrong"
}
END
mkdir -p "packages/cat/doman-success"
cat <<'END' > packages/cat/doman-success/doman-success-0.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_compile() {
    echo ${PNVR} >foo.1
    mkdir dir
    echo ${PNVR} >dir/foo.2
    echo ${PNVR} >foo.3x
    echo ${PNVR} >foo.4.gz
    echo ${PNVR} >foo.5f.bz2
    echo ${PNVR} >foo.6.Z
    echo ${PNVR} >foo.en.7
    echo ${PNVR} >foo.en_GB.8
    echo ${PNVR} >foo.e.9
    echo ${PNVR} >foo.enn.n
    echo ${PNVR} >foo.EN.1
    echo ${PNVR} >foo.en-GB.2
    echo ${PNVR} >foo.en_gb.3
    echo ${PNVR} >foo.en_G.4
    echo ${PNVR} >foo.en_GBB.5
    echo ${PNVR} >foo.nonkey
    touch foo.1x
    echo ${PNVR} >baz.6
    echo ${PNVR} >baz.en_US.7
}

src_install() {
    doman foo.* dir/foo.* || die
    doman -i18n=en_GB baz.* || die
    keepdir /meh || die
    cd "${IMAGE}"/meh || die
    doman .keep* || die
    rm "${IMAGE}"/usr/share/man/{man1/foo.1,man2/foo.2,man3/foo.3x,man4/foo.4.gz,man5/foo.5f.bz2} || die
    rm "${IMAGE}"/usr/share/man/{man6/foo.6.Z,en/man7/foo.7,en_GB/man8/foo.8,man9/foo.e.9,mann/foo.enn.n} || die
    rm "${IMAGE}"/usr/share/man/{man1/foo.EN.1,man2/foo.en-GB.2,man3/foo.en_gb.3,man4/foo.en_G.4} || die
    rm "${IMAGE}"/usr/share/man/{man5/foo.en_GBB.5,mann/foo.nonkey,en_GB/man6/baz.6,en_US/man7/baz.7} || die
    rmdir "${IMAGE}"/usr/share/man/{man1,man2,man3,man4,man5,man6,man9,mann,en/man7,en_GB/man6,en_GB/man8,en_US/man7,en,en_GB,en_US,} || die
}
END
mkdir -p "packages/cat/doman-failure"
cat <<'END' > packages/cat/doman-failure/doman-failure-0.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_compile() {
    echo ${PNVR} >bar.m
}

src_install() {
    doman bar.m
}
END
mkdir -p "packages/cat/change-globals"
cat <<'END' > packages/cat/change-globals/change-globals-0.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_compile() {
    DEFAULT_SRC_COMPILE_PARAMS="foo"
    default
}
END
mkdir -p "packages/cat/install"
cat <<'END' > packages/cat/install/install-0.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_unpack() {
    touch -- -s
}

src_install() {
    install -v -- -s dest
    [[ -x dest ]] || die "install didn't work"
}
END
mkdir -p "packages/cat/install-s"
cat <<'END' > packages/cat/install-s/install-s-0.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_unpack() {
    touch src
}

src_install() {
    install -s src dest
    [[ -x dest ]] && die "install didn't fail"
}
END
mkdir -p "packages/cat/doman-nonfatal"
cat <<'END' > packages/cat/doman-nonfatal/doman-nonfatal-0.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_install() {
    nonfatal doman bar.1 && die
}
END
mkdir -p "packages/cat/global-optionq"
cat <<'END' > packages/cat/global-optionq/global-optionq-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

optionq spork
END
mkdir -p "packages/cat/expecting-tests-enabled"
cat <<'END' > packages/cat/expecting-tests-enabled/expecting-tests-enabled-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

pkg_setup() {
    expecting_tests || die "expecting_tests"
    expecting_tests --any || die "expecting_tests --any"
    expecting_tests --recommended || die "expecting_tests --recommended"
}
END
mkdir -p "packages/cat/expecting-tests-disabled"
cat <<'END' > packages/cat/expecting-tests-disabled/expecting-tests-disabled-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

pkg_setup() {
    expecting_tests --expensive && die "expecting_tests --expensive"
}

src_test_expensive() {
    echo monkeys
}
END
mkdir -p "packages/cat/expecting-tests-none"
cat <<'END' > packages/cat/expecting-tests-none/expecting-tests-none-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

pkg_setup() {
    expecting_tests --expensive
}
END
mkdir -p "packages/cat/banned"
cat <<'END' > packages/cat/banned/banned-0.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

pkg_setup() {
    edo banned_by_distribution illegal-executable
}
END
cat <<'END' > packages/cat/banned/banned-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_install() {
    [[ -n ${BANNEDDIR} ]] || die

    dobanned illegal-executable
    [[ -x ${IMAGE}/${BANNEDDIR}/illegal-executable ]] || die
}
END
mkdir -p "packages/cat/exdirectory-phase"
cat <<'END' > packages/cat/exdirectory-phase/exdirectory-phase-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_install() {
    exdirectory --allow /stuff
}
END
mkdir -p "packages/cat/exdirectory-forbid"
cat <<'END' > packages/cat/exdirectory-forbid/exdirectory-forbid-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

pkg_setup() {
    exdirectory --forbid /usr
}

src_install() {
    herebin stuff <<EOT
EOT
}
END
mkdir -p "packages/cat/exdirectory-allow"
cat <<'END' > packages/cat/exdirectory-allow/exdirectory-allow-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

pkg_setup() {
    exdirectory --allow /stuff
}

src_install() {
    insinto /stuff
    hereins it <<EOT
EOT
}
END
mkdir -p "packages/cat/permitted-directories"
cat <<'END' > packages/cat/permitted-directories/permitted-directories-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_install() {
    insinto /
    hereins it <<EOT
EOT
}
END
cat <<'END' > packages/cat/permitted-directories/permitted-directories-2.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_install() {
    insinto /var
    hereins it <<EOT
EOT
}
END
cat <<'END' > packages/cat/permitted-directories/permitted-directories-3.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_install() {
    insinto /var/run
    hereins it <<EOT
EOT
}
END
cat <<'END' > packages/cat/permitted-directories/permitted-directories-4.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

pkg_setup() {
    exdirectory --forbid /foo
    exdirectory --allow /foo/bar/baz
}

src_install() {
    insinto /foo/bar/baz
    hereins it <<EOT
EOT
}
END
mkdir -p "packages/cat/exvolatile"
cat <<'END' > packages/cat/exvolatile/exvolatile-1.ebuild || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"

src_install() {
    insinto /var
    hereins first <<EOT
EOT
    hereins second <<EOT
EOT
    hereins third <<EOT
EOT
    hereins fourth <<EOT
EOT
    dosym first /var/symfirst
    dosym second /var/symsecond
    dosym third /var/symthird
    dosym fourth /var/symfourth
}

pkg_setup() {
    exvolatile /var/first /var/second /var/symfirst /var/symsecond
}

pkg_postinst() {
    echo a monkey stole my broccoli > ${ROOT}/var/second
    echo there is a weasel in my stew > ${ROOT}/var/fourth
    ln -sf /dev/null ${ROOT}/var/symsecond
    ln -sf /dev/null ${ROOT}/var/symfourth
}

pkg_postrm() {
    [[ -f ${ROOT}/var/first ]] && die "first should be removed"
    [[ -f ${ROOT}/var/second ]] && die "second should be removed"
    [[ -f ${ROOT}/var/third ]] && die "third should be removed"
    [[ -f ${ROOT}/var/fourth ]] || die "fourth shouldn't be removed"
    [[ -L ${ROOT}/var/symfirst ]] && die "symfirst should be removed"
    [[ -L ${ROOT}/var/symsecond ]] && die "symsecond should be removed"
    [[ -L ${ROOT}/var/symthird ]] && die "symthird should be removed"
    [[ -L ${ROOT}/var/symfourth ]] || die "symfourth shouldn't be removed"
}
END

cd ..

cd ..
