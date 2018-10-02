#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir e_repository_TEST_7_dir || exit 1
cd e_repository_TEST_7_dir || exit 1

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
IUSE="eclass-flag"

eclass_func() {
    :
}

END

mkdir -p "cat/banned-functions"
cat <<END > cat/banned-functions/banned-functions-7.ebuild || exit 1
EAPI="7"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

S="\${WORKDIR}"

src_install() {
    dohtml -A txt -r foo/.
}
END

# negative test for "dolib"
mkdir -p "cat/banned-functions2"
cat <<END > cat/banned-functions2/banned-functions2-7.ebuild || exit 1
EAPI="7"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

S="\${WORKDIR}"

src_install() {
    dolib libfoo.a foo.o
}
END

mkdir -p "cat/banned-functions3"
cat <<END > cat/banned-functions3/banned-functions3-7.ebuild || exit 1
EAPI="7"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

S="\${WORKDIR}"

src_unpack() {
    echo 'libfoo.a' > libfoo.a
    echo 'foo.o' > foo.o
    echo 'libfoo.so' > libfoo.so
    echo 'libfoo.so.1' > libfoo.so.1
}

src_install() {
    local ld="\${D%/}/usr/lib"
    local perms

    dolib.a libfoo.a foo.o

    perms=\$(stat -c "%a" "\$ld/libfoo.a")
    [[ \$perms == "644" ]] || die permission error
    perms=\$(stat -c "%a" "\$ld/foo.o")
    [[ \$perms == "644" ]] || die permission error

    dolib.so libfoo.so libfoo.so.1

    perms=\$(stat -c "%a" "\$ld/libfoo.so")
    [[ \$perms == "755" ]] || die permission error
    perms=\$(stat -c "%a" "\$ld/libfoo.so.1")
    [[ \$perms == "755" ]] || die permission error
}
END

# negative test for "libopts"
mkdir -p "cat/banned-functions4"
cat <<END > cat/banned-functions4/banned-functions4-7.ebuild || exit 1
EAPI="7"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

S="\${WORKDIR}"

src_install() {
    libopts "-m0644"
}
END

# test for inherit/eclasses still working after ECLASSDIR/ECLASSDIRS removal
mkdir -p "cat/banned-functions5"
cat <<END > cat/banned-functions5/banned-functions5-7.ebuild || exit 1
EAPI="7"
inherit test

DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

S="\${WORKDIR}"

src_install() {
    eclass_func
}

END

mkdir -p "cat/vers"
cat <<END > cat/vers/vers-7.ebuild || exit 1
EAPI="7"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE="spork"
LICENSE="GPL-2"
KEYWORDS="test"

S="\${WORKDIR}"

src_install() {
    [[ \$(ver_cut 0 1.2.3) == "" ]] || die ver_cut1 failed
    [[ \$(ver_cut 0-1 1.2.3) == "1" ]] || die ver_cut2 failed
    [[ \$(ver_cut 1 1.2.3) == "1" ]] || die ver_cut3 failed
    [[ \$(ver_cut 1- 1.2.3) == "1.2.3" ]] || die ver_cut4 failed
    [[ \$(ver_cut 1-2 1.2.3) == "1.2" ]] || die ver_cut5 failed
    [[ \$(ver_cut 1-3 1.2.3) == "1.2.3" ]] || die ver_cut6 failed
    [[ \$(ver_cut 2 1.2.3) == "2" ]] || die ver_cut7 failed
    [[ \$(ver_cut 2-3 1.2.3) == "2.3" ]] || die ver_cut8 failed
    [[ \$(ver_cut 3- 1.2.3) == "3" ]] || die ver_cut9 failed
    [[ \$(ver_cut 4- 1.2.3) == "" ]] || die ver_cut10 failed

    [[ \$(ver_rs 0 \# 1.2.3) == "1.2.3" ]] || die ver_rs1 failed
    [[ \$(ver_rs 0 \# .11.2.) == "#11.2." ]] || die ver_rs2 failed
    [[ \$(ver_rs 0-1 \# 1.2.3) == "1#2.3" ]] || die ver_rs3 failed
    [[ \$(ver_rs 1 \# 1.2.3) == "1#2.3" ]] || die ver_rs4 failed
    [[ \$(ver_rs 1-2 \# 1.2.3) == "1#2#3" ]] || die ver_rs5 failed
    [[ \$(ver_rs 2 \# 1.2.3) == "1.2#3" ]] || die ver_rs6 failed
    [[ \$(ver_rs 2-3 \# 1.2.3) == "1.2#3" ]] || die ver_rs7 failed
    [[ \$(ver_rs 3 \# 1.2.3) == "1.2.3" ]] || die ver_rs8 failed

    ver_test 1.2 -gt 1.1 || die ver_test1 failed
    ver_test 1.1.1 -ge 1.1 || die ver_test2 failed
    ver_test 1.1 -eq 1.1 || die ver_test3 failed
    ver_test 1.1 -ne 1.2 || die ver_test4 failed
    ver_test 1.1 -le 1.2 || die ver_test5 failed
    ver_test 1.0 -lt 1.2 || die ver_test6 failed
}
END

mkdir -p "cat/no-trail-slash"
cat <<'END' > cat/no-trail-slash/no-trail-slash-7.ebuild || exit 1
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
    [[ $ROOT == "" ]] || die ROOT is non-empty
    [[ $EROOT == "" ]] || die EROOT is non-empty
    [[ $D == ${D%/} ]] || die D has trailing slash
    [[ $ED == ${ED%/} ]] || die ED has trailing slash
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

cd ..
cd ..
