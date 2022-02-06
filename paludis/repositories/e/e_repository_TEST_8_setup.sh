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

dosym_relative_path_pms() {
    # Stricly speaking, this function requires realpath and dirname from
    # GNU coreutils 8.32 exactly, but it's difficult to provide that for
    # arbitary input.
    local link=$(realpath -m -s "/${2#/}")
    local linkdir=$(dirname "${link}")
    realpath -m -s --relative-to="${linkdir}" "$1"
}

src_install() {
    echo foo > foo
    dobin foo
    dosym "${EPREFIX}/usr/bin/foo" '/usr/bin/bar'
    dosym -r "${EPREFIX}/usr/bin/foo" '/usr/bin/baz'

    real_target="$(readlink "${D}/usr/bin/bar")"
    expected_target="${EPREFIX}/usr/bin/foo"
    [ "${expected_target}" = "${real_target}" ] || die "absolute link wrong; is: '${real_target}', should have been: '${expected_target}'"

    real_target="$(readlink "${D}/usr/bin/baz")"
    expected_target="foo"
    [ "${expected_target}" = "${real_target}" ] || die "relative link wrong; is: '${real_target}', should have been: '${expected_target}'"
}
END

# fetch+ SRC_URI prefix to override fetch restriction
# mirror+ SRC_URI prefix to override mirror restriction
mkdir -p mirror
cd mirror
for i in {1..3}; do
    touch "test${i}"
    tar cJvf "test${i}.tar.xz" "test${i}" > /dev/null
done
cd ..
MIRROR="$(realpath mirror)"
# RESTRICT | "URI prefix" | Fetching | Mirroring
# (none) | (any) |  allowed  | allowed
mkdir -p "cat/restrict-none"
cat <<END > cat/restrict-none/restrict-none-8.ebuild
EAPI="8"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI="file://${MIRROR}/test1.tar.xz"
SLOT="0"
RESTRICT=""

pkg_nofetch() {
    env|sort
    [[ -z \${A} ]] && return

    elog "The following files cannot be fetched for \${PN}:"
    local x
    for x in \${A}; do
        elog "   \${x}"
    done
    die failed to fetch
}
END

# mirror | (none) / fetch+ |  allowed | prohibited
# mirror | mirror+ | allowed | allowed
mkdir -p "cat/restrict-mirror"
cat <<END > cat/restrict-mirror/restrict-mirror-8.ebuild
EAPI="8"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI="file://${MIRROR}/test1.tar.xz fetch+file://${MIRROR}/test2.tar.xz mirror+file://${MIRROR}/test3.tar.xz"
SLOT="0"
RESTRICT="mirror"

pkg_nofetch() {
    [[ -z \${A} ]] && return

    elog "The following files cannot be fetched for \${PN}:"
    local x
    for x in \${A}; do
        elog "   \${x}"
    done
}
END

# fetch | (none) | prohibited | prohibited
# fetch | fetch+  | allowed | prohibited
# fetch | mirror+ | allowed | allowed
mkdir -p "cat/restrict-fetch"
cat <<END > cat/restrict-fetch/restrict-fetch-8.ebuild
EAPI="8"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI="file://${MIRROR}/test1.tar.xz file+file://${MIRROR}/test2.tar.xz mirror+file://${MIRROR}/test3.tar.xz"
SLOT="0"
RESTRICT="fetch"

pkg_nofetch() {
    [[ -z \${A} ]] && return

    elog "The following files cannot be fetched for \${PN}:"
    local x
    for x in \${A}; do
        elog "   \${x}"
    done
}
END

# hasq banned
mkdir -p "cat/banned-functions-hasq"
cat <<'END' > cat/banned-functions-hasq/banned-functions-hasq-8.ebuild
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
    hasq 'a' 'a'
}
END

# hasv banned
mkdir -p "cat/banned-functions-hasv"
cat <<'END' > cat/banned-functions-hasv/banned-functions-hasv-8.ebuild
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
    hasv 'a' 'a'
}
END

# useq banned
mkdir -p "cat/banned-functions-useq"
cat <<'END' > cat/banned-functions-useq/banned-functions-useq-8.ebuild
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
    useq '!spork'
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
IDEPEND="foo"

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
    echo first > file || die
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

PROPERTIES="BAZ"
RESTRICT="BAZ"

pkg_pretend() {
    [[ "${PROPERTIES}" == *FOO* ]] || die 'failed to accumulate PROPERTIES'
    [[ "${PROPERTIES}" == *BAR* ]] || die 'failed to accumulate PROPERTIES'
    [[ "${PROPERTIES}" == *BAZ* ]] || die 'failed to accumulate PROPERTIES'
    [[ "${RESTRICT}" == *FOO* ]] || die 'failed to accumulate RESTRICT'
    [[ "${RESTRICT}" == *BAR* ]] || die 'failed to accumulate RESTRICT'
    [[ "${RESTRICT}" == *BAZ* ]] || die 'failed to accumulate RESTRICT'
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
RESTRICT='test'
PROPERTIES="test_network"
S="${WORKDIR}"

src_test() {
    ping -c1 example.invalid
    export RAN_TEST="true"
}

src_install() {
    [[ -z "${RAN_TEST}" ]] && die "didn't run network tests"
}
END

# unpack no longer supports 7-Zip, LHA and RAR formats
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

pkg_config()   { test_impl; }
pkg_info()     { test_impl; }
pkg_nofetch()  { test_impl; }
pkg_postinst() { test_impl; }
pkg_postrm()   { test_impl; }
pkg_preinst()  { test_impl; }
pkg_prerm()    { test_impl; }
pkg_setup()    { test_impl; }
pkg_pretend()  { test_impl; }
END
