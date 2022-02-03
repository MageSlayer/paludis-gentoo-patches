#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

set -e

mkdir e_repository_TEST_8_dir
cd e_repository_TEST_8_dir

mkdir -p root/etc

mkdir -p vdb
touch vdb/THISISTHEVDB

mkdir -p build

mkdir -p distdir

for format in 7z lha lzh rar; do
    touch "distdir/test.${format}"
done

mkdir -p repo/{profiles/profile,metadata,eclass}
cd repo
echo "test-repo" >> profiles/repo_name
cat <<'END' > profiles/arch.list
amd64
arm
arm64
x86

# Prefix keywords
amd64-linux
arm-linux
arm64-linux
x86-linux
END

echo "cat" >> profiles/categories
cat <<'END' > profiles/profile/make.defaults
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

# bash 5.0 is now sanctioned
# PALUDIS_BASH_COMPAT
mkdir -p "cat/bash-compat"
cat <<'END' > cat/bash-compat/bash-compat-8.ebuild
EAPI="8"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

S="${WORKDIR}"

pkg_pretend() {
    [[ "${BASH_COMPAT}" == '5.0' ]] || [[ "${BASH_COMPAT}" == '50' ]] || die "BASH_COMPAT=${BASH_COMPAT}"
}
END

# econf passes --build, --target, --with-sysroot
# --datarootdir new econf-passed option
# --disable-static new econf-passed option
mkdir -p "cat/econf-added-options"
cat <<'END' > cat/econf-added-options/econf-added-options-8.ebuild
EAPI="8"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

S="${WORKDIR}"

src_prepare(){
    cat <<'EOF' > configure
#!/bin/sh

param=''
for param in "${@}"; do
    if [ '--help' = "${param}" ]; then
        printf '%s\n' '--datarootdir'
        printf '%s\n' '--disable-static'
        exit 0
    fi
done

param=''
datarootdir='0'
disablestatic='0'
for param in "${@}"; do
    if [ "--datarootdir=${EPREFIX}/usr/share" = "${param}" ]; then
        datarootdir='1'
    fi

    if [ '--disable-static' = "${param}" ]; then
        disablestatic='1'
    fi

    if [ '1' = "${datarootdir}" ] && [ '1' = "${disablestatic}" ]; then
        exit '0'
    fi
done

if [ '0' = "${datarootdir}" ]; then
    printf '%s missing\n' '--datarootdir'
fi

if [ '0' = "${disablestatic}" ]; then
    printf '%s missing\n' '--disable-static'
fi

exit '1'
EOF

    chmod +x configure
}
END

# doconfd no longer affected by insopts
# doenvd no longer affected by insopts
# doheader no longer affected by insopts
# doinitd no longer affected by exeopts
mkdir -p "cat/changed-opts"
cat <<'END' > cat/changed-opts/changed-opts-8.ebuild
EAPI="8"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

S="${WORKDIR}"

src_install() {
   insopts --foo
   exeopts --bar
   echo foo > foo
   doconfd foo
   doenvd foo
   doheader foo
   doinitd foo
}
END

# dosym -r allows creating relative symlinks
mkdir -p "cat/dosym-rel"
cat <<'END' > cat/dosym-rel/dosym-rel-8.ebuild
EAPI="8"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

S="${WORKDIR}"

src_install() {
    echo bar > bar
    dobin bar
    dosym ${D}/usr/bin/bar /usr/bin/foo
    [[ "$(readlink "${D}/usr/bin/bar")" == "foo" ]] || die 'link wrong'
}
END

# fetch+ SRC_URI prefix to override fetch restriction
# mirror+ SRC_URI prefix to override mirror restriction
# TODO
mkdir -p "cat/fetch-restrictions"
cat <<'END' > cat/fetch-restrictions/fetch-restrictions-8.ebuild
EAPI="8"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI="fetch+test.7z mirror+test.rar"
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"
RESTRICT="fetch mirror"

END

# hasq banned
# hasv banned
# useq banned
mkdir -p "cat/banned-functions"
cat <<'END' > cat/banned-functions/banned-functions-8.ebuild
EAPI="8"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

S="${WORKDIR}"

pkg_pretend() {
    [[ -n "$(declare -F hasq)" ]]  && die 'hasq is banned'
    [[ -n "$(declare -F hasv)" ]]  && die 'hasv is banned'
    [[ -n "$(declare -F useq)" ]]  && die 'useq is banned'
}
END

# IDEPEND new dependency type for pkg_postinst deps
mkdir -p "cat/changed-vars"
cat <<'END' > cat/changed-vars/changed-vars-8.ebuild
EAPI="8"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"

pkg_pretend() {
for var in IDEPEND; do
  [ -z "${!var+x}" ] && echo "${var} has been added and should be set"
done
}
END

# PATCHES no longer permits specifying options (paths only)
mkdir -p "cat/patches-no-opts"
cat <<'END' > cat/patches-no-opts/patches-no-opts-8.ebuild
EAPI="8"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"

PATCHES=( -p0 "${FILESDIR}"/${P}-foo.patch )

S="${WORKDIR}"

src_unpack() {
    echo first >file || die
}

src_compile() {
    [[ "$(< file)" == 'second' ]] || die 'file wrong'
}
END

mkdir -p "cat/patches-no-opts/files" || exit 1
cat << 'END' > cat/patches-no-opts/files/patches-no-opts-8-foo.patch || exit 1
diff --git a/file b/file
index 9c59e24..e019be0 100644
--- a/file
+++ b/file
@@ -1 +1 @@
-first
+second
END

# PROPERTIES now accumulated across eclasses
# RESTRICT now accumulated across eclasses
# TODO
cat <<'END' > eclass/foo.eclass
PROPERTIES="FOO"
RESTRICT="FOO"
END
cat <<'END' > eclass/bar.eclass
PROPERTIES="BAR"
RESTRICT="BAR"
END

mkdir -p "cat/accumulated-vars"
cat <<'END' > cat/accumulated-vars/accumulated-vars-8.ebuild
EAPI="8"
inherit foo bar
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"

pkg_pretend() {
    [[ "${PROPERTIES}" == *FOO* ]] || die 'failed to accumulate PROPERTIES'
    [[ "${PROPERTIES}" == *BAR* ]] || die 'failed to accumulate PROPERTIES'
    [[ "${RESTRICT}" == *FOO* ]] || die 'failed to accumulate RESTRICT'
    [[ "${RESTRICT}" == *BAR* ]] || die 'failed to accumulate RESTRICT'
}
END

# test_network new PROPERTIES value for tests requiring Internet
mkdir -p "cat/test-network"
cat <<'END' > cat/test-network/test-network-8.ebuild
EAPI="8"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="flag"
LICENSE="GPL-2"
KEYWORDS="test"
PROPERTIES="test_network"

pkg_pretend() {
    die 'not implemented'
}
src_prepare() {
    die 'not implemented'
}
END

# unpack no longer supports 7-Zip, LHA and RAR formats
# TODO
mkdir -p "cat/unpack-formats-removed"
cat <<'END' > cat/unpack-formats-removed/unpack-formats-removed-8.ebuild
EAPI="8"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI="test.7z test.lha test.lzh test.rar"
SLOT="0"
IUSE="flag"
LICENSE="GPL-2"
KEYWORDS="test"
S="${WORKDIR}"

src_unpack() {
    export PALUDIS_UNPACK_UNRECOGNISED_IS_FATAL=yes
    for format in 7z lha lzh rar; do
        nonfatal unpack test.${format} 2> /dev/null > /dev/null
        [[ '0' -eq "$?" ]] && die "${format} should not be unpacked"
    done

}
END

# updates filenames no longer have to follow nQ-yyyy style
# TODO

# usev accepts second argument to override output value
mkdir -p "cat/usev-second-arg"
cat <<'END' > cat/usev-second-arg/usev-second-arg-8.ebuild
EAPI="8"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="flag"
LICENSE="GPL-2"
KEYWORDS="test"

pkg_pretend() {
    [[ "$(usev flag true)" == "true" ]] || die 'usev second arg no supported'
}
END

# working directory pkg_* phases now start in an empty directory
# TODO
mkdir -p "cat/pkg-empty-dir"
cat <<'END' > cat/pkg-empty-dir/pkg-empty-dir-8.ebuild
EAPI="8"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"

test_impl() {
    if [[ "$(find . | wc -l)" -gt '1' ]]; then
        die "$(pwd) not empty"
    fi

    # Try to make the cwd dirty to check if subsequent pkg_* phases find it
    # empty again.
    # However, while the directory must be empty, it need not be writable, so
    # accept failures.
    touch 'force-non-empty' || :
}

pkg_config() { test_impl; }
pkg_info() { test_impl; }
pkg_nofetch() { test_impl; }
pkg_postinst() { test_impl; }
pkg_postrm() { test_impl; }
pkg_preinst() { test_impl; }
pkg_prerm() { test_impl; }
pkg_setup() { test_impl; }
pkg_pretend() { test_impl; }
END
