#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir e_repository_TEST_symlink_rewriting_dir || exit 1
cd e_repository_TEST_symlink_rewriting_dir || exit 1

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

mkdir -p repo/{eclass,distfiles,profiles/profile} || exit 1
mkdir -p repo/cat/pkg || exit 1
cd repo || exit 1
echo "test-repo" >> profiles/repo_name || exit 1
echo "cat" >> profiles/categories || exit 1
cat <<END > profiles/profile/make.defaults
ARCH=test
USERLAND="GNU"
KERNEL="linux"
LIBC="glibc"
CHOST="i286-badger-linux-gnu"
END
cat <<END > cat/pkg/pkg-1.ebuild || exit 1
SLOT="0"
PLATFORMS="test"
src_install() {
	ln -s "\${IMAGE}/foo" "\${IMAGE}/bar" || die
}
END
cd ..

cd ..

