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

cd ..
cd ..
