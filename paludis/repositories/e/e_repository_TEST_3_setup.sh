#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir e_repository_TEST_3_dir || exit 1
cd e_repository_TEST_3_dir || exit 1

mkdir -p root/etc

mkdir -p vdb
touch vdb/THISISTHEVDB

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

mkdir -p fetchable
echo "one" > fetchable/fetchable-1.txt || exit 1
echo "two" > fetchable/fetchable-2.txt || exit 1

mkdir -p repo/{profiles/profile,metadata,eclass} || exit 1
cd repo || exit 1
echo "test-repo" >> profiles/repo_name || exit 1
echo "cat" >> profiles/categories || exit 1
cat <<END > profiles/profile/virtuals
virtual/virtual-pretend-installed cat/pretend-installed
virtual/virtual-doesnotexist cat/doesnotexist
END
cat <<END > profiles/profile/make.defaults
ARCH="cheese"
KERNEL="linux"
LIBC="glibc"
CHOST="i286-badger-linux-gnu"
LINGUAS="enabled_en enabled_en_GB enabled_en_GB@UTF-8"
USE_EXPAND="LINGUAS"
USE_EXPAND_UNPREFIXED="ARCH"
USE_EXPAND_IMPLICIT="ARCH"
USE_EXPAND_VALUES_ARCH="cheese otherarch"
IUSE_IMPLICIT="build"
END
mkdir -p "cat/prefix"
cat <<'END' > cat/prefix/prefix-3.ebuild || exit 1
EAPI="3"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"

pkg_setup() {
    [[ "/" == ${EROOT} ]] || die eroot
    [[ "${D}" == "${ED}" ]] || die ed
    [[ "${EPREFIX}" == "" ]] || die eprefix
}
END
cd ..
