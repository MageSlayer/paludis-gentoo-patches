#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

set -e

mkdir e_repository_TEST_7_dir
cd e_repository_TEST_7_dir

mkdir -p root/etc

mkdir -p vdb
touch vdb/THISISTHEVDB

mkdir -p build

mkdir -p distdir

mkdir -p repo/{profiles/profile,metadata,eclass}
cd repo
echo "test-repo" >> profiles/repo_name
cat <<END > profiles/arch.list
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
cat<<END > eclass/test.eclass
gen_deps() {
    :
}
END

# assert now legal in subshell
mkdir -p "cat/assert-in-subshell"
cat <<'END' > cat/assert-in-subshell/assert-in-subshell-7.ebuild
EAPI="7"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

S="${WORKDIR}"

pkg_setup() {
    (
        test_func
    )
}

test_func() {
    (
        true | false | true
        assert 'test_func'
    )
}
END

# best_version -b, -d, -r for different dependency types
mkdir -p "cat/best-version"
cat <<'END' > cat/best-version/best-version-7.ebuild
EAPI="7"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

S="${WORKDIR}"

pkg_pretend() {
    for option in -r -b -d; do
        if ! best_version ${option} cat/pretend-installed >/dev/null ; then
            die 'failed cat/pretend-installed'
        fi

        BV1=$(best_version ${option} cat/pretend-installed )
        [[ "${BV1}" == 'cat/pretend-installed-1' ]] || die "BV1 is ${BV1}"

        if best_version ${option} cat/doesnotexist >/dev/null ; then
            die 'not failed cat/doesnotexist'
        fi

        BV2=$(best_version ${option} cat/doesnotexist )
        [[ "${BV2}" == '' ]] || die "BV2 is ${BV2}"
    done
}
END

# die now legal in subshell
mkdir -p "cat/die-in-subshell"
cat <<'END' > cat/die-in-subshell/die-in-subshell-7.ebuild
EAPI="7"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

S="${WORKDIR}"

src_configure() {
    ( die "boom?" )
}
END

#dohtml removed; replacement: dodoc -r, docinto
mkdir -p "cat/banned-dohtml"
cat <<'END' > cat/banned-dohtml/banned-dohtml-7.ebuild
EAPI="7"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

S="${WORKDIR}"

src_prepare() {
    mkdir foo
    touch foo/a.txt
    touch foo/b.txt
}

src_install() {
    dohtml -A txt -r foo/.
}
END

#dolib removed; replacement: dolib.a, dolib.so
mkdir -p "cat/banned-dolib"
cat <<'END' > cat/banned-dolib/banned-dolib-7.ebuild
EAPI="7"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

S="${WORKDIR}"

src_unpack() {
    echo 'libfoo.a' > libfoo.a
    echo 'foo.o' > foo.o
}

src_install() {
    dolib libfoo.a foo.o
}
END

mkdir -p "cat/banned-dolib-rep"
cat <<'END' > cat/banned-dolib-rep/banned-dolib-rep-7.ebuild
EAPI="7"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

S="${WORKDIR}"

src_unpack() {
    echo 'libfoo.a' > libfoo.a
    echo 'foo.o' > foo.o
    echo 'libfoo.so' > libfoo.so
    echo 'libfoo.so.1' > libfoo.so.1
}

src_install() {
    local ld="${D%/}/usr/lib"
    local perms

    dolib.a libfoo.a foo.o

    perms="$(stat -c "%a" "${ld}/libfoo.a")"
    [[ "${perms}" == "644" ]] || die 'permission error'
    perms="$(stat -c "%a" "${ld}/foo.o")"
    [[ "${perms}" == "644" ]] || die 'permission error'

    dolib.so libfoo.so libfoo.so.1

    perms="$(stat -c "%a" "${ld}/libfoo.so")"
    [[ "${perms}" == "755" ]] || die 'permission error'
    perms="$(stat -c "%a" "${ld}/libfoo.so.1")"
    [[ "${perms}" == "755" ]] || die 'permission error'
}
END

# domo no longer respects into, always uses /usr
mkdir -p "cat/changed-domo"
cat <<'END' > cat/changed-domo/changed-domo-7.ebuild
EAPI="7"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

S="${WORKDIR}"

src_install() {
    touch foo.mo
    into 'foo'
    domo foo.mo
    [[ -d "${D}/foo" ]] && die 'domo used into'
}
END

# dostrip new command; controls stripping per file
mkdir -p "cat/added-dostrip"
cat <<'END' > cat/added-dostrip/added-dostrip-7.ebuild
EAPI="7"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

S="${WORKDIR}"

src_prepare() {
    echo "int main(){}" > test.cpp
}

src_compile() {
    ${CXX:-g++} -g3 test.cpp -o test
}

src_install() {
    dobin test
    dostrip -x test || die 'dostrip -x failed'
}

pkg_postinst() {
    file "${D}/usr/bin/test" | grep 'not stripped' || die 'dostrip -x has no effect'
}
END

# eapply GNU patch 2.7 guaranteed, git patches supported
mkdir -p "cat/eapply-git-diff-support/files" || exit 1
cat << 'END' > cat/eapply-git-diff-support/eapply-git-diff-support-7.ebuild
EAPI="7"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"

S=${WORKDIR}

src_unpack() {
    echo first >file || die 'normal setup'
    echo 'thank you' >rename_me || die 'rename setup'
}

src_prepare() {
    eapply "${FILESDIR}"/first || die 'eapply normal'
    [[ "$(< file)" == 'second' ]] || die 'file normal'

    eapply "${FILESDIR}"/rename || die 'eapply rename'
    [[ "$(< renamed_you)" == 'thank you' ]] || die 'file rename'
}
END
cat << 'END' > cat/eapply-git-diff-support/files/first
diff --git a/file b/file
index 9c59e24..e019be0 100644
--- a/file
+++ b/file
@@ -1 +1 @@
-first
+second
END
cat << 'END' > cat/eapply-git-diff-support/files/rename
diff --git a/rename_me b/renamed_you
similarity index 100%
rename from rename_me
rename to renamed_you
END

# eapply_user GNU patch 2.7 guaranteed, git patches supported
mkdir -p "cat/eapply-user-git-diff-support/files" || exit 1
cat << 'END' > cat/eapply-user-git-diff-support/eapply-user-git-diff-support-7.ebuild
EAPI="7"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"

S=${WORKDIR}

src_unpack() {
    echo first >file || die 'normal setup'
    echo 'thank you' >rename_me || die 'rename setup'
}

src_prepare() {
    eapply_user || die 'eapply_user'
    [[ "$(< file)" == 'second' ]] || die 'patch failed normal'
    [[ "$(< renamed_you)" == 'thank you' ]] || die 'patch failed rename'
}
END
mkdir -p "../root/var/paludis/user_patches/cat/eapply-user-git-diff-support-7"
cat << 'END' > ../root/var/paludis/user_patches/cat/eapply-user-git-diff-support-7/eapply-user-git-diff-support-7.patch
diff --git a/file b/file
index 9c59e24..e019be0 100644
--- a/file
+++ b/file
@@ -1 +1 @@
-first
+second
diff --git a/rename_me b/renamed_you
similarity index 100%
rename from rename_me
rename to renamed_you
END

# ebegin no longer outputs to stdout
mkdir -p "cat/ebegin-not-to-stdout"
cat <<'END' > cat/ebegin-not-to-stdout/ebegin-not-to-stdout-7.ebuild
EAPI="7"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

S="${WORKDIR}"

pkg_pretend() {
    ebegin "foo" > stdout 2> stderr
    [[ -s stdout ]] && die 'ebegin wrote to stdout'
}
END

# econf passes --build, --target, --with-sysroot
mkdir -p "cat/econf-added-options"
cat <<'END' > cat/econf-added-options/econf-added-options-7.ebuild
EAPI="7"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
CBUILD="i286-banana-linux-gnu"
CTARGET="i386-kiwi-linux-gnu"
LICENSE="GPL-2"
KEYWORDS="test"

S="${WORKDIR}"

src_prepare() {
    cat <<'EOF' > configure
#!/bin/sh

param=''
for param in "${@}"; do
    if [ '--help' = "${param}" ]; then
        printf '%s\n' '--build'
        printf '%s\n' '--target'
        printf '%s\n' '--with-sysroot'
        exit 0
    fi
done

param=''
build='0'
target='0'
withsysroot='0'
for param in "${@}"; do
    if [ "--build=${CBUILD}" = "${param}" ]; then
        build='1'
    fi

    if [ "--target=${CTARGET}" = "${param}" ]; then
        target='1'
    fi

    if [ "--with-sysroot=${ESYSROOT:-/}" = "${param}" ]; then
        withsysroot='1'
    fi

    if [ '1' = "${build}" ] && [ '1' = "${target}" ] && [ '1' = "${withsysroot}" ]; then
        exit '0'
    fi
done

if [ '0' = "${build}" ]; then
    printf '%s missing\n' '--build'
fi

if [ '0' = "${target}" ]; then
    printf '%s missing\n' '--target'
fi

if [ '0' = "${withsysroot}" ]; then
    printf '%s missing\n' '--with-sysroot'
fi

exit '1'
EOF

    chmod +x configure
}

src_configure() {
    # For some reason, CTARGET is defined as a variable in the current phase,
    # but not exported to the environment.
    # For this test to work, we need it available in the configure script, so
    # just export it.
    export CTARGET

    default
}
END

# eend no longer outputs to stdout
mkdir -p "cat/eend-not-to-stdout"
cat <<'END' > cat/eend-not-to-stdout/eend-not-to-stdout-7.ebuild
EAPI="7"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

S="${WORKDIR}"

pkg_pretend() {
    eend "foo" > stdout 2> stderr
    [[ -s stdout ]] && die 'eend wrote to stdout'
}
END

# einfo no longer outputs to stdout
mkdir -p "cat/einfo-not-to-stdout"
cat <<'END' > cat/einfo-not-to-stdout/einfo-not-to-stdout-7.ebuild
EAPI="7"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

S="${WORKDIR}"

pkg_pretend() {
    einfo "foo" > stdout 2> stderr
    [[ -s stdout ]] && die 'einfo wrote to stdout'
}
END

# elog no longer outputs to stdout
mkdir -p "cat/elog-not-to-stdout"
cat <<'END' > cat/elog-not-to-stdout/elog-not-to-stdout-7.ebuild
EAPI="7"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

S="${WORKDIR}"

pkg_pretend() {
    elog "foo" > stdout 2> stderr
    [[ -s stdout ]] && die 'elog wrote to stdout'
}
END

# eqawarn new command; prints QA warning for devs
mkdir -p "cat/eqawarn-not-to-stdout"
cat <<'END' > cat/eqawarn-not-to-stdout/eqawarn-not-to-stdout-7.ebuild
EAPI="7"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

S="${WORKDIR}"

pkg_pretend() {
    eqawarn "foo" > stdout 2> stderr || die 'eqawarn failed, maybe undefined?'
    [[ -s stdout ]] && die 'eqawarn wrote to stdout'
}
END

# has_version -b, -d, -r for different dependency types
mkdir -p "cat/has-version"
cat <<'END' > cat/has-version/has-version-7.ebuild
EAPI="7"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

S="${WORKDIR}"

pkg_pretend() {
    for option in -r -b -d; do
    if ! has_version ${option} cat/pretend-installed ; then
        die 'failed cat/pretend-installed'
    fi

    if has_version ${option} cat/doesnotexist >/dev/null ; then
        die 'not failed cat/doesnotexist'
    fi
    done
}
END

# libopts removed; replacement: doins w/ get_libdir
mkdir -p "cat/banned-libopts"
cat <<'END' > cat/banned-libopts/banned-libopts-7.ebuild
EAPI="7"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

S="${WORKDIR}"

src_unpack() {
    echo 'libfoo.a' > libfoo.a
}

src_install() {
    libopts -m0755
    dolib.a libfoo.a
}
END

# nonfatal implemented both as external helper and function
mkdir -p "cat/nonfatal-external-and-function"
cat <<'END' > cat/nonfatal-external-and-function/nonfatal-external-and-function-7.ebuild
EAPI="7"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

S="${WORKDIR}"

try_other_tests() {
    emake -j1 check-1
    emake check-2
}

src_test() {
    # Works in EAPI 4 and newer
    nonfatal emake check
    # Requires EAPI 7: try_other_tests is a shell function
    nonfatal try_other_tests
}

src_install() {
    insinto /usr/share/mytext

    # Works in EAPI 4 and newer
    if ! nonfatal find -name '*.txt' -exec doins {} +; then
        die 'Installing text files failed for some reason!'
    fi

    # Requires EAPI 7: nonfatal called via subprocess
    if ! find -name '*.txt' -exec nonfatal doins {} +; then
        die 'Installing text files failed for some reason!'
    fi
}
END

# test for inherit/eclasses still working after ECLASSDIR/ECLASSDIRS removal
mkdir -p "cat/removed-eclassdir"
cat <<'END' > cat/removed-eclassdir/removed-eclassdir-7.ebuild
EAPI="7"
inherit test

DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

S="${WORKDIR}"

src_install() {
    gen_deps
}
END

mkdir -p "cat/vers"
cat <<'END' > cat/vers/vers-7.ebuild || exit 1
EAPI="7"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

S="${WORKDIR}"

pkg_pretend() {
    [[ "$(ver_cut 0 1.2.3)" == "" ]] || die 'ver_cut1 failed'
    [[ "$(ver_cut 0-1 1.2.3)" == "1" ]] || die 'ver_cut2 failed'
    [[ "$(ver_cut 1 1.2.3)" == "1" ]] || die 'ver_cut3 failed'
    [[ "$(ver_cut 1- 1.2.3)" == "1.2.3" ]] || die 'ver_cut4 failed'
    [[ "$(ver_cut 1-2 1.2.3)" == "1.2" ]] || die 'ver_cut5 failed'
    [[ "$(ver_cut 1-3 1.2.3)" == "1.2.3" ]] || die 'ver_cut6 failed'
    [[ "$(ver_cut 2 1.2.3)" == "2" ]] || die 'ver_cut7 failed'
    [[ "$(ver_cut 2-3 1.2.3)" == "2.3" ]] || die 'ver_cut8 failed'
    [[ "$(ver_cut 3- 1.2.3)" == "3" ]] || die 'ver_cut9 failed'
    [[ "$(ver_cut 4- 1.2.3)" == "" ]] || die 'ver_cut10 failed'

    [[ "$(ver_rs 0 \# 1.2.3)" == "1.2.3" ]] || die 'ver_rs1 failed'
    [[ "$(ver_rs 0 \# .11.2.)" == "#11.2." ]] || die 'ver_rs2 failed'
    [[ "$(ver_rs 0-1 \# 1.2.3)" == "1#2.3" ]] || die 'ver_rs3 failed'
    [[ "$(ver_rs 1 \# 1.2.3)" == "1#2.3" ]] || die 'ver_rs4 failed'
    [[ "$(ver_rs 1-2 \# 1.2.3)" == "1#2#3" ]] || die 'ver_rs5 failed'
    [[ "$(ver_rs 2 \# 1.2.3)" == "1.2#3" ]] || die 'ver_rs6 failed'
    [[ "$(ver_rs 2-3 \# 1.2.3)" == "1.2#3" ]] || die 'ver_rs7 failed'
    [[ "$(ver_rs 3 \# 1.2.3)" == "1.2.3" ]] || die 'ver_rs8 failed'

    ver_test 1.2 -gt 1.1 || die 'ver_test1 failed'
    ver_test 1.1.1 -ge 1.1 || die 'ver_test2 failed'
    ver_test 1.1 -eq 1.1 || die 'ver_test3 failed'
    ver_test 1.1 -ne 1.2 || die 'ver_test4 failed'
    ver_test 1.1 -le 1.2 || die 'ver_test5 failed'
    ver_test 1.0 -lt 1.2 || die 'ver_test6 failed'
}
END

mkdir -p "cat/no-trail-slash"
cat <<'END' > cat/no-trail-slash/no-trail-slash-7.ebuild
EAPI="7"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

S="${WORKDIR}"

test_vars() {
    [[ -z "${ROOT}" ]] || die 'ROOT is non-empty'
    [[ -z "${EROOT}" ]] || die 'EROOT is non-empty'
    [[ "${BROOT}" == "${BROOT%/}" ]] || die 'BROOT has trailing slash'
    [[ "${D}" == "${D%/}" ]] || die 'D has trailing slash'
    [[ "${ED}" == "${ED%/}" ]] || die 'ED has trailing slash'
}

pkg_setup() {
    test_vars
}

src_install() {
    test_vars
}

pkg_postinst() {
    test_vars
}
END

# eprefix
mkdir -p "cat/prefix"
cat <<'END' > cat/prefix/prefix-7.ebuild
EAPI="7"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"

pkg_setup() {
    die 'FIXME'
    echo "EPREFIX ${EPREFIX}"

    echo "ED ${ED}"
    echo "   ${D%/}${EPREFIX}"

    echo "EROOT ${EROOT}"
    echo "ROOT ${ROOT}"
    echo "BROOT ${BROOT}"
    echo "SYSROOT ${SYSROOT}"
    echo "ESYSROOT ${ESYSROOT}"
    [[ "${ED}" == "${D%/}${EPREFIX}" ]] || die 'ED is wrong'
    [[ "${EROOT}" == "${ROOT%/}${EPREFIX}" ]] || die 'EROOT is wrong'
    [[ "${ESYSROOT}" == "${SYSROOT%/}${EPREFIX}" ]] || die 'ESYSROOT is wrong'
}
END

mkdir -p "cat/changed-vars"
cat <<'END' > cat/changed-vars/changed-vars-7.ebuild
EAPI="7"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"

pkg_setup() {
    for var in DESTTREE ECLASSDIR INSDESTTREE PORTDIR; do
        [ -z "${!var+x}" ] || die "${var} has been removed and should not be set"
    done
    for var in ENV_UNSET BDEPEND BROOT ESYSROOT SYSROOT; do
        [ -z "${!var+x}" ] && die "${var} has been added and should be set"
    done
}
END

mkdir -p "cat/selectors-dep"
cat <<'END' > cat/selectors-dep/selectors-dep-7.ebuild
EAPI="7"
inherit test

DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"

# This will trigger an error if gen_deps outputs empty string
DEPEND="|| ( $(gen_deps) )"
END

mkdir -p "cat/selectors-or"
cat <<END > cat/selectors-or/selectors-or-7.ebuild
EAPI="7"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="a foo b bar"
LICENSE="GPL-2"
KEYWORDS="test"

# EAPI 6: this is satisfied w/ USE="-a -b"
# EAPI 7: requires a+foo OR b+bar
REQUIRED_USE="|| ( a? ( foo ) b? ( bar ) )"
END

mkdir -p "cat/selectors-xor"
cat <<END > cat/selectors-xor/selectors-xor-7.ebuild
EAPI="7"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="a foo b bar"
LICENSE="GPL-2"
KEYWORDS="test"

# EAPI 6: this is satisfied w/ USE="-a -b"
# EAPI 7: requires a+foo XOR b+bar
REQUIRED_USE="^^ ( a? ( foo ) b? ( bar ) )"
END
