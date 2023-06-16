#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir -p vdb_repository_TEST_eapis_dir || exit 1
cd vdb_repository_TEST_eapis_dir || exit 1

mkdir -p distdir
mkdir -p build
mkdir -p root/etc

mkdir -p dstrepo

mkdir -p srcrepo/{profiles/profile,cat/{target,vars}{,-exheres},eclass}
cat <<END > srcrepo/profiles/profile/make.defaults
ARCH=test
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

cat <<'END' > srcrepo/cat/target/target-4.ebuild
EAPI="4"
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

cat <<'END' > srcrepo/cat/target/target-5.ebuild
EAPI="5"
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
    echo MONKEY > ${IMAGE}/usr/share/monkey
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

