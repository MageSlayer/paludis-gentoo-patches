#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir -p vdb_repository_TEST_dir || exit 1
cd vdb_repository_TEST_dir || exit 1

mkdir -p distdir
mkdir -p build
mkdir -p root/etc

mkdir -p repo1/cat-{one/{pkg-one-1,pkg-both-1},two/{pkg-two-2,pkg-both-2}} || exit 1

for i in SLOT EAPI; do
    echo "0" >repo1/cat-one/pkg-one-1/${i}
done

for i in DEPEND RDEPEND LICENSE INHERITED IUSE PDEPEND PROVIDE; do
    touch repo1/cat-one/pkg-one-1/${i}
done

echo "test flag1 flag2 kernel_linux" >>repo1/cat-one/pkg-one-1/USE
echo "flag1 flag2 flag3" >>repo1/cat-one/pkg-one-1/IUSE
echo "KERNEL" >repo1/cat-one/pkg-one-1/USE_EXPAND

cat <<END >repo1/cat-one/pkg-one-1/CONTENTS
dir /directory
  obj /directory/file 4 2
sym /directory/symlink -> target 2 
dir	/directory with spaces
dir /directory with trailing space 
dir /directory  with  consecutive  spaces
obj /file with spaces 4    2
obj /file  with  consecutive  spaces 4 	  2 	  
obj /file with  trailing   space	  4 2
sym /symlink -> target  with  consecutive  spaces 2
sym /symlink with spaces -> target with spaces 2
sym /symlink  with  consecutive  spaces -> target  with  consecutive  spaces 2
sym /symlink -> target -> with -> multiple -> arrows 2
sym /symlink -> target with trailing space  2
sym /symlink ->  target with leading space 2
sym /symlink with trailing space  -> target 2
fif /fifo
fif /fifo with spaces
fif /fifo  with  consecutive  spaces
dev /device
dev /device with spaces
dev /device  with  consecutive  spaces
misc /miscellaneous
misc /miscellaneous with spaces
misc /miscellaneous  with  consecutive  spaces

obj 
  obj	
obj /file
obj /file   
obj /file  2 
sym foobar 2
sym foo -> bar
sym -> bar 2
sym foo -> 2
END

touch "world-empty"
cat <<END > world-no-match
cat-one/foo
cat-one/bar
cat-one/oink
END
cat <<END > world-match
cat-one/foo
cat-one/foofoo
cat-one/bar
END
cat <<END > world-no-match-no-eol
cat-one/foo
cat-one/bar
END
echo -n "cat-one/oink" >> world-no-match-no-eol

mkdir -p repo2/category/package-1 || exit 1
echo "exheres-0" >repo2/category/package-1/EAPI
echo "0" >repo2/category/package-1/SLOT
echo "cat/pkg1 build: cat/pkg2 build+run: cat/pkg3 suggestion: cat/pkg4 post: cat/pkg5" >repo2/category/package-1/DEPENDENCIES

mkdir -p repo3

mkdir -p srcrepo/{profiles/profile,cat/{target,vars}{,-exheres},eclass}
cat <<END > srcrepo/profiles/profile/make.defaults
ARCH=test
USERLAND="GNU"
KERNEL="linux"
CHOST="i286-badger-linux-gnu"
END
echo "srcrepo" > srcrepo/profiles/repo_name || exit 1

cat <<'END' > srcrepo/cat/target/target-0.ebuild
EAPI="0"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"
DEPEND="foo/bar"

src_install() {
    echo MONKEY > ${D}/monkey
}

pkg_info() {
    echo "This is pkg_info"
}

pkg_config() {
    echo "This is pkg_config"
}
END

cat <<'END' > srcrepo/cat/target/target-1.ebuild
EAPI="1"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"
DEPEND="foo/bar"

src_install() {
    echo MONKEY > ${D}/monkey
}

pkg_info() {
    echo "This is pkg_info"
}

pkg_config() {
    echo "This is pkg_config"
}
END

cat <<'END' > srcrepo/cat/target/target-2.ebuild
EAPI="2"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"
DEPEND="foo/bar"

src_install() {
    echo MONKEY > ${D}/monkey
}

pkg_info() {
    echo "This is pkg_info"
}

pkg_config() {
    echo "This is pkg_config"
}
END

cat <<'END' > srcrepo/cat/target/target-3.ebuild
EAPI="3"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"
DEPEND="foo/bar"
S="${WORKDIR}"

src_install() {
    echo MONKEY > ${D}/monkey
}

pkg_info() {
    echo "This is pkg_info"
}

pkg_config() {
    echo "This is pkg_config"
}
END

cat <<'END' > srcrepo/cat/target-exheres/target-exheres-0.ebuild
EAPI="exheres-0"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS=""
LICENCES="GPL-2"
PLATFORMS="test"
WORK="${WORKBASE}"
DEPENDENCIES=""

src_install() {
    echo MONKEY > ${IMAGE}/monkey
}

pkg_info() {
    echo "This is pkg_info"
}

pkg_config() {
    echo "This is pkg_config"
}
END

cat <<'END' > srcrepo/cat/vars/vars-0.ebuild
EAPI="0"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"
DEPEND="foo/bar"

pkg_setup() {
    einfo "${EBUILD_PHASE}: T=${T}"
    [[ -d "${T}" ]] || die "T not a dir"
}

src_compile() {
    einfo "${EBUILD_PHASE}: T=${T}"
    [[ -d "${T}" ]] || die "T not a dir"
}

pkg_preinst() {
    einfo "${EBUILD_PHASE}: T=${T}"
    [[ -d "${T}" ]] || die "T not a dir"
}

pkg_prerm() {
    einfo "${EBUILD_PHASE}: T=${T}"
    [[ -d "${T}" ]] || die "T not a dir"
}

pkg_info() {
    einfo "${EBUILD_PHASE}: T=${T}"
    [[ -d "${T}" ]] || die "T not a dir"
}

pkg_config() {
    einfo "${EBUILD_PHASE}: T=${T}"
    [[ -d "${T}" ]] || die "T not a dir"
}
END

cat <<'END' > srcrepo/cat/vars/vars-1.ebuild
EAPI="1"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"
DEPEND="foo/bar"

pkg_setup() {
    einfo "${EBUILD_PHASE}: T=${T}"
    [[ -d "${T}" ]] || die "T not a dir"
}

src_compile() {
    einfo "${EBUILD_PHASE}: T=${T}"
    [[ -d "${T}" ]] || die "T not a dir"
}

pkg_preinst() {
    einfo "${EBUILD_PHASE}: T=${T}"
    [[ -d "${T}" ]] || die "T not a dir"
}

pkg_prerm() {
    einfo "${EBUILD_PHASE}: T=${T}"
    [[ -d "${T}" ]] || die "T not a dir"
}

pkg_info() {
    einfo "${EBUILD_PHASE}: T=${T}"
    [[ -d "${T}" ]] || die "T not a dir"
}

pkg_config() {
    einfo "${EBUILD_PHASE}: T=${T}"
    [[ -d "${T}" ]] || die "T not a dir"
}
END

cat <<'END' > srcrepo/cat/vars-exheres/vars-exheres-0.ebuild
EAPI="exheres-0"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS=""
LICENCES="GPL-2"
PLATFORMS="test"
DEPENDENCIES="foo/bar"

pkg_setup() {
    einfo "${EBUILD_PHASE}: TEMP=${TEMP}"
    [[ -d "${TEMP}" ]] || die "TEMP not a dir"
}

src_compile() {
    einfo "${EBUILD_PHASE}: TEMP=${TEMP}"
    [[ -d "${TEMP}" ]] || die "TEMP not a dir"
}

pkg_preinst() {
    einfo "${EBUILD_PHASE}: TEMP=${TEMP}"
    [[ -d "${TEMP}" ]] || die "TEMP not a dir"
}

pkg_prerm() {
    einfo "${EBUILD_PHASE}: TEMP=${TEMP}"
    [[ -d "${TEMP}" ]] || die "TEMP not a dir"
}

pkg_info() {
    einfo "${EBUILD_PHASE}: TEMP=${TEMP}"
    [[ -d "${TEMP}" ]] || die "TEMP not a dir"
}

pkg_config() {
    einfo "${EBUILD_PHASE}: TEMP=${TEMP}"
    [[ -d "${TEMP}" ]] || die "TEMP not a dir"
}
END

mkdir -p namesincrtest/.cache/names/installed namesincrtest_src/{eclass,profiles/profile,cat1/{pkg1,pkg2},{cat2,cat3}/pkg1} || exit 1
echo paludis-2 >namesincrtest/.cache/names/installed/_VERSION_
echo installed >>namesincrtest/.cache/names/installed/_VERSION_

cat <<END > namesincrtest_src/profiles/profile/make.defaults
ARCH=test
USERLAND="GNU"
KERNEL="linux"
CHOST="i286-badger-linux-gnu"
END
echo namesincrtest_src >namesincrtest_src/profiles/repo_name
echo cat1 >namesincrtest_src/profiles/categories
echo cat2 >>namesincrtest_src/profiles/categories
echo cat3 >>namesincrtest_src/profiles/categories

cat <<END >namesincrtest_src/cat1/pkg1/pkg1-1.ebuild
KEYWORDS="test"
SLOT="\${PV:0:1}"
END
cp namesincrtest_src/cat1/pkg1/pkg1-{1,1.1}.ebuild
cp namesincrtest_src/cat1/pkg1/pkg1-{1,2}.ebuild
cp namesincrtest_src/cat1/{pkg1/pkg1,pkg2/pkg2}-1.ebuild
cp namesincrtest_src/{cat1,cat2}/pkg1/pkg1-1.ebuild

cat <<END >namesincrtest_src/cat3/pkg1/pkg1-1.ebuild
EAPI=paludis-1
KEYWORDS="test"
SLOT="0"
END
cp namesincrtest_src/cat3/pkg1/pkg1-{1,2}.ebuild

mkdir -p providestest/{.cache,cat1/{pkg1,pkg2,pkg3}-{1,2}} || exit 1
for f in providestest/cat1/{pkg1,pkg2,pkg3}-{1,2}/EAPI; do
    echo 0 >${f}
done
for f in providestest/cat1/{pkg1,pkg2,pkg3}-1/SLOT; do
    echo 1 >${f}
done
for f in providestest/cat1/{pkg1,pkg2,pkg3}-2/SLOT; do
    echo 2 >${f}
done
for f in providestest/cat1/{pkg1,pkg2,pkg3}-{1,2}/USE; do
    echo enabled >${f}
done
for f in providestest/cat1/{pkg1,pkg2,pkg3}-{1,2}/IUSE; do
    echo disabled enabled >${f}
done

echo '            virtual/foo'                >providestest/cat1/pkg1-1/PROVIDE
echo 'enabled?  ( virtual/foo )'              >providestest/cat1/pkg1-2/PROVIDE
echo 'enabled?  ( virtual/foo ) virtual/bar'  >providestest/cat1/pkg2-1/PROVIDE
echo 'disabled? ( virtual/foo ) virtual/bar'  >providestest/cat1/pkg2-2/PROVIDE
echo 'disabled? ( virtual/foo )'              >providestest/cat1/pkg3-1/PROVIDE
echo ''                                       >providestest/cat1/pkg3-2/PROVIDE

mkdir -p providesincrtest/.cache providesincrtest_src{1,2}/{eclass,profiles/profile,{cat1,cat2}/{pkg1,pkg2}} || exit 1
echo paludis-3 >providesincrtest/.cache/provides
echo installed >>providesincrtest/.cache/provides

cat <<END > providesincrtest_src1/profiles/profile/make.defaults
ARCH=test
USERLAND="GNU"
KERNEL="linux"
CHOST="i286-badger-linux-gnu"
END
echo providesincrtest_src1 >providesincrtest_src1/profiles/repo_name
echo providesincrtest_src2 >providesincrtest_src2/profiles/repo_name
echo cat1 >providesincrtest_src1/profiles/categories
echo cat2 >>providesincrtest_src1/profiles/categories
echo cat1 >providesincrtest_src2/profiles/categories

cat <<END >providesincrtest_src1/cat1/pkg1/pkg1-1.ebuild
KEYWORDS="test"
SLOT="\${PV:0:1}"
PROVIDE="enabled? ( virtual/foo ) disabled? ( virtual/bar )"
IUSE="enabled disabled"
END
cp providesincrtest_src1/cat1/pkg1/pkg1-{1,1.1}.ebuild
cp providesincrtest_src1/cat1/pkg1/pkg1-{1,2}.ebuild
cp providesincrtest_src1/cat1/{pkg1/pkg1,pkg2/pkg2}-1.ebuild
cp providesincrtest_src1/cat1/pkg1/pkg1-1.1.ebuild providesincrtest_src2/cat1/pkg1/pkg1-1.1-r0.ebuild

cat <<END >providesincrtest_src2/cat1/pkg1/pkg1-1.ebuild
KEYWORDS="test"
SLOT="\${PV:0:1}"
PROVIDE="enabled? ( virtual/bar ) disabled? ( virtual/foo )"
IUSE="enabled disabled"
END

cat <<END >providesincrtest_src1/cat2/pkg1/pkg1-1.ebuild
EAPI=paludis-1
KEYWORDS="test"
SLOT="0"
PROVIDE="virtual/moo"
END
cp providesincrtest_src1/cat2/pkg1/pkg1-{1,2}.ebuild

mkdir -p reinstalltest reinstalltest_src{1,2}/{eclass,profiles/profile,cat/pkg} || exit 1

cat <<END > reinstalltest_src1/profiles/profile/make.defaults
ARCH=test
USERLAND="GNU"
KERNEL="linux"
CHOST="i286-badger-linux-gnu"
END
echo reinstalltest_src1 >reinstalltest_src1/profiles/repo_name
echo reinstalltest_src2 >reinstalltest_src2/profiles/repo_name
echo cat >reinstalltest_src1/profiles/categories
echo cat >reinstalltest_src2/profiles/categories

cat <<END >reinstalltest_src1/cat/pkg/pkg-1.ebuild
KEYWORDS="test"
SLOT="0"
END
cp reinstalltest_src1/cat/pkg/pkg-1.ebuild reinstalltest_src2/cat/pkg/pkg-1-r0.ebuild

mkdir -p postinsttest postinsttest_src1/{eclass,profiles/profile,cat/pkg} || exit 1

cat <<END > postinsttest_src1/profiles/profile/make.defaults
ARCH=test
USERLAND="GNU"
KERNEL="linux"
CHOST="i286-badger-linux-gnu"
END
echo postinsttest >postinsttest_src1/profiles/repo_name
echo cat >postinsttest_src1/profiles/categories

cat <<END >postinsttest_src1/cat/pkg/pkg-0.ebuild
if [[ \${PV} == 0* ]]; then
    EAPI=1
else
    EAPI=paludis-1
fi
KEYWORDS="test"
if [[ \${PV} == 2* ]]; then
    SLOT="2"
else
    SLOT="1"
fi
pkg_preinst() {
    OTHER=\$(best_version "\${CATEGORY}/\${PN}:\${SLOT}")
    if [[ -n \${OTHER} ]]; then
        if [[ \${EAPI} == paludis-1 ]] || has_version "=\${CATEGORY}/\${PF}:\${SLOT}"; then
            COMMAND=rmdir
        else
            COMMAND=mkdir
        fi
    else
        COMMAND=:
    fi
}
pkg_postinst() {
    \${COMMAND} "\${ROOT}"/\${OTHER##*/} || die
}
pkg_postrm() {
    if [[ \$VDB_REPOSITORY_TEST_RMDIR == \${PV} ]] ; then
        rmdir "\${ROOT}"/\${PF} || die "rmdir failed, pv is \${PV}, comparing to \$VDB_REPOSITORY_TEST_RMDIR"
    else
        mkdir "\${ROOT}"/\${PF} || die "mkdir failed, pv is \${PV}, comparing to \$VDB_REPOSITORY_TEST_RMDIR"
    fi
}
END
cp postinsttest_src1/cat/pkg/pkg-{0,0.1}.ebuild
cp postinsttest_src1/cat/pkg/pkg-{0,1}.ebuild
cp postinsttest_src1/cat/pkg/pkg-{0,1.1}.ebuild
cp postinsttest_src1/cat/pkg/pkg-{0,2}.ebuild

