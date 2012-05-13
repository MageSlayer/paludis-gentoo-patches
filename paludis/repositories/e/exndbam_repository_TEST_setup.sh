#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir -p exndbam_repository_TEST_dir || exit 1
cd exndbam_repository_TEST_dir || exit 1

mkdir -p distdir
mkdir -p build
mkdir -p root/etc

mkdir -p repo1/ || exit 1

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

