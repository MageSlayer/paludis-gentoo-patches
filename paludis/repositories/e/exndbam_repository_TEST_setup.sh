#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir -p exndbam_repository_TEST_dir || exit 1
cd exndbam_repository_TEST_dir || exit 1

mkdir -p distdir
mkdir -p build
mkdir -p root/etc

mkdir -p repo1/ || exit 1

mkdir -p installed || exit 1
mkdir -p parts/{metadata,profiles/profile,packages/category/partitioned} || exit 1
mkdir -p postinsttest postinsttest_src1/{eclass,profiles/profile,cat/pkg} || exit 1

cat <<END > postinsttest_src1/profiles/profile/make.defaults
ARCH=test
KERNEL="linux"
CHOST="i286-badger-linux-gnu"
END
echo postinsttest >postinsttest_src1/profiles/repo_name
echo cat >postinsttest_src1/profiles/categories

cat <<END >postinsttest_src1/cat/pkg/pkg-0.ebuild
EAPI=1
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
    if has_version "\${CATEGORY}/\${PN}:\${SLOT}[<\${PVR}&=0*]" || has_version "\${CATEGORY}/\${PN}:\${SLOT}[>\${PVR}&=0*]"; then
        rmdir "\${ROOT}"/\${PF} || die
    else
        mkdir "\${ROOT}"/\${PF} || die
    fi
}
END
cp postinsttest_src1/cat/pkg/pkg-{0,0.1}.ebuild
cp postinsttest_src1/cat/pkg/pkg-{0,1}.ebuild
sed -i -e 's/EAPI=1/EAPI=paludis-1/' postinsttest_src1/cat/pkg/pkg-1.ebuild
cp postinsttest_src1/cat/pkg/pkg-{1,1.1}.ebuild
cp postinsttest_src1/cat/pkg/pkg-{1,2}.ebuild

echo '*/* PLATFORM: (test)' > parts/profiles/profile/options.conf
cat <<- EOF > parts/profiles/profile/make.defaults
CHOST="i686-pc-linux-gnu"
EOF
echo parts > parts/profiles/repo_name
echo category > parts/metadata/categories.conf

cat <<- EOF > parts/packages/category/partitioned/partitioned-0.exheres-0
PLATFORMS="test"
MYOPTIONS="parts: binaries"

SLOT="0"

src_unpack() {
    edo mkdir -p "\${WORK}"
}

src_install() {
    edo mkdir -p "\${IMAGE}"/usr/{bin,lib,share/man/man1}
    edo touch "\${IMAGE}"/usr/{bin/binary,lib/library.{so,a},share/man/man1/expart.1}

    expart binaries /usr/bin
    expart libraries /usr/lib
}
EOF

cat <<- EOF > parts/packages/category/partitioned/partitioned-1.exheres-0
PLATFORMS="test"
MYOPTIONS="parts: binaries libraries"

SLOT="0"

src_unpack() {
    edo mkdir -p "\${WORK}"
}

src_install() {
    edo mkdir -p "\${IMAGE}"/usr/{bin,lib,share/man/man1}
    edo touch "\${IMAGE}"/usr/{bin/binary,lib/library.{so,a},share/man/man1/expart.1}

    expart binaries /usr/bin
    expart libraries /usr/lib
}
EOF

